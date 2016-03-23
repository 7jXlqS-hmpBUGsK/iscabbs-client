#include <sys/types.h>
#include <netinet/in.h>

/*
 * Default SOCKS server host; you MUST set this for your site.
 * This is overridden at run time by the contents of environment
 * variable SOCKS_SERVER if it exists.
 */
#define SOCKS_DEFAULT_SERVER	"firewall"

/*
 * Default Domain Nameserver for the SOCKS clients.
 * Leave it undefined if all your client mechines can do general
 * DNS lookup for all Internet hosts correctly with the DNS servers
 * specified in /etc/resolv.conf.
 * Otherwise, define it using the IP ADDRESS (NOT NAME!) of a DNS
 * server which can resolve all Internet hosts and which is IP-reachable
 * from your client machines.
 * This is overriden at run time by the contents of environment
 * variable SOCKS_NS if it exists.
 */
/* #define SOCKS_DEFAULT_NS	"1.2.3.4" */

/* >>> jon r. luini <jonl@hal.com> */
/*
 * Default domain name to use for the resolver to use.
 * Leave it undefined unless you run in an environment where
 * you have a number of clients which will be running the socks
 * utilities without the correct domain name specified in /etc/resolv.conf
 * (or determined automatically by the system). If you try to run
 * socks and it complains that it cannot lookup the local hostname,
 * that is a good indication you need to define this appropriately.
 * This is overriden at run time by the contents of environment
 * variable SOCKS_DNAME if it exists.
 */
/* #define       SOCKS_DEFAULT_DNAME     "hal.COM" */
/* <<< jon r. luini <jonl@hal.com> */

/*
 * Full pathname of the regular 'finger' program.
 * You will have to rename your
 * regular 'finger' program to something else, e.g., from /usr/ucb/finger
 * to /usr/ucb/finger.orig and the pathname you should use here is the
 * new (altered) pathname, i.e., /usr/ucb/finger.orig.
 */
#define ORIG_FINGER	"/usr/ucb/finger.orig"
/* Overridden at runtime by environment variable ORIG_FINGER if it exists. */

/* Control file for clients */
#define SOCKS_CONF	"/etc/socks.conf"
#define	SOCKS_FC	"/etc/socks.fc"

/*
 * Default port number for SOCKS services.
 * On the SOCKS server host, if the server is under inetd control,
 * then the port must be specified in socks/tcp entry in /etc/services.
 * For servers not under inetd control and for all clients,
 * the port number is obtained from socks/tcp entry in /etc/services if
 * it exists, otherwise the number defined by SOCKS_DEF_PORT will be used.
 */
#define SOCKS_DEF_PORT	1080

/*
**  How long (in seconds) to keep a connection around while it is idle
*/
#define SOCKS_TIMEOUT	2*60*60	/* 2hr in seconds */

/* How long before connection attempts timed out */
#define CLIENT_CONN_TIMEOUT 60*2 /* 2 minutes */
#define SOCKD_CONN_TIMEOUT 60*3 /* 3 minutes */
/* You may have to adjust these to fit your network situation */

/*
 * Where the config file lives on the SOCKS server host.
 * This is the file that controls access to the SOCKS server
 * and its services.
 */
#define SOCKD_CONF	"/etc/sockd.conf"
#define SOCKD_FC	"/etc/sockd.fc"

/*
 * Define this if your SOCKS server is multi-homed (i.e.,
 * having two or more network interfaces) and is not behaving
 * as a router (i.e., has its IP forwarding turned off).
 * Leave it undefined otherwise.
 * Has no effect is NO_RBIND is defined in the top Makefile.
 */
#define MULTIHOMED_SERVER

/*
 * For multi-homed servers, you must supply the file /etc/sockd.route
 * to tell the program which interface to use for communicating with
 * which destination networks/hosts. See sockd man pages for details.
 * This has no effects if NO_RBIND (in top Makefile) is defined or
 * if MULTIHOMED_SERVER is undefined.
 */
#define SOCKD_ROUTE_FILE "/etc/sockd.route"
#define SOCKD_FROUTE_FILE "/etc/sockd.fr"

/* Current SOCKS protocol version */
#define SOCKS_VERSION	4

#define SOCKS_REPLY_VERSION 0

#define RELEASE	"4.3"

