/* Rgethostbynam.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#if defined(ISC)
#include <net/errno.h>
#endif /* #if defined(ISC) */
#include <stdio.h>
#include <netdb.h>
#include <syslog.h>
#if (defined(sun) && !defined(SOLARIS)) || defined(sgi)
#include <strings.h>
#else
#include <string.h>
#endif
#include "socks.h"

#if !defined(NULL)
#define NULL 0
#endif

extern int socks_useSyslog;

struct hostent socks_fakeIP[NUMFAKEIP];
static char *Fh_aliases[NUMFAKEIP], *Fh_addr_list[NUMFAKEIP*2];
static u_int32 F_iplist[NUMFAKEIP];

static struct hostent socks_Hostent[NUMHOSTENT];


static int initHostent()
{
	int i;
	char **faddr_list = Fh_addr_list;

	for (i = 0; i < NUMFAKEIP; i++) {
		socks_fakeIP[i].h_name = NULL;
		Fh_aliases[i] = NULL;
		socks_fakeIP[i].h_aliases = &Fh_aliases[i];
		socks_fakeIP[i].h_addrtype = AF_INET;
		socks_fakeIP[i].h_length = IPADDRLENG;
		socks_fakeIP[i].h_addr_list = faddr_list;
		*faddr_list++ = (char *)&F_iplist[i];
		F_iplist[i] = htonl(i+1);
		*faddr_list++ = 0;
		
	}

	for (i = 0; i < NUMHOSTENT; i++) {
		socks_Hostent[i].h_name = NULL;
		socks_Hostent[i].h_addrtype = AF_INET;
		socks_Hostent[i].h_length = IPADDRLENG;
	}

} /* initHostent */


struct hostent	*Rgethostbyname(name)
	char *name;
{
	static int initdone = 0;
	static int nextFIP = 0;
	static int currentFIP = 0;
	static int numFIP = 0;
	struct hostent *hr, *hp;
	int ipt, naliases, naddrs;
	char *iplist = NULL;
	char **iparray = NULL;
	char *halist = NULL;
	char **harray = NULL;
	char **p;
	char *q;
	int i, cl;
	static int nextHosetent = 0;
	static int numHostent = 0;
	static int currentHostent = 0;

	if (initdone == 0) {
		initHostent();
		initdone = 1;
	}

	for (ipt = currentHostent, i = 0; i < numHostent; i++) {
		hr = &socks_Hostent[ipt];
		if (strcasecmp(hr->h_name, name) == 0)
			return hr;
		if (--ipt < 0)
			ipt = NUMHOSTENT -1;
	}

	for (ipt = currentFIP, i = 0; i < numFIP; i++) {
		hr = &socks_fakeIP[ipt];
		if ( strcasecmp(hr->h_name, name) == 0)
			return hr;
		if (--ipt < 0)
			ipt = NUMFAKEIP -1;
	}

	if ((hp = gethostbyname(name)) != NULL)
		goto realIP;

	/* Return hostent address of a fake IP */
	if (++currentFIP >= NUMFAKEIP)
		currentFIP = 0;
	if (++numFIP >= NUMFAKEIP)
		numFIP = NUMFAKEIP;
	hr = &socks_fakeIP[currentFIP];
	if (hr->h_name != NULL) {
		free(hr->h_name);
	}
	if ((hr->h_name = strdup(name)) == NULL) {
		goto out_of_memory;
	}
	return hr;

realIP:
	/* Return hostent address of a real IP */
	if (++currentHostent >= NUMHOSTENT)
		currentHostent = 0;
	if (++numHostent >= NUMHOSTENT)
		numHostent = NUMHOSTENT;
	hr = &socks_Hostent[currentHostent];
	if (hr->h_name != NULL) {
		free(hr->h_name);
		harray = hr->h_aliases;
		if (*harray != NULL)
			free(*harray);
		free(harray);
		iparray = hr->h_addr_list;
		free(*iparray);
		free(iparray);
	}
	if ((hr->h_name = strdup(name)) == NULL) {
		goto out_of_memory;
	}
	naliases = 1;
	cl = 0;
	for (p = hp->h_aliases; *p != NULL; p++) {
		naliases++;
		cl += (strlen(*p) +1);
	}
	if (cl > 0) {
		if ((halist = (char *)malloc(cl)) == NULL) {
			goto out_of_memory;
		}
	}
	naddrs = 1;
	for (p = hp->h_addr_list; *p != NULL; p++) {
		naddrs++;
	}

	if (((harray = (char **)malloc(naliases * sizeof(iplist))) == NULL) ||
	    ((iparray = (char **)malloc(naddrs * sizeof(iplist))) == NULL) ||
	    ((iplist = (char *)malloc((naddrs - 1) * IPADDRLENG)) == NULL)) {
		goto out_of_memory;
	}
	
	hr->h_aliases = harray;
	for (p = hp->h_aliases; *p != NULL; p++) {
		*harray++ = halist;
		for (q = *p; *q != '\0'; )
			*halist++ = *q++;
		*halist++ = '\0';
	}
	*harray = NULL;

	hr->h_addr_list = iparray;
	for (p = hp->h_addr_list; *p != NULL; p++) {
		*iparray++ = iplist;
		q = *p;
		*iplist++ = *q++;
		*iplist++ = *q++;
		*iplist++ = *q++;
		*iplist++ = *q++;
	}
	*iparray = NULL;
	return hr;

out_of_memory:
	if(socks_useSyslog)
		syslog(LOG_LOW, "Out of memory\n");
	else
		fprintf(stderr, "Out of memory\n");
	exit(1);

}
