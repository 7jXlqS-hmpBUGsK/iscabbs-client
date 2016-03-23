/*
 * Everything Unix/system specific goes in this file.  If you are looking to
 * port to some system the code currently doesn't work on, most if not all of
 * your problems should restricted to this file.
 *
 * This file covers Unix and Windows (cygwin) builds.  The Windows-specific
 * stuff is located in windows.c.
 */
#define _IN_UNIX_C
#include "defs.h"
#include "ext.h"
#include "unix.h"

#ifdef USE_CYGWIN
#include <w32api/windows.h>
#include <w32api/winuser.h>
#else
static struct passwd *pw;
#endif

#ifdef HAVE_OPENSSL
SSL_CTX *ctx;


void
killSSL (void)
{
    SSL_shutdown (ssl);
    SSL_free (ssl);
}


int
initSSL (void)
{
    SSL_METHOD *meth;

    SSL_load_error_strings ();
    SSLeay_add_ssl_algorithms ();
    meth = SSLv23_client_method ();
    ctx = SSL_CTX_new (meth);

    if (!ctx) {
        printf ("%s\n", ERR_reason_error_string (ERR_get_error ()));
        exit (1);
    }

    ssl = SSL_new (ctx);
    if (!ssl) {
        printf ("SSL_new failed\n");
        return 0;
    }

#ifdef DEBUG
    printf ("SSL initialized\n");
#endif
    return 1;
}
#endif /* HAVE_OPENSSL */

/*
 * Wait for the next activity from either the user or the network -- we ignore
 * the user while we have a child process running.  Returns 1 for user input
 * pending, 2 for network input pending, 3 for both.
 */
int
waitnextevent ()
{
    fd_set  fdr;
    int     result;

    for (;;) {
        FD_ZERO (&fdr);
        if (!childpid && !flags.check)
            FD_SET (0, &fdr);
        if (!ignore_network)
            FD_SET (net, &fdr);

        if (select (ignore_network ? 1 : net + 1, &fdr, 0, 0, 0) < 0) {
            if (errno == EINTR) {
                continue;
            }
            else {
                std_printf ("\r\n");
                fatalperror ("select", "Local error");
            }
        }

        if ((result = ((FD_ISSET (net, &fdr) != 0) << 1 | (FD_ISSET (0, &fdr) != 0))))
            return (result);
    }
}


/*
 * Find the user's home directory (needed for .bbsrc and .bbstmp)
 */
void
findhome ()
{
#ifdef USE_CYGWIN
    if (getenv ("USERNAME")) {
        strcpy (user, (char *) getenv ("USERNAME"));
        strcat (user, "  (Win32)");
    }
    else
        strcpy (user, "No username!  (Win32)");
#else
    if ((pw = getpwuid (getuid ())))
        strcpy (user, pw->pw_name);
    else if (getenv ("USER"))
        strcpy (user, (char *) getenv ("USER"));
    else
        fatalexit ("findhome: You don't exist, go away.", "Local error");
#endif /* USE_CYGWIN */
    if (login_shell)
        strcat (user, "  (login shell)");
}


/*
 * Locate the bbsrc file.  Usually is ~/.bbsrc, can be overriden by providing
 * an argument to the command that invokes this client.  If the argument is not
 * provided the BBSRC environment will specify the name of the BBSRC file if it
 * is set.  Returns a pointer to the file via openbbsrc().
 */
