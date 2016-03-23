/* Rconnect */

#include "../defs.h"
#include "../ext.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#if defined(ISC)
#include <net/errno.h>
#endif /* #if defined(ISC) */
#include <stdio.h>
#include <netdb.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <pwd.h>
#include <syslog.h>
#if (defined(sun) && !defined(SOLARIS)) || defined(sgi)
#include <strings.h>
#else
#include <string.h>
#endif
#include "socks.h"
#ifdef LINUX
#include <linux/time.h>
#endif

/* >>> K. Shackelford */
#if defined(hpux) || defined(ultrix) || defined (__NetBSD__) || defined(__FreeBSD__) || defined(AIX) || defined(__bsdi__) || defined(SCO) || defined(ISC) || defined(BIND_RESOLVER)
extern int h_errno;
#endif
/* <<< K.Shackelford */

#include <signal.h>
#include <sys/wait.h>

char *socks_def_server;
char *socks_server;
char *socks_serverlist;

extern char	*getenv();
extern char	*getlogin();
struct sockaddr_in	socks_cursin;
pid_t socks_conn_sock = 0;
pid_t socks_conn_init = 0;
unsigned short	socks_conn_port = 0;
u_int32	socks_conn_host = 0;
int	socks_conn_code = 0;
int	socks_init_done = 0;
unsigned short	socks_last_conn_port = 0;
u_int32	socks_last_conn_host = 0;

struct sockaddr_in socks_nsin;
static struct sockaddr_in	me;
static struct passwd		*pw;
static int	socks_direct = 0;
struct config *scfAddr = NULL;
int Nscf = 0;
int socks_no_conf = 0;
static char *cfStrings = NULL;

int socks_useSyslog = 0;

extern struct hostent socks_fakeIP[];

extern	char	*socks_porttoserv();

struct sockshost_s socks_srcsh, socks_dstsh;
#define dst_name socks_dstsh.dmname[0]
#define dst_serv socks_dstsh.portname
#define src_user socks_srcsh.user
#define real_user socks_srcsh.ruser
#define src_name socks_srcsh.dmname[0]
#define socks_cmd socks_dstsh.user

struct in_addr socks_self;

int	socks_check_result(code)
char code;
{
	switch (code) {
		case SOCKS_FAIL:
			errno = ECONNREFUSED;
			return -1;
		case SOCKS_NO_IDENTD:
			errno = ECONNREFUSED;
			if (socks_useSyslog)
				syslog(LOG_LOW, "Error: SOCKS server %s cannot connect to identd on this host.\n", socks_server);
			else
				fprintf(stderr, "Error: SOCKS server %s cannot connect to identd on this host.\n", socks_server);
			return -1;
		case SOCKS_BAD_ID:
			errno = ECONNREFUSED;
			if (socks_useSyslog)
				syslog(LOG_LOW, "Error: user-id does not agree with the one reported by identd on this host.\n");
			else
				fprintf(stderr, "Error: user-id does not agree with the one reported by identd on this host.\n");
			return -1;
		default:
			return 0;
	}
}


