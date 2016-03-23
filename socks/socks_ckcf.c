/* socks_ckcf */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <ctype.h>
#if (defined(sun) && !defined(SOLARIS)) || defined(sgi)
#include <strings.h>
#else
#include <string.h>
#endif
#include "socks.h"

extern	int	socks_check_addr();
extern	int	socks_check_user();
extern char *socks_def_server;
extern char *socks_serverlist;
extern struct in_addr socks_self;


int socks_ckcf(srcshp, dstshp, cfAddr, Ncf, useSyslog)
/* Return 0 if sockd should be used,
	  1 if direct connection should be made,
	 -1 if the connection request should be denied.
 */
struct sockshost_s *srcshp, *dstshp;
struct config *cfAddr;
int Ncf;
int useSyslog;
{
	unsigned short	dst_sin_port = ntohs(dstshp->port); 
	int i;
	struct config *cp;
  
  
 	if ((dstshp->shipaddr[0].s_addr == socks_self.s_addr) || (dstshp->shipaddr[0].s_addr == 0))
 		return SOCKS_DIRECT;

	for (i = 0, cp = cfAddr; i++ < Ncf; cp++) {

		socks_serverlist = cp->serverlist;
	/* comparisons of port numbers must be done in host order */

		if (socks_ckadr(dstshp, cp->ddomain, &cp->daddr, &cp->dmask) &&
		    socks_ckusr(cp->userlist, srcshp->user, useSyslog) &&
		    socks_ckprt(cp->tst, dst_sin_port, cp->dport))
			goto GotIt;
	}

	return SOCKS_DENY;

GotIt:
	if ((socks_serverlist == NULL) || (*socks_serverlist == '\0'))
		socks_serverlist = socks_def_server;
	if (cp->cmdp != NULL)
		socks_shell_cmd(cp->cmdp, srcshp, dstshp);
	return cp->action;
	
}

