/* shell_cmd */

 /*
  * socks_shell_cmd() takes a shell command template and performs
  * %x substitutions.
  * The result is executed
  * by a /bin/sh child process, with standard input, standard output and
  * standard error connected to /dev/null.
  * 
  * Diagnostics are reported through syslog(3).
  * 
  * Author: Wietse Venema, Eindhoven University of Technology, The Netherlands.
  *
  * Adapted for use with SOCKS by Ying-Da Lee, NEC Systems Lab, CSTC
  * ylee@syl.dl.nec.com
  *
  */

#ifndef lint
static char sccsid[] = "@(#) shell_cmd.c 1.2 92/06/11 22:21:28";
#endif

/* System libraries. */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <syslog.h>
#include "socks.h"

#if !defined(SOLARIS)
extern char *strncpy();
extern char *strchr();
#endif
#if defined(DGUX)
extern int closelog();
#else
extern void closelog();
#endif
extern void exit();

#define src_name srcshp->dmname[0]
#define src_user srcshp->user
#define real_user srcshp->ruser
#define dst_name dstshp->dmname[0]
#define dst_serv dstshp->portname
#define socks_cmd dstshp->user

/* Forward declarations. */

static void do_child();
static void percent_x();

/* socks_shell_cmd - expand %<char> sequences and execute shell command */

void    socks_shell_cmd(string, srcshp, dstshp)
char   *string;
struct sockshost_s	*srcshp, *dstshp;
{
    char    cmd[BUFSIZ];
    static char    alpha_num[] = "abcdefghijklmnopqrstuvwxyz\
ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int     child_pid;
    int     wait_pid;
    int     daemon_pid = getpid();

    percent_x(cmd, sizeof(cmd), string, srcshp, dstshp, daemon_pid);
    if (strpbrk(cmd, alpha_num) == NULL) {
	syslog(LOG_HIGH, "error -- shell command \"%s\" contains no alphanumeric characters.", cmd);
	return;
    }
    
    /*
     * Most of the work is done within the child process, to minimize the
     * risk of damage to the parent.
     */

    switch (child_pid = fork()) {
    case -1:					/* error */
	syslog(LOG_HIGH, "error -- socks_shell_cmd fork() %m");
	break;
    case 00:					/* child */
	do_child(daemon_pid, cmd);
	/* NOTREACHED */
    default:					/* parent */
	while ((wait_pid = wait((int *) 0)) != -1 && wait_pid != child_pid)
	     /* void */ ;
    }
}

/* do_child - exec command with { stdin, stdout, stderr } to /dev/null */

static void do_child(daemon_pid, command)
int   daemon_pid;
char   *command;
{
    char   *error = 0;
    int     tmp_fd;

    /*
     * Close a bunch of file descriptors. The Ultrix inetd only passes stdin,
     * but other inetd implementations set up stdout as well. Ignore errors.
     */

    closelog();
    for (tmp_fd = 0; tmp_fd < 10; tmp_fd++)
	(void) close(tmp_fd);

    /* Set up new stdin, stdout, stderr, and exec the shell command. */

    if (open("/dev/null", 2) != 0) {
	error = "open /dev/null: %m";
    } else if (dup(0) != 1 || dup(0) != 2) {
	error = "dup: %m";
    } else {
	(void) execl("/bin/sh", "sh", "-c", command, (char *) 0);
	error = "execl /bin/sh: %m";
    }

    /* We can reach the following code only if there was an error. */

#ifdef LOG_DAEMON
    (void) openlog("sockd", LOG_PID, SYSLOG_FAC);
#else
    (void) openlog("sockd", LOG_PID);
#endif
    syslog(LOG_HIGH, "Cannot execute shell command for pid %d", daemon_pid);
    exit(0);
}


 /*
  * percent_x() takes a string and performs %x subsitutions.
  * It aborts the program when the result of
  * expansion would overflow the output buffer. Because the result of %<char>
  * expansion is typically passed on to a shell process, characters that may
  * confuse the shell are replaced by underscores.
  * 
  * Diagnostics are reported through syslog(3).
  * 
  * Author: Wietse Venema, Eindhoven University of Technology, The Netherlands.
  *
  * Adapted for use with SOCKS by Ying-Da Lee, NEC Systems Lab, CSTC
  * ylee@syl.dl.nec.com
  *
  */


/* percent_x - do %<char> expansion, abort if result buffer is too small */

static void    percent_x(result, result_len, str, srcshp, dstshp, pid)
char   *result;
int     result_len;
char   *str;
struct sockshost_s	*srcshp, *dstshp;
int     pid;
{
    char   *end = result + result_len - 1;	/* end of result buffer */
    char   *expansion;
    int     expansion_len;
    char    pid_buf[10];
    char    port_buf[10];
    static char ok_chars[] = "1234567890!@%-_=+\\:,./\
abcdefghijklmnopqrstuvwxyz\
ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char   *cp;

    /*
     * %A: the client domainname if known, IP address otherwise
     * %a: the client IP address
     * %c: "connect" or "bind"
     * %p: the daemon or client program process id
     * %S: the service name (ftp, telnet,etc.) if known, port number otherwise
     * %s: the destination port number 
     * %U: for sockd, this is the username as reported by identd;
     *	   for client program, this is the name used at login
     * %u: for sockd, this is the username as reported by the client program;
     *	   for client program, this is the username of the effective userid
     * %Z: the destination domainname if known, IP address otherwise
     * %z: the destination IP address
     *
     * %% becomes a %, and %other is ignored. We terminate with a diagnostic if
     * we would overflow the result buffer. Characters that may confuse the
     * shell are mapped to underscores.
     */

    while (*str) {
	if (*str == '%') {
	    str++;
	    expansion =
		*str == 'A' ? (str++, src_name) :
		*str == 'a' ? (str++, inet_ntoa(srcshp->shipaddr[0])) :
		*str == 'c' ? (str++, socks_cmd) :
		*str == 'p' ? (str++, sprintf(pid_buf, "%d", pid), pid_buf) :
		*str == 'S' ? (str++, dst_serv) :
		*str == 's' ? (str++, sprintf(port_buf, "%u", ntohs(dstshp->port)), port_buf) :
		*str == 'U' ? (str++, real_user) :
		*str == 'u' ? (str++, src_user) :
		*str == 'Z' ? (str++, dst_name) :
		*str == 'z' ? (str++, inet_ntoa(dstshp->shipaddr[0])) :
		*str == '%' ? (str++, "%") :
		*str == 0 ? "" : (str++, "");
	    expansion_len = strlen(expansion);
	    for (cp = expansion; *cp; cp++)
		if (strchr(ok_chars, *cp) == 0)
		    *cp = '_';
	} else {
	    expansion = str++;
	    expansion_len = 1;
	}
	if (result + expansion_len >= end) {
	    syslog(LOG_HIGH, "shell command too long: %30s...", result);
	    exit(0);
	}
	strncpy(result, expansion, expansion_len);
	result += expansion_len;
    }
    *result = 0;
}