/*
**  Response commands/codes
*/
#define SOCKS_CONNECT	1
#define SOCKS_BIND	2
#define SOCKS_RESULT	90
#define SOCKS_FAIL	91
#define SOCKS_NO_IDENTD	92 /* Failed to connect to Identd on client machine */
#define SOCKS_BAD_ID	93 /* Client's Identd reported a different user-id */
  
#if defined(__alpha)
typedef unsigned int u_int32;
#else
typedef unsigned long u_int32;
#endif

typedef struct {
	u_int32			host; /* in network byte order */
	unsigned short		port; /* in network byte oreder */
	unsigned char		version;
	unsigned char		cmd;
} Socks_t;

typedef enum portcmp Portcmp;
enum portcmp { e_lt, e_gt, e_eq, e_neq, e_le, e_ge, e_nil };

/*
 * Define STAND_ALONE_SERVER if you want a standalone SOCKS server,
 * one which is not under the control of inetd.
 * This can drastically improve the performance if you have to use
 * a large sockd.conf file and especially if you are not using
 * frozen configuration files.
 */
/* #define STAND_ALONE_SERVER */

/* Location of the pid file of the running sockd. Meaningful only
 * when STAND_ALONE_SERVER is defined.
 */
#define PID_FILE "/etc/sockd.pid"

#define BAD_ID_STR	"#BAD_ID:"
#define NO_IDENTD_STR	"#NO_IDENTD:"

/* structure for caching configurations.  this improves performance in
 * clients or in servers * when STAND_ALONE_SERVER is defined.
 * Also used in the SOCKS library.
 */
 
struct config {
	char *userlist, *serverlist;
	int action;
	int use_identd;
	Portcmp tst;
	struct in_addr saddr,	/* source addr, or  */
				/* output interface for route file */
		smask,		/* source mask */
		daddr,		/* destination addr */
		dmask;		/* destination mask */
	unsigned short dport;
	char *cmdp, *sdomain, *ddomain;
};


/* for the action field */
#define SOCKS_DIRECT	1
#define SOCKS_SOCKD	0
#define SOCKS_DENY	-1
#define SOCKD_DENY	0
#define SOCKD_PERMIT	1
#define BAD_ID		5
#define NO_IDENTD	6

#ifdef MAKEFC
#define CONF_INCR	1000	/* step increment for realloc */
#else
#define CONF_INCR	100	/* step increment for realloc */
#endif /* #ifdef MAKEFC */

/*
 * Maximum number of concurrent requests a SOCKS server will accept.
 * Meaningful only if the server is not under the control
 * of inetd, i.e., when STAND_ALONE_SERVER is defined.
 */
#define MAX_CLIENTS	5


#ifdef SOLARIS
/* for bcopy(), bzero() and bcmp() */
#include "bstring.h"
#endif

/* Define NO_SYSLOG to suppress logging */
/* #define NO_SYSLOG */

#if defined(NO_SYSLOG)
# define syslog
# define openlog
#endif

#define SYSLOG_FAC	LOG_DAEMON
/* #define SYSLOG_FAC	LOG_LOCAL0 */
#define LOG_LOW		LOG_NOTICE
#define LOG_HIGH	LOG_ERR

/* The following struct linger declaration seemed to be
 * missing from older versions of LINUX but is present in
 * current.  If you need it, you must define NEED_STRUCT_LINGER
 * in the top level Makefile.
 */

#ifdef NEED_STRUCT_LINGER
/*
 * Structure used for manipulating linger option.
 */
struct  linger {
        int     l_onoff;                /* option on/off */
        int     l_linger;               /* linger time */
};
#endif /* ifdef NEED_STRUCT_LINGER */

#define IPADDRLENG 4

#define MAXIPPERHOST 20
#define MAXNAMESPERHOST 20
#define NAMELEN 128

struct sockshost_s {
	char *dmname[MAXNAMESPERHOST];
	struct in_addr	shipaddr[MAXIPPERHOST];
	unsigned short port; /* in network order */
	char portname[NAMELEN];
	char user[NAMELEN];
	char ruser[NAMELEN];
};

#define STREQ(a,b) (strcasecmp(a,b) == 0)
#define IDENTD_TIMEOUT 15
#define NUMFAKEIP 20	/* must be <= 254 */
#define NUMHOSTENT 20
