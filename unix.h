/*
 * This is where all the system-specific #include files go, and all the #ifdefs
 * for portability to different Unix systems belong here and in unix.c. 
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_OPENSSL
#include <openssl/err.h>
#endif

#if defined(sun) && defined(unix) && !defined(FIONREAD) && !defined(__svr4__)
#define __svr4__
#endif

#ifdef M_XENIX
#define _IBCS2
#endif

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#else
#ifdef HAVE_SGTTY_H
#include <sgtty.h>
#endif
/* If neither is present, punt */
#endif

#ifdef _AIX
#include <sys/select.h>
#endif

#ifdef NeXT
#include <libc.h>
#endif

#if defined(AMIX) || defined(__svr4__)
#include <sys/filio.h>
#endif