int SOCKSinit(Progname)
char *Progname; /* name of the calling program, "rfinger", "rftp", etc. */
{
	struct stat statfc, statcf;
#ifdef SOCKS_DEFAULT_NS
	static char	*defaultNS = SOCKS_DEFAULT_NS;
#endif
#ifdef SOCKS_DEFAULT_DNAME
	static char	*defaultDNAME = SOCKS_DEFAULT_DNAME;
#endif
	static char	*defaultSERVER = SOCKS_DEFAULT_SERVER;
	char	socks_src_name[NAMELEN];
	char		*cp, *ns, *dp;
	struct hostent	*hp;
	struct servent	*sp;
	int		v,uid;

	if (socks_init_done)
		return;
	socks_init_done = 1;

	bzero((char *)&socks_cursin, sizeof(socks_cursin));
	bzero((char *)&socks_nsin, sizeof(socks_nsin));
	bzero((char *)&me, sizeof(me));

	/* skip the path if included in Progname */
	if( (cp = rindex(Progname, '/')) == NULL)
		cp = Progname;
	else
		cp++;

#ifndef LOG_DAEMON
	(void) openlog(cp, LOG_PID);
#else
	(void) openlog(cp, LOG_PID, SYSLOG_FAC);
#endif

	socks_self.s_addr = inet_addr("127.0.0.1");
	gethostname(socks_src_name, sizeof(socks_src_name));
	if (socks_host(socks_src_name, &socks_srcsh) < 0) {
		if (socks_useSyslog)
			syslog(LOG_LOW, "Out of Memory\n");
		else
			fprintf(stderr, "Out of Memory\n");
		exit(1);
	}
	if (socks_srcsh.shipaddr[0].s_addr = 0) {
		if (socks_useSyslog)
			syslog(LOG_LOW, "Cannot resolve the host's own name: %s\n", socks_src_name);
		else
			fprintf(stderr, "Cannot resolve the host's own name: %s\n", socks_src_name);
		exit(1);
	}

	if ((cp = getlogin()) == NULL) {
	/* some systems returns NULL if process is not attached to a terminal */
		strcpy(real_user, "unknown");
	} else
		strncpy(real_user, cp, sizeof(real_user));
	if ((pw = getpwuid(uid=geteuid())) == NULL) {
		if (socks_useSyslog)
			syslog(LOG_LOW, "Unknown user-id %d\n",uid);
		else
			fprintf(stderr, "Unknown user-id %d\n",uid);
		exit(1);
	}
	strncpy(src_user, pw->pw_name, sizeof(src_user));

#if !defined(DNS_THROUGH_NIS)

	if ((ns = getenv("SOCKS_NS")) == NULL) {
#ifdef SOCKS_DEFAULT_NS
		ns = defaultNS;
#else
		;
#endif
	}
	if ((dp = getenv("SOCKS_DNAME")) == NULL) {
#ifdef SOCKS_DEFAULT_DNAME
		dp = defaultDNAME;
#else
		;
#endif
	}

	if ((ns != NULL) || (dp != NULL)) {
		res_init();
#ifdef sgi
		sethostresorder("local:nis:bind");
#endif
	}
#if defined(SOLARIS)
	(void)gethostbyname("dont.care");
	/* without this, the first call to gethostbyname() will
	 * wipe out what we put into the _res structure! */
#endif /* #if defined(SOLARIS) */

	if (ns != NULL) {
#ifdef ultrix
		_res.ns_list[0].addr.sin_addr.s_addr = inet_addr(ns);
#else
		_res.nsaddr_list[0].sin_addr.s_addr = inet_addr(ns);
#endif
		_res.nscount = 1;
	}
	if (dp != NULL) {
		strncpy(_res.defdname, dp, sizeof(_res.defdname)-1);
	}

/* >>> jon r. luini <jonl@hal.com> */
/*
#ifdef	SOCKS_DEFAULT_DNAME
	bzero (_res.defdname, sizeof (_res.defdname));

	if ( (cp = getenv("SOCKS_DNAME")) != NULL ) 
	{
	    strncpy (_res.defdname, cp, sizeof (_res.defdname)-1);
	}
	else
	{
	    strncpy (_res.defdname, SOCKS_DEFAULT_DNAME,
	        sizeof (_res.defdname)-1);
	}
#endif
*/
/* <<< jon r. luini <jonl@hal.com> */

#endif /* #if !defined(DNS_THROUGH_NIS) */

	if ((socks_def_server = getenv("SOCKS_SERVER")) == NULL)
		socks_def_server = defaultSERVER;
	else
		socks_def_server = socks_fw;
	socks_server = socks_def_server;
	if ((cp = getenv("SOCKS_BANNER")) != NULL) {
		if (socks_useSyslog)
			syslog(LOG_LOW, "SOCKS %s client. Default SOCKS server: %s\n",
			RELEASE, socks_def_server);
		else
			fprintf(stderr, "SOCKS  lient. Default SOCKS server: %s\n",
			RELEASE, socks_def_server);
	}

	socks_nsin.sin_family = AF_INET;
	socks_nsin.sin_port = htons(socks_fw_port);
/*
	if ((hp = gethostbyname(socks_server)) == NULL) {
		socks_nsin.sin_addr.s_addr = inet_addr(socks_server);
	} else {
		bcopy(hp->h_addr_list[0], &socks_nsin.sin_addr.s_addr, hp->h_length);
	}
*/
	if (stat(SOCKS_FC, &statfc) == 0)
        	socks_rdfz(SOCKS_FC, &scfAddr, &Nscf, &cfStrings, socks_useSyslog);
	else if (stat(SOCKS_CONF, &statcf) == 0)
        	socks_rdconf(SOCKS_CONF, &scfAddr, &Nscf, socks_useSyslog);
	else
		socks_no_conf = 1;
	return 0;
}


