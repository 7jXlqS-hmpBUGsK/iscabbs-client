/*
 * This is the home of all global variables (see global.c) 
 */

#ifndef EXT_H_INCLUDED
#define EXT_H_INCLUDED

extern char escape[2];
extern Flags flags;             /* Miscellaneous flags */
extern Color color;             /* color transformations */
extern char lastname[MAXLAST][21];  /* last username received in post or X */
extern bool away;               /* away from keyboard? */
extern bool xland;              /* X Land - auto-fill-in-recipient */
extern queue *xlandQueue;       /* X Land name queue */
extern char autoname[21];       /* Automatic login name */

extern char autopasswd[21];     /* Automatic password */
extern bool autopasswdsent;     /* Set after password sent to BBS */
extern bool autologgedin;       /* Has autologin been done? */
extern enum SendingX_Type SendingX; /* automatically sending an X? */
extern int replyrecip[21];      /* reply recipient for autoreply */
extern char bbsrcname[PATH_MAX];    /* bbsrc filename (usually ~/.bbsrc) */
extern FILE *bbsrc;             /* file descriptor of .bbsrc file */
extern bool bbsrcro;            /* set if .bbsrc file is read-only */
extern char tempfilename[PATH_MAX]; /* bbstmp filename (usually ~/.bbstmp) */
extern FILE *tempfile;          /* file pointer to above */
extern char username[80];       /* username */
extern char editor[80];         /* name of editor to invoke */
extern char myeditor[80];       /* name of user's preferred editor */

extern pid_t childpid;          /* process id of child process */

#ifdef USE_POSIX_SIGSETJMP
extern sigjmp_buf jmpenv;       /* Yuck!  I have to use longjmp!  Gag! */
#else
extern jmp_buf jmpenv;          /* Yuck!  I have to use longjmp!  Gag! */
#endif

extern char bbshost[64];        /* name of bbs host (bbs.isca.uiowa.edu) */
extern char cmdlinehost[64];    /* name of bbs host from command line */
extern unsigned short bbsport;  /* port to connect to (23 or 992) */
extern unsigned short cmdlineport;  /* port to connect to from command line */
extern bool want_ssl;           /* Whether the connection should be secure */
extern bool is_ssl;             /* Whether the current connection is secured */

extern slist *friendList;       /* 'friend' list */
extern slist *enemyList;        /* 'enemy' list */
extern slist *whoList;          /* saved who list */
extern char saveheader[160];    /* for saving our message header */

extern char macro[128][72];     /* array for macros */
extern char awaymsg[6][80];     /* Away from keyboard message */
extern int commandkey;          /* hotkey for signalling a macro follows */
extern int quitkey;             /* hotkey to quit (commandkey quitkey) */
extern int suspkey;             /* hotkey for suspending (" suspkey) */
extern int capturekey;          /* Toggle text capture key (" capturekey) */
extern int capture;             /* Capture status */
extern int shellkey;            /* hotkey for shelling out (" shellkey) */
extern char shell[PATH_MAX];    /* User's preferred shell */
extern int awaykey;             /* Hotkey for away from keyboard */
extern int browserkey;          /* Hotkey to spawn a Web browser */
extern char browser[PATH_MAX];  /* User's preferred Web browser */
extern char defaultbrowser[PATH_MAX];   /* Detected system default browser */
extern bool textonly;           /* Don't use GUI features of the host OS */

#ifdef HAVE_OPENSSL
extern SSL *ssl;                /* SSL connection */
#endif
extern int net;                 /* file descriptor of network socket */
extern FILE *netofp;            /* file pointer for output to net */
extern unsigned char netibuf[2048]; /* buffer for input from net */

/* This is necessary because we aren't using buffered input under VMS */
extern unsigned char *netifp;   /* buffer pointer for input from net */
extern int netilen;             /* length of current input buffer from net */
extern unsigned char ptyibuf[1024]; /* buffer for input from pty */
extern unsigned char *ptyifp;   /* buffer pointer for input from pty */
extern int ptyilen;             /* length of current input buffer from pty */

extern int rows;                /* number of rows on user's screen */
extern int oldrows;             /* previous value of rows */
extern char lastcolor;          /* last color code received from BBS */

extern long byte;               /* current byte (remotely synched with bbs) */
extern long targetbyte;         /* where the client wants to get */
extern long bytep;              /* where the client is */
extern unsigned char save[1000];    /* buffer to save past user bytes */

extern char bbsfriendsname[PATH_MAX];   /* added friends file name */
extern FILE *bbsfriends;        /* added friends file */

                /* for saved who list */
extern char parsing[400];       /* for incoming X messages */
extern bool login_shell;        /* whether this client is a login shell */
extern char keymap[128];        /* key remapping array */
extern int version;             /* Client version number */

/* Below variables were removed from telnet.c telrcv() */
extern bool recving_wholist;    /* Are we currently receiving a who list? */
extern char postbuf[160];       /* Buffer for post header (for kill files) */
extern int postbufp;            /* Pointer for post header buffer */
extern char xmsgbuf[580];       /* Buffer for X message */
extern int highxmsg;            /* Highest X message counter */
extern char *xmsgbufp;          /* Pointer for X message buffer */
extern int xmsgflag;            /* True if X message header being received */
extern int postnow;             /* True while we are receiving a post */
extern int xmsgnow;             /* True while we are receiving an X message */
extern bool needx;
extern int eatline;

extern bool use_socks;          /* Use a SOCKS firewall server */
extern char socks_fw[128];      /* Name of SOCKS firewall server */
extern unsigned short socks_fw_port;    /* Port number of SOCKS firewall server */
extern queue *urlQueue;         /* Structure holding recently seen URLs */

#endif /* EXT_H_INCLUDED */