FILE   *
findbbsrc ()
{
    FILE   *f;

    if (login_shell)
        sprintf (bbsrcname, "/tmp/bbsrc.%d", getpid ());
    else {
        if (getenv ("BBSRC"))
            strcpy (bbsrcname, (char *) getenv ("BBSRC"));
#ifdef USE_CYGWIN
        else if (getenv ("USERPROFILE"))
            sprintf (bbsrcname, "%s/bbs.rc", (char *) getenv ("USERPROFILE"));
        else {
            if (!getcwd (bbsrcname, sizeof bbsrcname)) {
                fatalperror ("findbbsrc: getcwd", "Local error");
            }
            strcat (bbsrcname, "/bbs.rc");
        }
        move_if_needed ("c:\\.bbsrc", bbsrcname);
#else
        else if (pw)
            sprintf (bbsrcname, "%s/.bbsrc", pw->pw_dir);
        else if (getenv ("HOME"))
            sprintf (bbsrcname, "%s/.bbsrc", (char *) getenv ("HOME"));
        else
            fatalexit ("findbbsrc: You don't exist, go away.", "Local error");
#endif /* USE_CYGWIN */
    }
    if ((f = fopen (bbsrcname, "r")) && chmod (bbsrcname, 0600) < 0)
        s_perror ("Can't set access on bbsrc file", "Warning");
    if (f)
        fclose (f);
    return (openbbsrc ());
}



/* Added by Dave (Isoroku).  Finds .bbsfriends for friends list */
/* Edited by IO ERROR.  We read-only the .bbsfriends now, if it exists. */
FILE   *
findbbsfriends ()
{
    if (login_shell)
        sprintf (bbsfriendsname, "/tmp/bbsfriends.%d", getpid ());
    else {
        if (getenv ("BBSFRIENDS"))
            strcpy (bbsfriendsname, (char *) getenv ("BBSFRIENDS"));
#ifdef USE_CYGWIN
        else if (getenv ("USERPROFILE"))
            sprintf (bbsfriendsname, "%s/bbs.friends", (char *) getenv ("USERPROFILE"));
        else {
            if (!getcwd (bbsfriendsname, sizeof bbsrcname)) {
                fatalperror ("findbbsfriends: getcwd", "Local error");
            }
            strcat (bbsfriendsname, "/.bbsfriends");
        }
#else
        else if (pw)
            sprintf (bbsfriendsname, "%s/.bbsfriends", pw->pw_dir);
        else if (getenv ("HOME"))
            sprintf (bbsfriendsname, "%s/.bbsfriends", (char *) getenv ("HOME"));
        else
            fatalexit ("findbbsfriends: You don't exist, go away.", "Local error");
#endif /* USE_CYGWIN */
    }
    chmod (bbsfriendsname, 0600);
    return (openbbsfriends ());
}



/*
 * Truncates bbsrc file to the specified length.
 */
void
truncbbsrc (len)
     int     len;
{
    /* Anyone know how to do this in SCO/Xenix?  If so, please let me know! */
#ifndef M_XENIX
    if (ftruncate (fileno (bbsrc), len) < 0)
        fatalexit ("ftruncate", "Local error");
#endif
}



/*
 * Opens the temp file, ~/.bbstmp.  If the BBSTMP environment variable is set,
 * that file is used instead.
 */
void
opentmpfile ()
{
    if (login_shell)
        sprintf (tempfilename, "/tmp/bbstmp.%d", getpid ());
    else {
        if (getenv ("BBSTMP"))
            strcpy (tempfilename, (char *) getenv ("BBSTMP"));
#ifdef USE_CYGWIN
        else if (getenv ("USERPROFILE"))
            sprintf (tempfilename, "%s\\bbstmp.txt", (char *) getenv ("USERPROFILE"));
        else {
            if (!getcwd (tempfilename, sizeof tempfilename)) {
                fatalperror ("opentmpfile: getcwd", "Local error");
            }
            strcat (tempfilename, "\\bbstmp.txt");
        }
        move_if_needed ("c:\\.bbstmp", tempfilename);
#else
        else if (pw)
            sprintf (tempfilename, "%s/.bbstmp", pw->pw_dir);
        else if (getenv ("HOME"))
            sprintf (tempfilename, "%s/.bbstmp", (char *) getenv ("HOME"));
        else
            fatalexit ("opentmpfile: You don't exist, go away.", "Local error");
#endif /* USE_CYGWIN */
    }
    if (!(tempfile = fopen (tempfilename, "a+")))
        fatalperror ("opentmpfile: fopen", "Local error");
    if (chmod (tempfilename, 0600) < 0)
        s_perror ("opentmpfile: chmod", "Warning");
}


