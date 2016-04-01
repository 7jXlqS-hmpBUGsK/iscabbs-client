/*
 * This is where I've put all the #include files, to keep them separate in a
 * single location.  Pure C stuff goes here, the system-specific stuff is kept
 * over in unix.h. 
 */

#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

#define INTVERSION	240
#include "config.h"

#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <assert.h>
#include <limits.h>

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

#ifdef HAVE_OPENSSL
#include <openssl/ssl.h>
#endif

/* The __attribute__ macro is GCC-specific */
#ifndef __GNUC__
#define __attribute__(X)        /* empty */
#endif

/* Workaround for buggy glibc 2.1+ */
#define USE_POSIX_SIGSETJMP 1

#include <errno.h>
/* extern int errno; */

#include "string_buf.h"

typedef struct {
    char   *start;              /* Pointer to beginning of queue */
    int     head;               /* Index of current head */
    int     tail;               /* Index of current tail */
    int     size;               /* Number of objects queue can hold */
    int     nobjs;              /* Number of objects queued */
    int     objsize;            /* Size of one object */
} queue;

#define CTRL_D		4
#define TAB		9
#define CTRL_R		18
#define CTRL_U		21
#define CTRL_W		23
#define CTRL_X		24
#define CTRL_Z		26
#define ESC		27
#define DEL		127

#define BBSHOST		"bbs.iscabbs.com"
#define BBSIPNUM	"162.220.12.126"
#define BBSPORT		23
#define SSLPORT		992

/* SendingX defines */
enum SendingX_Type {
    SX_NOT = 0, SX_SENT_x = 1, SX_SENT_NAME = 2, SX_REPLYING = 3, SX_WANT_TO = 5, SX_SEND_NEXT = 8
};

/* Color transform defines */
#define CX_NORMAL	0
#define CX_POST		1
#define CX_EXPRESS	2
#define CX_INFO		3           /* not yet used */

#define MAXLAST		20
typedef struct {
    int     posting:1;          /* true if user is currently posting */
    int     lastsave:1;         /* true if last time user edited they saved */
    int     check:1;            /* true if waiting to check BBS for X's */
    int     configflag:1;       /* true if we are in bbsrc config funcs */
    int     useansi:1;          /* true if BBS is in ANSI color mode */
    int     usebold:1;          /* true if using bold in ANSI color mode */
    int     offbold:1;          /* true if we need to force bold ANSI off */
    int     moreflag:1;         /* true if we are inside a MORE prompt */
    int     squelchpost:1;      /* true if we should squelch enemy posts */
    int     squelchexpress:1;   /* true if we should squelch enemy express */
    int     ansiprompt:1;       /* true if we automatically answer ANSI ? */
    int     browserbg:1;        /* true if browser can be backgrounded */
    int     autofix_posts:1;
} Flags;

typedef struct {
    int     (*sortfn) ();       /* function to sort list; see slist.c */
    size_t  nitems;             /* number of items in list */
    void  **items;              /* dynamic array containing item pointers */
} slist;

typedef struct {
    int     magic;              /* Magic number */
    char    name[21];           /* User name */
    char    info[54];           /* Friend description */
    time_t  time;               /* Time online */
} Friend;                       /* User list entry */

/* The ordering of this struct is important!  Do not change it! IO ERROR */
typedef struct {
    char    text;               /* Plain text color */
    char    forum;              /* Forum prompt color */
    char    number;             /* Numbers and Read cmd prompt color */
    char    err;                /* Warning/error messages color */
    char    reserved1;
    char    reserved2;
    char    reserved3;
    char    postdate;           /* Post date stamp color */
    char    postname;           /* Post author name color */
    char    posttext;           /* Post text color */
    char    postfrienddate;     /* Post friend date stamp color */
    char    postfriendname;     /* Post friend name color */
    char    postfriendtext;     /* Post friend text color */
    char    anonymous;          /* Anonymous post header color */
    char    moreprompt;         /* More prompt color */
    char    reserved4;
    char    reserved5;
    char    background;         /* Background color */
    char    input1;             /* Text input fields */
    char    input2;             /* Text input fields (highlight) */
    char    expresstext;        /* X message text color */
    char    expressname;        /* X message name color */
    char    expressfriendtext;  /* X message from friend text color */
    char    expressfriendname;  /* X message from friend name color */
} Color;

#include "proto.h"

#endif /* DEFS_H_INCLUDED */
