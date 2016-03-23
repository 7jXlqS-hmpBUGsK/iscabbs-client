/* SendGetDst */

#include <sys/types.h>
#if defined(ISC)
#include <sys/bsdtypes.h>
#endif /* #if defined(ISC) */
#include <sys/time.h>
#include <syslog.h>
#include <errno.h>
#if defined(ISC)
#include <net/errno.h>
#endif /* #if defined(ISC) */
#include "socks.h"

#ifndef NULL
#define NULL	0L
#endif

int socks_SendDst(s, dst)
int	s;
Socks_t	*dst;
{
	char c[sizeof(Socks_t)];
	char *p = c;
	int i = sizeof(Socks_t), n, ret;
	fd_set	fds;
	int	fdsbits = s + 1;
	struct	timeval timeout;

	c[0] = dst->version;
	c[1] = dst->cmd;
	bcopy(&dst->port, c+2, sizeof(dst->port));
	bcopy(&dst->host, c+2+sizeof(dst->port), sizeof(dst->host));

	while ( i > 0) {
		FD_ZERO(&fds);
		FD_SET(s, &fds);
		timeout.tv_sec = 15;
		timeout.tv_usec = 0;
		if ((ret = select(fdsbits, NULL, &fds, NULL, &timeout)) == 0)
			continue;
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			else {
/*
				syslog(LOG_LOW, "select() in socks_SendDst(): %m");
*/
				return -1;
			}
		}
		if((n = write(s, p, i)) > 0) {
			p += n;
			i -= n;
		} else if ((n < 0) && ((errno == EWOULDBLOCK) || (errno == EINTR)))
			continue;
		else {
/*
			syslog(LOG_LOW, "write() in socks_SendDst(): %m");
*/
			return (-2);
		}
	}
	return 0;
}

int socks_GetDst(s, dst)
int	s;
Socks_t	*dst;
{
	char	c[sizeof(Socks_t)];
	char	*p = c;
	int	i = sizeof(Socks_t), n, ret;
	fd_set	fds;
	int	fdsbits = s + 1;
	struct	timeval timeout;

	while ( i > 0) {
		FD_ZERO(&fds);
		FD_SET(s, &fds);
		timeout.tv_sec = 15;
		timeout.tv_usec = 0;
		if ((ret = select(fdsbits, &fds, NULL, NULL, &timeout)) == 0)
			continue;
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			else {	
/*
				syslog(LOG_LOW, "select() in socks_GetDst(): %m");
*/
				return -1;
			}
		}
		if((n = read(s, p, i)) > 0) {
			p += n;
			i -= n;
		} else if ((n < 0) && ((errno == EWOULDBLOCK) || (errno == EINTR)))
			continue;
		else {
/*
			syslog(LOG_LOW, "read() in socks_GetDst(): %m");
*/
			return -2;
		}
	}

	dst->version = c[0];
	dst->cmd = c[1];
	bcopy(c+2, &dst->port, sizeof(dst->port));
	bcopy(c+2+sizeof(dst->port), &dst->host, sizeof(dst->host));
	return 0;
}