void
titlebar (void)
{
#ifdef ENABLE_TITLEBAR
    char    title[80];

    sprintf (title, "%s:%d%s - BBS Client %s (%s)",
             cmdlinehost, cmdlineport, is_ssl ? " (Secure)" : "", VERSION, IsWin32 ? "Windows" : "Unix");
    /* xterm */
    if (!strcmp (getenv ("TERM"), "xterm"))
        printf ("\033]0;%s\007", title);
    /* NeXT */
    if (getenv ("STUART")) {
        printf ("\033]1;%s\\", title);
        printf ("\033]2;%s\\", title);
    }
#endif
    return;
}


void
notitlebar (void)
{
#ifdef ENABLE_TITLEBAR
    /* xterm */
    if (!strcmp (getenv ("TERM"), "xterm"))
        printf ("\033]0;xterm\007");
    /* NeXT */
    if (getenv ("STUART")) {
        struct winsize ws;

        ioctl (0, TIOCGWINSZ, (char *) &ws);
        printf ("\033]1; csh (%s)\033\\", rindex ((char *) ttyname (0), '/') + 1);
        printf ("\033]2; (%s) %dx%d\033\\", rindex ((char *) ttyname (0), '/') + 1, ws.ws_col, ws.ws_row);
    }
    fflush (stdout);
#endif
    return;
}


/*
 * Open a socket connection to the bbs.  Defaults to BBSHOST with port BBSPORT
 * (by default a standard telnet to bbs.isca.uiowa.edu) but can be overridden
 * in the bbsrc file if/when the source to the ISCA BBS is released and others
 * start their own on different machines and/or ports.
 */
void
connectbbs ()
{
    register struct hostent *host;
    register int err;
    struct sockaddr_in sa;

    if (!*bbshost)
        strcpy (bbshost, BBSHOST);
    if (!bbsport)
        bbsport = BBSPORT;
    if (!*cmdlinehost)
        strcpy (cmdlinehost, bbshost);
    if (!cmdlineport)
        cmdlineport = bbsport;
    strncpy ((char *) &sa, "", sizeof sa);
    sa.sin_family = AF_INET;
    sa.sin_port = htons (cmdlineport);  /* Spurious gcc warning */
    if (isdigit (*cmdlinehost))
        sa.sin_addr.s_addr = inet_addr (cmdlinehost);
    else if (!(host = gethostbyname (cmdlinehost)))
        sa.sin_addr.s_addr = inet_addr (BBSIPNUM);
    else
        strncpy ((char *) &sa.sin_addr, host->h_addr, sizeof sa.sin_addr);

    net = socket (AF_INET, SOCK_STREAM, 0);
    if (net < 0)
        fatalperror ("socket", "Local error");
#ifdef ENABLE_SOCKS
    if (use_socks)
        err = Rconnect (net, (struct sockaddr *) &sa, sizeof sa);
    else
#endif
        err = connect (net, (struct sockaddr *) &sa, sizeof sa);
    if (err < 0) {
#define BBSREFUSED	"The BBS has refused connection, try again later.\r\n"
#define BBSNETDOWN	"Network problems prevent connection with the BBS, try again later.\r\n"
#define BBSHOSTDOWN	"The BBS is down or there are network problems, try again later.\r\n"

#ifdef ECONNREFUSED
        if (errno == ECONNREFUSED)
            std_printf (BBSREFUSED);
#endif
#ifdef ENETDOWN
        if (errno == ENETDOWN)
            std_printf (BBSNETDOWN);
#endif
#ifdef ENETUNREACH
        if (errno == ENETUNREACH)
            std_printf (BBSNETDOWN);
#endif
#ifdef ETIMEDOUT
        if (errno == ETIMEDOUT)
            std_printf (BBSHOSTDOWN);
#endif
#ifdef EHOSTDOWN
        if (errno == EHOSTDOWN)
            std_printf (BBSHOSTDOWN);
#endif
#ifdef EHOSTUNREACH
        if (errno == EHOSTUNREACH)
            std_printf (BBSNETDOWN);
#endif
        fatalperror ("connect", "Network error");
    }
#ifdef HAVE_OPENSSL
    if (want_ssl) {
        initSSL ();
        if (SSL_set_fd (ssl, net) != 1) {
            printf ("%s\n", ERR_reason_error_string (ERR_get_error ()));
            shutdown (net, 2);
            exit (1);
        }
        if ((err = SSL_connect (ssl)) != 1) {
            printf ("%s\n", ERR_reason_error_string (ERR_get_error ()));
            shutdown (net, 2);
            exit (1);
        }
        is_ssl = 1;
    }
#endif
    std_printf ("[%secure connection established]\n", (want_ssl) ? "S" : "Ins");
    titlebar ();
    fflush (stdout);

    /*
     * We let the stdio libraries handle buffering issues for us.  Only for
     * output, there are portability problems with what is needed for input.
     */
#ifdef __EMX__
    if (!(netifp = fdopen (net, "r")))
        fatalperror ("fdopen r", "Local error");
#endif
    if (!(netofp = fdopen (net, "w")))
        fatalperror ("fdopen w", "Local error");
}