int socks_connect_sockd(sock)
int	sock;
/* returns 0 if successfully connected to a SOCKS server,
   returns -1 otherwise
 */
{
	int	last = 0;
	int	new_sock;
	struct hostent	*hp;
	char **cp;

	while (socks_server = socks_serverlist) {
		if (socks_serverlist = index(socks_serverlist, ','))
			*socks_serverlist = '\0';
		if (socks_GetQuad(socks_server, &socks_nsin.sin_addr) != -1) {
			cp = NULL;
		} else if ((hp = gethostbyname(socks_server)) == NULL) {
			break;
		} else {
			cp = hp->h_addr_list;
			bcopy(*cp++, &socks_nsin.sin_addr, IPADDRLENG);
		}
			
		while (1) {
			new_sock = socket(PF_INET, SOCK_STREAM, 0);
			if (new_sock < 0) {
				return -1;
			}
			if (connect(new_sock, (struct sockaddr *)&socks_nsin, sizeof(struct sockaddr_in)) == 0) {
				if (dup2(new_sock, sock) < 0) {
					close(new_sock);
					return -1;
				} else {
					close(new_sock);
					return 0;
				}
			} else {
				close(new_sock);
#ifdef EAGAIN
				if ((errno == EISCONN) || (errno == EINPROGRESS) || (errno == EAGAIN))
#else
				if ((errno == EISCONN) || (errno == EINPROGRESS))
#endif
					return -1;
			}
			if ((cp == NULL) || (*cp == NULL))
				break;
			bcopy(*cp++, &socks_nsin.sin_addr, IPADDRLENG);
		} 
		syslog(LOG_LOW, "Failed to connect to sockd at %s: %m", socks_server);
		if (!(socks_serverlist)) {
			return -1;
		}
		if (socks_serverlist) *socks_serverlist++ =',';
	}
	errno = ECONNREFUSED;
	return -1;
	
}


static int send_name(s, name)
int s;
char *name;
{
	char *p = name;
	int i, n, ret;
	fd_set	fds;
	int	fdsbits = s + 1;
	struct	timeval timeout;

	i = strlen(name) + 1;
	while ( i > 0) {
		FD_ZERO(&fds);
		FD_SET(s, &fds);
		timeout.tv_sec = 15;
		timeout.tv_usec = 0;
		if ((ret = select(fdsbits, NULL, &fds, NULL, &timeout)) < 0) {
			if (errno == EINTR)
				continue;
			return(-1);
		}
		if (ret == 0)
			continue;
		if((n = write(s, p, i)) <= 0) {
			return(-2);
		}
		p += n;
		i -= n;
	}
	return(0);
}


