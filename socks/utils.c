/* utils */

#include <netdb.h>
/* >>> K. Shackelford */
#include <sys/types.h>
#include <netinet/in.h>
/* <<< K. Shackelford */ 
#include <arpa/inet.h>
#include <stdio.h>
#include <ctype.h>
#if (defined(sun) && !defined(SOLARIS)) || defined(sgi)
#include <strings.h>
#endif
#include <string.h>
#include <sys/socket.h>
#include <syslog.h>
#include "socks.h"

#ifdef SOLARIS
#include "bstring.h"
#endif

extern struct in_addr socks_self;
extern struct hostent socks_fakeIP[];

/*
 * These functions are used by both Validate (for sockd)
 * and socks_check_cconf (for clients).
 */

/*
**  Simple 'socks_mkargs' doesn't handle \, ", or '.
*/
void socks_mkargs(cp, argc, argv, max)
char	*cp;
int	*argc;
char	*argv[];
int	max;
{
	*argc = 0;
	while (isspace(*cp))
		cp++;

	while (*cp != '\0') {
		argv[(*argc)++] = cp;
		if (*argc >= max)
			return;

		while (!isspace(*cp) && (*cp != '\0'))
			cp++;
		while (isspace(*cp))
			*cp++ = '\0';
	}
}

int socks_GetQuad(dotquad, addr)
char		*dotquad;
struct in_addr	*addr;
/* dotquad must be in dotted quad form. Returns -1 if not. */
{
	if ((addr->s_addr = inet_addr(dotquad)) != (u_int32) -1)
		return 0;
	if (strcmp(dotquad, "255.255.255.255") == 0)
		return 0;
	return -1;
}

/* 
**  Get address, must be dotted quad, or full or partial domain name.
**  Partial domain names are indicated by a leading period.
*/
int socks_GetAddr(name, addr, domain)
char		*name, **domain;
struct in_addr	*addr;
{
	struct hostent	*hp;
	struct netent	*np;

	if (*domain != NULL)
		free(*domain);
	if (socks_GetQuad(name, addr) != -1) {
		*domain = NULL;
		return 0;
	} 
	if ((*domain = strdup(name)) != NULL) {
		addr->s_addr = 0;
		return 0;
	}
	return -1;
}

long socks_GetPort(name)
char		*name;
/* result is in HOST byte order */
{
	struct servent	*sp;

	if ((sp = getservbyname(name, "tcp")) != NULL) {
		return ntohs((short)sp->s_port);
	}
	if (!isdigit(*name))
		return -1;
	return atol(name);
}


int socks_IPtohost(ipaddr, shp)
struct in_addr *ipaddr;
struct sockshost_s *shp;
{
	u_int32 addr;
	struct hostent *hp;
	char	**p;
	int i, rvok;
	char *name;

	for (i = 0; i < MAXNAMESPERHOST; i++) {
		if (shp->dmname[i] != NULL)
			free(shp->dmname[i]);
	}
	bzero(shp, sizeof(struct sockshost_s));
	if ((ipaddr->s_addr == 0) || (ipaddr->s_addr == socks_self.s_addr))
		goto simple_conversion;

	addr = ntohl(ipaddr->s_addr);
	if ((addr >> 8) == 0) {
		shp->shipaddr[0].s_addr = ipaddr->s_addr;
		if ((shp->dmname[0] = strdup(socks_fakeIP[addr-1].h_name)) != NULL)
			return 0;
		else
			return -1;
	}
	hp = gethostbyaddr(&ipaddr->s_addr, IPADDRLENG, AF_INET);
	if (hp == NULL)
		goto simple_conversion;

	/* Double check -- it's too easy to fake IP->name data */
	if ((name = strdup(hp->h_name)) == NULL)
		return -1;
	hp = gethostbyname(name);
	free(name);
	if (hp == NULL)
		goto simple_conversion;
	for (i = 0, p = hp->h_addr_list; (i < MAXIPPERHOST - 1) && (*p != NULL);) {
		bcopy(*p++, &(shp->shipaddr[i++].s_addr), IPADDRLENG);
	}
	/* make sure the original IP address is in the list */
	rvok = 0;
	for (i = 0; shp->shipaddr[i].s_addr != 0; ) {
		if (ipaddr->s_addr == shp->shipaddr[i++].s_addr) {
			rvok = 1;
			break;
		}
	}
	if (!rvok) {
	/* no match; will only use the original IP address */
		goto simple_conversion;
	}
	shp->shipaddr[0].s_addr = ipaddr->s_addr;
	shp->shipaddr[1].s_addr = 0;
	if ((shp->dmname[0] = strdup(hp->h_name)) == NULL) {
		return -1;
	}
	for (i = 1, p = hp->h_aliases; (*p != NULL) && (i < MAXNAMESPERHOST - 1); ) {
		if ((shp->dmname[i++] = strdup(*p++)) == NULL)
			return -1;
	}
	return 0;

simple_conversion:
	shp->shipaddr[0].s_addr = ipaddr->s_addr;
	shp->shipaddr[1].s_addr = 0;
	if ((shp->dmname[0] = strdup(inet_ntoa(*ipaddr))) != NULL)
		return 0;
	else
		return -1;
}

