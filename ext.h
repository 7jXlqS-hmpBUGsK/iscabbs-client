/*
 * This is the home of all global variables (see global.c) 
 */

#ifndef EXT_H_INCLUDED
#define EXT_H_INCLUDED

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
extern char bbsrcname[PATH_MAX];    /* bbsrc filename (usually ~/.bbsrc) */
extern FILE *bbsrc;             /* file descriptor of .bbsrc file */
extern bool bbsrcro;            /* set if .bbsrc file is read-only */
extern char tempfilename[PATH_MAX]; /* bbstmp filename (usually ~/.bbstmp) */
extern FILE *tempfile;          /* file pointer to above */
extern char editor[80];         /* name of editor to invoke */
extern char myeditor[80];       /* name of user's preferred editor */
extern pid_t childpid;          /* process id of child process */
extern char bbshost[64];        /* name of bbs host (bbs.isca.uiowa.edu) */
extern char cmdlinehost[64];    /* name of bbs host from command line */
extern unsigned short bbsport;  /* port to connect to (23 or 992) */
extern unsigned short cmdlineport;  /* port to connect to from command line */
extern bool want_ssl;           /* Whether the connection should be secure */
extern bool is_ssl;             /* Whether the current connection is secured */

extern UList friendList;        /* 'friend' list */
extern UList enemyList;         /* 'enemy' list */
extern UList whoList;           /* saved who list */
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
extern bool textonly;           /* Don't use GUI features of the host OS */

#ifdef HAVE_OPENSSL
extern SSL *ssl;                /* SSL connection */
#endif
extern int net;                 /* file descriptor of network socket */
extern FILE *netofp;            /* file pointer for output to net */


extern int rows;                /* number of rows on user's screen */
extern int oldrows;             /* previous value of rows */
extern char lastcolor;          /* last color code received from BBS */

extern long sync_byte;          /* current byte (remotely synched with bbs) */
extern long targetbyte;         /* where the client wants to get */
extern long bytep;              /* where the client is */
extern unsigned char save[1000];    /* buffer to save past user bytes */

extern char bbsfriendsname[PATH_MAX];   /* added friends file name */
extern FILE *bbsfriends;        /* added friends file */

                /* for saved who list */
extern char parsing[2048];      /* for incoming X messages */
extern bool login_shell;        /* whether this client is a login shell */
extern char keymap[128];        /* key remapping array */
extern int version;             /* Client version number */

/* Below variables were removed from telnet.c telrcv() */
extern int recving_wholist;     /* Are we currently receiving a who list? */
extern char xmsgbuf[2048];      /* Buffer for X message */
extern int highxmsg;            /* Highest X message counter */
extern char *xmsgbufp;          /* Pointer for X message buffer */
extern bool postnow;            /* True while we are receiving a post */
extern bool xmsgnow;            /* True while we are receiving an X message */
extern bool needx;
extern int eatline;

extern queue *urlQueue;         /* Structure holding recently seen URLs */

extern string *scratch;         /* general purpose scratch buffer. Use as needed. */
#endif /* EXT_H_INCLUDED */