int socksC_proto(s, dst)
int s;
Socks_t	*dst;
{
	int sta;
	u_int32	addr;
	char *hostname;

	if ((sta = socks_SendDst(s, dst)) < 0) {
		if (socks_useSyslog)
			syslog(LOG_LOW, "Connection refused by SOCKS server %s\n", socks_server);
		else
			fprintf(stderr, "Connection refused by SOCKS server %s\n", socks_server);
		return(sta);
	}
	if ((sta = send_name(s, src_user)) < 0) {
		if (socks_useSyslog)
			syslog(LOG_LOW, "Connection refused by SOCKS server %s\n", socks_server);
		else
			fprintf(stderr, "Connection refused by SOCKS server %s\n", socks_server);
		return(sta);
	}

	addr = ntohl(dst->host);
	if ((addr >> 8) == 0) {
	/* Using fake IP, send destination hostname */
		if ((sta = send_name(s, socks_fakeIP[addr-1].h_name)) < 0) {
		if (socks_useSyslog)
				syslog(LOG_LOW, "Connection refused by SOCKS server %s\n", socks_server);
		else
				fprintf(stderr, "Connection refused by SOCKS server %s\n", socks_server);
		}
	}
		
	if ((sta = socks_GetDst(s, dst)) < 0) {
		if (socks_useSyslog)
			syslog(LOG_LOW, "Connection refused by SOCKS server %s\n", socks_server);
		else
			fprintf(stderr, "Connection refused by SOCKS server %s\n", socks_server);
		return(sta);
	}
	return(0);
}

static void quit_C_proto()
{
	exit(SOCKS_FAIL);
}

static void do_C_proto(sock, port, addr)
int sock;
unsigned short port;
u_int32 addr;
{
	Socks_t	dst;

	signal(SIGALRM, quit_C_proto);
	alarm(CLIENT_CONN_TIMEOUT);
	dst.version = SOCKS_VERSION;
	dst.cmd = SOCKS_CONNECT;
	dst.port = port;
	dst.host = addr;
	if (socksC_proto(sock, &dst) < 0) {
		alarm(0);
		exit(SOCKS_FAIL);
	}
	alarm(0);
	if ((dst.cmd == SOCKS_FAIL) || (dst.cmd == SOCKS_NO_IDENTD)
	    || (dst.cmd == SOCKS_BAD_ID)) {
		exit(dst.cmd);
	}
	exit(SOCKS_RESULT);

}