/*
 * Suspend the client.  Restores terminal to previous state before suspending,
 * puts it back in proper mode when client restarts, and checks if the window
 * size was changed while we were away.
 */
void
suspend ()
{
#ifdef __EMX__
    /* TODO: find out how to make SIGSTOP work under OS/2 */
    printf ("\r\n[Suspension not supported under OS/2]\r\n");
#else
    notitlebar ();
    resetterm ();
    kill (0, SIGSTOP);
    setterm ();
    titlebar ();
    printf ("\r\n[Continue]\r\n");
#endif
    if (oldrows != getwindowsize () && oldrows != -1)
        sendnaws ();
}


/*
 * Quits gracefully when we are given a HUP or STOP signal.
 */
RETSIGTYPE
bye (int signum)
{
    myexit ();
}


/*
 * Handles a WINCH signal given when the window is resized
 */
RETSIGTYPE
naws (int signum)
{
    if (oldrows != -1)
        sendnaws ();
#ifdef SIGWINCH
    signal (SIGWINCH, naws);
#endif
}


/*
 * Handles the death of the child by doing a longjmp back to the function that
 * forked it.  We get spurious signals when the child is stopped, and to avoid
 * confusion we don't allow the child to be stopped -- therefore we attempt to
 * send a continue signal to the child here.  If it fails, we assume the child
 * did in fact die, and longjmp back to the function that forked it.  If it
 * doesn't fail, the child is restarted and the user is forced to exit the
 * child cleanly to get back into the main client.
 */
RETSIGTYPE
reapchild (int signum)
{
#ifndef __EMX__
    wait (0);
    titlebar ();
    if (kill (childpid, SIGCONT) < 0)
#ifdef USE_POSIX_SIGSETJMP
        siglongjmp (jmpenv, 1);
#else
        longjmp (jmpenv, 1);
#endif /* USE_POSIX_SIGSETJMP */
#endif /* !__EMX__ */
}


/*
 * Initialize necessary signals
 */
void
siginit ()
{
    oldrows = -1;

    signal (SIGINT, SIG_IGN);
    signal (SIGQUIT, SIG_IGN);
    signal (SIGPIPE, SIG_IGN);
#ifdef SIGTSTP
    signal (SIGTSTP, SIG_IGN);
#endif
#ifdef SIGTTOU
    signal (SIGTTOU, SIG_IGN);
#endif
    signal (SIGHUP, bye);
    signal (SIGTERM, bye);
#ifdef SIGWINCH
    signal (SIGWINCH, naws);
#endif
}