int socks_host(name, shp)
char *name;
struct sockshost_s *shp;
{
	struct in_addr addr;
	struct hostent *hp;
	char	**p;
	int i;

	if (socks_GetQuad(name, &addr) != -1)
		return socks_IPtohost(&addr, shp);

	for (i = 0; i < MAXNAMESPERHOST; i++) {
		if (shp->dmname[i] != NULL)
			free(shp->dmname[i]);
	}
	bzero(shp, sizeof(struct sockshost_s));
	hp = gethostbyname(name);
	if (hp == NULL) {
		if ((shp->dmname[0] = strdup(name)) != NULL)
			return 0;
		else
			return -1;
	}
	for (i = 0, p = hp->h_addr_list; (i < MAXIPPERHOST - 1) && (*p != NULL);) {
		bcopy(*p++, &(shp->shipaddr[i++].s_addr), IPADDRLENG);
	}
	if ((shp->dmname[0] = strdup(hp->h_name)) == NULL) {
		return -1;
	}
	for (i = 1, p = hp->h_aliases; (*p != NULL) && (i < MAXNAMESPERHOST - 1); ) {
		if ((shp->dmname[i++] = strdup(*p++)) == NULL)
			return -1;
	}
	return 0;
}

int socks_ckadr(shp, domain, addr, mask)
struct sockshost_s *shp;
char *domain;
struct in_addr *addr, *mask;
/* domain = '.xyz.com' will match 'a.xyz.com', 'q.wtr.xyz.com' AND 'xyz.com' */
{
	int i;
	u_int32 maddr;
	char **q;
	int dlen;

	if (domain == NULL) {
		if (mask->s_addr == 0)
			return 1;
		maddr = addr->s_addr & mask->s_addr;
		for (i = 0; (i < MAXIPPERHOST - 1) && (shp->shipaddr[i].s_addr != 0); ) {
			if ((shp->shipaddr[i++].s_addr & mask->s_addr) == maddr)
				return 1;
		}
		return 0;
	}
	if (strcmp(domain, "ALL") == 0)
		return 1;
	if (*domain != '.') {
		for (q = shp->dmname; *q != 0; ) {
			if (strcasecmp(*q++, domain) == 0)
				return 1;
		}
		return 0;
	}
	dlen = strlen(domain);
	for (q = shp->dmname; *q != 0; q++) {
		if ((strcasecmp(*q + (strlen(*q) - dlen), domain) == 0) ||
			(strcasecmp(*q, domain + 1) == 0))
			return 1;
	}
	return 0;
}


static int check_userfile(userfile, user, useSyslog)
char	*userfile, *user;
int useSyslog;
/* return 1 if match, 0 otherwise */
/* return -1 if cannot open file */
/* comparison is case-sensitive */
{
	FILE	*fd;
#define BUFLEN 1024
	static char buf[BUFLEN];
	char *p, *q;

	if ((fd = fopen(userfile, "r")) == NULL) {
		if (useSyslog)
			syslog(LOG_HIGH,"Unable to open userfile (%s): %m\n", userfile);
		else
			fprintf(stderr,"Unable to open userfile (%s)\n", userfile);
		return (-1);
	}

	while (fgets(buf, BUFLEN, fd) != NULL) {
		if ((p = index(buf, '\n')) != NULL)
			*p = '\0';
		if (( p = index(buf, '#')) != NULL)
			*p = '\0';

		p = buf;
		while (1) {
			p += strspn(p, " ,\t");
			if ((q = strpbrk(p, " ,\t")) != NULL)
				*q = '\0';
			if (strcmp(p, user) == 0) {
				fclose(fd);
				return 1;
			}
			if (q == NULL)
				break;
			p = q + 1;
		}
	}
	fclose(fd);
	return 0;
}

int socks_ckusr(userlist, user, useSyslog)
char	*userlist, *user;
int useSyslog;

/*
 * Unless userlist is a null pointer, in which case all users are
 * allowed (return 1), otherwise
 * userlist is a nonempty string containing userids separated by
 * commas, no other separators are allowed in the string.
 * 94/03/02: if userlist starts with '/', it specifies a file
 *	containing userids.
 *
 * Return 1 if user is in the userlist; 
 * return 0 if not, or if userfile cannot be open.
 */
{
	char	*p, *q;

	if (!(p = userlist)) {
		return 1;
	}
	do {
		if (q = index(p, ','))
			*q = '\0';
		if (*p == '/') {
			switch (check_userfile(p, user, useSyslog)) {
			case 1:
				return 1;
			case -1:
				return 0;
			default:
				;
			}
		} else if (strcmp(p, user) == 0) {
			return 1;
		}
		if (q) *q++ = ',';

	} while ( p = q);
	return 0;
}

int socks_ckprt(test, port1, port2)
int test, port1, port2;
{
	switch (test) {
	case e_nil:
		return 1;
	case e_eq:
		if (port1 == port2)
			return 1;
		return 0;
	case e_neq:
		if (port1 != port2)
			return 1;
		return 0;
	case e_gt:
		if (port1 > port2)
			return 1;
		return 0;
	case e_ge:
		if (port1 >= port2)
			return 1;
		return 0;
	case e_lt:
		if (port1 < port2)
			return 1;
		return 0;
	case e_le:
		if (port1 <= port2)
			return 1;
		return 0;
	default:
		return 0;
	}
}