int Rconnect(sock, sin, size)
int			sock;
struct sockaddr_in	*sin;
int			size;
{
	Socks_t		dst;
	int	i;
	int	res_ret, con_ret, con_errno;
	u_int32 addr;
	int status, wait_ret, child_pid;


	if (sin->sin_family != AF_INET) {
		return connect(sock, (struct sockaddr *)sin, size);
	}

	if (socks_init_done == 0)
		SOCKSinit("SOCKSclient");

	if ((sock != socks_conn_sock) || (sin->sin_port != socks_conn_port)
	    || (sin->sin_addr.s_addr != socks_conn_host)) {
		if (socks_conn_init)
			kill(socks_conn_init, SIGKILL);
		socks_conn_code = 0;
		socks_conn_init = 0;
		strcpy(socks_cmd, "connect");
		if (socks_IPtohost(&sin->sin_addr, &socks_dstsh) < 0) {
			if (socks_useSyslog)
				fprintf(stderr, "Out of memory\n");
			else
				fprintf(stderr, "Out of memory\n");
			exit(1);
		}
		socks_dstsh.port = sin->sin_port;
		socks_porttoserv(sin->sin_port, dst_serv, sizeof(dst_serv));
	} else if (status = socks_conn_code) {
		socks_conn_init = 0;
		socks_conn_code = 0;
		socks_conn_sock = 0;
		socks_conn_port = 0;
		socks_conn_host = 0;
		res_ret = socks_check_result(status);
		if (status == SOCKS_RESULT) {
			errno = EISCONN;
			socks_last_conn_host = sin->sin_addr.s_addr;
			socks_last_conn_port = sin->sin_port;
		} else {
			syslog(LOG_LOW, "Connection failed.\n");
		}
		return -1;
	} else if (socks_conn_init) {
#if defined(NO_WAITPID)
		wait_ret = wait3(&status, WNOHANG, (struct rusage *) NULL);
#else
		wait_ret = waitpid(socks_conn_init, &status, WNOHANG);
#endif /* #if defined(NO_WAITPID) */
		if (wait_ret == 0) {
			errno = EALREADY;
			return -1;
		} else if (wait_ret == socks_conn_init) {
			socks_conn_init = 0;
			socks_conn_code = 0;
			socks_conn_sock = 0;
			socks_conn_port = 0;
			socks_conn_host = 0;
			if (status & 0x00ff) {
				kill(socks_conn_init, SIGKILL);
				errno = ECONNREFUSED;
				syslog(LOG_LOW, "Connection failed.\n");
				return -1;
			} else {
				status = (status >> 8) & 0x00ff;
				res_ret = socks_check_result(status);
				if (res_ret == 0) {
					errno = EISCONN;
					socks_last_conn_host = sin->sin_addr.s_addr;
					socks_last_conn_port = sin->sin_port;
				} else {
					syslog(LOG_LOW, "Connection failed.\n");
				}
				return -1;
			}
		} else {
			kill(socks_conn_init, SIGKILL);
			errno = ECONNREFUSED;
			socks_conn_init = 0;
			socks_conn_code = 0;
			socks_conn_sock = 0;
			socks_conn_port = 0;
			socks_conn_host = 0;
			syslog(LOG_LOW, "Connection failed.\n");
			return -1;
		}
	}


	if (socks_no_conf)
		socks_direct = SOCKS_DIRECT;
	else
		socks_direct = socks_ckcf(&socks_srcsh, &socks_dstsh, scfAddr, Nscf, socks_useSyslog);
	if (socks_direct == SOCKS_DENY) {
		syslog(LOG_LOW, "refused -- connect() from %s(%s) to %s (%s)",
			src_user, real_user, dst_name, dst_serv);
		errno = ECONNREFUSED;
		return -1;
	}

	if (socks_direct == SOCKS_DIRECT) {
		syslog(LOG_LOW, "connect() directly from %s(%s) to %s (%s)",
			src_user, real_user, dst_name, dst_serv);
		con_ret = connect(sock, (struct sockaddr *)sin, size);
		if (con_ret == 0) {
			socks_last_conn_host = sin->sin_addr.s_addr;
			socks_last_conn_port = sin->sin_port;
		}
		return(con_ret);
	}
	
	con_ret = socks_connect_sockd(sock);
	if (con_ret == 0) {
		syslog(LOG_LOW, "connect() from %s(%s) to %s (%s) using sockd at %s",
				src_user, real_user, dst_name, dst_serv, socks_server);
		dst.version = SOCKS_VERSION;
		dst.cmd = SOCKS_CONNECT;
		dst.port = sin->sin_port;
		dst.host = sin->sin_addr.s_addr;
		if (socksC_proto(sock, &dst) < 0) {
			return -1;
		}
		res_ret = socks_check_result(dst.cmd);
		if (res_ret == 0) {
			socks_last_conn_host = sin->sin_addr.s_addr;
			socks_last_conn_port = sin->sin_port;
		}
		return(res_ret);
	}
	if ((con_ret < 0) && (errno != EINPROGRESS)) {
		return -1;
	}
	syslog(LOG_LOW, "connect() from %s(%s) to %s (%s) using sockd at %s",
			src_user, real_user, dst_name, dst_serv, socks_server);

	switch (child_pid = fork()) {
	case -1:
		if (socks_useSyslog)
			syslog(LOG_LOW, "Rconnect(): cannot fork: %m\n");
		else
			perror("Rconnect(): fork()");
		errno = ECONNREFUSED;
		return -1;
	case 0:
		do_C_proto(sock, sin->sin_port, sin->sin_addr.s_addr);
	default:
		socks_conn_init = child_pid;
		socks_conn_code = 0;
		socks_conn_sock = sock;
		socks_conn_port = sin->sin_port;
		socks_conn_host = sin->sin_addr.s_addr;
		errno = EINPROGRESS;
		return -1;
	}
}