/*
 * Turn off signals now that we are ready to terminate
 */
void
sigoff ()
{
    signal (SIGALRM, SIG_IGN);
#ifdef SIGWINCH
    signal (SIGWINCH, SIG_IGN);
#endif
    signal (SIGHUP, SIG_IGN);
    signal (SIGTERM, SIG_IGN);
}



static int savedterm = 0;

#ifdef HAVE_TERMIO_H
static struct termio saveterm;

#else
static struct sgttyb saveterm;
static struct tchars savetchars;
static struct ltchars saveltchars;
static int savelocalmode;

#endif


/*
 * Set terminal state to proper modes for running the client/bbs
 */
void
setterm ()
{
#ifdef HAVE_TERMIO_H
    struct termio tmpterm;

#else
    struct sgttyb tmpterm;
    struct tchars tmptchars;
    struct ltchars tmpltchars;
    int     tmplocalmode;

#endif

    getwindowsize ();

    if (flags.useansi)
        printf ("\033[%cm\033[3%c;4%cm", flags.usebold ? '1' : '0', lastcolor, color.background);
    fflush (stdout);

    titlebar ();
#ifdef HAVE_TERMIO_H
    if (!savedterm)
        ioctl (0, TCGETA, &saveterm);
    tmpterm = saveterm;
    tmpterm.c_iflag &= ~(INLCR | IGNCR | ICRNL);
    tmpterm.c_iflag |= IXOFF | IXON | IXANY;
    tmpterm.c_oflag &= ~(ONLCR | OCRNL);
    tmpterm.c_lflag &= ~(ISIG | ICANON | ECHO);
    tmpterm.c_cc[VMIN] = 1;
    tmpterm.c_cc[VTIME] = 0;
    ioctl (0, TCSETA, &tmpterm);
#else
    if (!savedterm)
        ioctl (0, TIOCGETP, (char *) &saveterm);
    tmpterm = saveterm;
    tmpterm.sg_flags &= ~(ECHO | CRMOD);
    tmpterm.sg_flags |= CBREAK | TANDEM;
    ioctl (0, TIOCSETN, (char *) &tmpterm);
    if (!savedterm)
        ioctl (0, TIOCGETC, (char *) &savetchars);
    tmptchars = savetchars;
    tmptchars.t_intrc = tmptchars.t_quitc = tmptchars.t_eofc = tmptchars.t_brkc = -1;
    ioctl (0, TIOCSETC, (char *) &tmptchars);
    if (!savedterm)
        ioctl (0, TIOCGLTC, (char *) &saveltchars);
    tmpltchars = saveltchars;
    tmpltchars.t_suspc = tmpltchars.t_dsuspc = tmpltchars.t_rprntc = -1;
    ioctl (0, TIOCSLTC, (char *) &tmpltchars);
    if (!savedterm)
        ioctl (0, TIOCLGET, (char *) &savelocalmode);
    tmplocalmode = savelocalmode;
    tmplocalmode &= ~(LPRTERA | LCRTERA | LCRTKIL | LCTLECH | LPENDIN | LDECCTQ);
    tmplocalmode |= LCRTBS;
    ioctl (0, TIOCLSET, (char *) &tmplocalmode);
#endif
    savedterm = 1;
}


/*
 * Reset the terminal to the previous state it was in when we started.
 */
void
resetterm ()
{
    if (flags.useansi)
/*	printf("\033[0m\033[1;37;49m"); */
        printf ("\033[0;39;49m");
    fflush (stdout);
    if (!savedterm)
        return;
#ifdef HAVE_TERMIO_H
    ioctl (0, TCSETA, &saveterm);
#else
    ioctl (0, TIOCSETN, (char *) &saveterm);
    ioctl (0, TIOCSETC, (char *) &savetchars);
    ioctl (0, TIOCSLTC, (char *) &saveltchars);
    ioctl (0, TIOCLSET, (char *) &savelocalmode);
#endif
}


/*
 * Get the current window size.
 */
int
getwindowsize ()
{
#ifdef TIOCGWINSZ
    struct winsize ws;

    if (ioctl (0, TIOCGWINSZ, (char *) &ws) < 0)
        return (rows = 24);
    else if ((rows = ws.ws_row) < 5 || rows > 120)
        return (rows = 24);
    else
        return (rows);
#else
    return (rows = 24);
#endif
}



void
mysleep (sec)
     unsigned int sec;
{
    sleep (sec);
}



/*
 * This function flushes the input buffer in the same manner as the BBS does.
 * By doing it on the client end we save the BBS the trouble of doing it, but
 * in general the same thing will happen on one end or the other, so you won't
 * speed things up at all by changing this, the sleep is there for your
 * protection to insure a cut and paste gone awry or line noise doesn't cause
 * you too much hassle of posting random garbage, changing your profile or
 * configuration or whatever.
 */
void
flush_input (invalid)
     unsigned int invalid;
{
    int     i;

    if (invalid / 2)
        mysleep (invalid / 2 < 3 ? invalid / 2 : 3);
#ifdef FIONREAD
    while (INPUT_LEFT (stdin) || (!ioctl (0, FIONREAD, &i) && i > 0))
#else
#ifdef TCFLSH
    i = 0;
    ioctl (0, TCFLSH, &i);
#endif
    while (INPUT_LEFT (stdin))
#endif
        (void) ptyget ();
}



/*
 * Run the command 'cmd' with argument 'arg'.  Used only for running the editor
 * right now.  In order to work properly with all the versions of Unix I've
 * tried to port this to so far without be overly complicated, I have to use a
 * setjmp to save the local stack context in this function, then longjmp back
 * here once I receive a signal from the child that it has terminated. So I
 * guess there actually IS a use for setjmp/longjmp after all! :-)
 */
void
run (cmd, arg)
     char   *cmd;
     char   *arg;
{
#ifdef __EMX__
    /* TODO: find out how to make SIGCONT work under OS/2 */
    printf ("[Editor not supported under OS/2]\r\n");
    return;
#else
    fflush (stdout);
#ifdef USE_POSIX_SIGSETJMP
    if (sigsetjmp (jmpenv, 1)) {
#else
    if (setjmp (jmpenv)) {
#endif /* USE_POSIX_SIGSETJMP */
        signal (SIGCHLD, SIG_DFL);
        if (childpid < 0) {
            childpid = 0;
            myexit ();
        }
        else {
            setterm ();
            childpid = 0;
        }
    }
    else {
        signal (SIGCHLD, reapchild);
        notitlebar ();
        resetterm ();

        if (!(childpid = fork ())) {
            execlp (cmd, cmd, arg, 0);
            fprintf (stderr, "\r\n");
            s_perror ("exec", "Local error");
            _exit (0);
        }
        else if (childpid > 0) {

            /*
             * Flush out anything in our stdio buffer -- it was copied to the
             * child process, we don't want it waiting for us when the child
             * is done.
             */
            flush_input (0);
            (void) inkey ();
        }
        else
            fatalperror ("fork", "Local error");
    }
#endif /* __EMX__ */
}


void
techinfo (void)
{
    std_printf ("Technical information\r\n\n");

    feed_pager (3, "ISCA BBS Client " VERSION " (Unix)\r\n", "Compiled on: " HOSTTYPE "\r\n", "With: "
#ifdef __STDC__
                "ANSI "
#endif
#ifdef __cplusplus
                "C++ "
#endif
#ifdef __GNUC__
                "gcc "
#endif
#ifdef _POSIX_SOURCE
                "POSIX "
#endif
#ifdef ENABLE_SAVE_PASSWORD
                "save-password "
#endif
#ifdef USE_POSIX_SIGSETJMP
                "sigsetjmp "
#endif
#ifdef ENABLE_SOCKS
                "SOCKS "
#endif
                "\r\n", NULL);
}


void
initialize (protocol)
     const char *protocol;
{
    if (!isatty (0) || !isatty (1) || !isatty (2))
        exit (0);

    ptyifp = ptyibuf;
#ifndef __EMX__
    netifp = netibuf;
#endif

    /* Check for Win32, we need this as a variable in a few places */
#ifdef USE_CYGWIN
    IsWin32 = 1;
#else
    IsWin32 = 0;
#endif /* USE_CYGWIN */

    away = 0;

#ifdef _IOFBF
#ifdef SETVBUF_REVERSED
    setvbuf (stdout, _IOFBF, NULL, 4096);
#else
    setvbuf (stdout, NULL, _IOFBF, 4096);
#endif
#endif

    std_printf ("\nISCA BBS Client %s (%s)\n", VERSION,
#ifdef USE_CYGWIN
                "Windows"
#else
                "Unix"
#endif
        );
    std_printf ("\nCopyright (C) 1995-2003 Michael Hampton.\n");
    std_printf ("OSI Certified Open Source Software.  GNU General Public License version 2.\n");
    std_printf ("For information about this client visit http://www.ioerror.us/client/\n\n");
#if DEBUG
    std_printf ("DEBUGGING VERSION - DEBUGGING CODE IS ENABLED!  DO NOT USE THIS CLIENT!\r\n\n");
#endif
    fflush (stdout);
    xlandQueue = new_queue (21, MAXLAST);
    if (!xlandQueue)
        xland = 0;
    if (login_shell)
        strcpy (shell, "/bin/true");
    else {
        if (getenv ("SHELL"))
            strcpy (shell, (char *) getenv ("SHELL"));
        else
            strcpy (shell, "/bin/sh");
    }
    if (!login_shell)
        strcpy (browser, "netscape -remote");
    if (login_shell)
        strcpy (myeditor, "\0");
    else {
        if (getenv ("EDITOR"))
            strcpy (myeditor, (char *) getenv ("EDITOR"));
        else
            strcpy (myeditor, "vi");
    }
}


void
deinitialize ()
{
    char    tfile[100];

    notitlebar ();
    /* Get rid of ~ file emacs always leaves behind */
    strcpy (tfile, tempfilename);
    strcat (tfile, "~");
    unlink (tfile);
    if (login_shell) {
        unlink (tempfilename);
        unlink (bbsrcname);
        unlink (bbsfriendsname);
    }
}


int
deletefile (const char *pathname)
{
    return unlink (pathname);
}


int
s_prompt (const char *info, const char *question, int def)
{
#ifdef USE_CYGWIN
    int     flags = MB_APPLMODAL | MB_YESNO | MB_ICONQUESTION;
    int     ret;

    if (!textonly) {
        if (def)
            flags |= MB_DEFBUTTON1; /* Yes */
        else
            flags |= MB_DEFBUTTON2; /* No */

        ret = MessageBox (NULL, info, question, flags);
        if (ret == IDYES)
            return 1;
        return 0;
    }
#endif
    std_printf ("\r\n%s\r\n\n", info);
    std_printf ("%s (%s) -> ", question, def ? "Yes" : "No");
    if (yesnodefault (def))
        return 1;
    return 0;
}

void
s_info (const char *info, const char *heading)
{
#ifdef USE_CYGWIN
    if (!textonly) {
        MessageBox (NULL, info, heading, MB_ICONINFORMATION | MB_APPLMODAL | MB_OK);
        return;
    }
#endif
    /* Heading ignored for Unix */
    std_printf ("\r\n%s\r\n\n", info);
    return;
}


void
s_perror (const char *msg, const char *heading)
{
    char    buf[4096];

#ifdef USE_CYGWIN
    if (!textonly) {
        sprintf (buf, "%s: %s", msg, strerror (errno));
        MessageBox (NULL, buf, heading, MB_APPLMODAL | MB_OK | MB_ICONERROR);
        return;
    }
#endif
    sprintf (buf, "%s: %s", heading, msg);
    perror (buf);
    fprintf (stderr, "\r");
    return;
}


void
s_error (const char *msg, const char *heading)
{
    char    buf[4096];

#ifdef USE_CYGWIN
    if (!textonly) {
        MessageBox (NULL, msg, heading, MB_APPLMODAL | MB_OK | MB_ICONERROR);
    }
#endif
    sprintf (buf, "%s: %s", heading, msg);
    fflush (stdout);
    fprintf (stderr, "%s\r\n", buf);
}


/* TODO: system() is kind of cheating */
/* TODO: Peeking into the queue object itself is REALLY cheating */
void
open_browser (void)
{
    int     c, capturestate;
    char    line[4];
    char    cmd[4096];
    char   *p;

    if (urlQueue->nobjs < 1)
        return;
    if (urlQueue->nobjs == 1) {
#ifdef USE_CYGWIN
        ShellExecute (NULL, "open", urlQueue->start + (urlQueue->objsize * urlQueue->head), NULL, NULL,
                      SW_SHOW);
#else
        sprintf (cmd, "%s \"%s\"%s", browser,
                 urlQueue->start + (urlQueue->objsize * urlQueue->head), flags.browserbg ? " &" : "");
        system (cmd);
#endif
        if (!flags.browserbg)
            reprint_line ();
        return;
    }

    capturestate = capture;
    capture = 0;
    ignore_network = 1;
    printf ("\r\n\n");
    p = urlQueue->start + (urlQueue->objsize * urlQueue->head);
    for (c = 0; c < urlQueue->nobjs; c++) {
        if (strlen (p) > 72) {
            char    junk[71];

            strncpy (junk, p, 70);
            junk[70] = 0;
            printf ("%d. %-70s...\r\n", c + 1, junk);
        }
        else
            printf ("%d. %s\r\n", c + 1, p);
        p += urlQueue->objsize;
        if (p >= (char *) (urlQueue->start + (urlQueue->objsize * urlQueue->size)))
            p = urlQueue->start;
    }

    printf ("\r\nChoose the URL you want to view: ");
    get_string (3, line, 1);    /* No more than 999 URLs in a post? */
    printf ("\r\n");
    c = atoi (line);
    p = urlQueue->start + (urlQueue->objsize * urlQueue->head);
    if (c > 0 && c <= urlQueue->nobjs) {
        int     j;

        c -= 1;
        for (j = 0; j < c; j++) {
            p += urlQueue->objsize;
            if (p >= (char *) (urlQueue->start + (urlQueue->objsize * urlQueue->size)))
                p = urlQueue->start;
        }
#ifdef USE_CYGWIN
        ShellExecute (NULL, "open", p, NULL, NULL, SW_SHOW);
#else
        sprintf (cmd, "%s \"%s\"%s", browser, p, flags.browserbg ? " &" : "");
        system (cmd);
#endif
    }
    ignore_network = 0;
    reprint_line ();
    capture = capturestate;
    return;
}


/* 
 * Move oldpath to newpath if oldpath exists and newpath does not exist.
 * Then delete oldpath, even if newpath already exists.
 */
void
move_if_needed (const char *oldpath, const char *newpath)
{
    FILE   *old;
    FILE   *new;
    char    buf[BUFSIZ];
    int     i;
    size_t  s;

    old = fopen (oldpath, "r");
    if (!old)
        return;

    new = fopen (newpath, "a");
    if (!new)
        return;

    s = ftell (new);
    if (s == 0) {
        /* Args 2 and 3 intentionally reversed */
        while ((i = fread (buf, 1, BUFSIZ, old)) > 0) {
            i = fwrite (buf, 1, BUFSIZ, new);
        }
    }

    fclose (old);
    fclose (new);
    unlink (oldpath);
    return;
}
