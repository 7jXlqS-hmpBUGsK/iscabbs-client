/*
 * Return next letter the user typed.  This function can be indirectly
 * recursive -- inkey may call telrcv(), which calls other functions which can
 * call inkey()... 
 *
 * Basically, if there is a keypress waiting in the stdio buffer, it is returned
 * immediately, if not the socket's stdio buffer is checked (this is where the
 * recursion can take place) and if that is also empty, the output buffers are
 * flushed, then the program blocks in select() until either the user types
 * something or something is sent from the remote side. 
 */
#include "defs.h"
#include "ext.h"

static int lastcr = 0;


/*
 * The functionality of the former inkey() has been renamed to getkey(), so
 * that inkey() could strip a \n after a \r (common terminal program problem
 * with user misconfiguration) and translate certain keypresses into common
 * Unix equivalents (i.e. \r -> \n, DEL -> BS, ctrl-U -> ctrl-X)
 */
int
inkey (void)
{
    int     c;

    for (;;) {
        c = getkey ();
        if (!lastcr || c != '\n')
            break;
        lastcr = 0;
    }
    lastcr = 0;
    if (c == '\r') {
        c = '\n';
        lastcr = 1;
    }
    else if (c == 127)
        c = '\b';
    else if (c == CTRL_U)
        c = CTRL_X;
    return c;
}


int
getkey (void)
{
    static int macron = 0;
    static int macrop = 0;      /* pointer into the macro array */
    static int macronext = 0;   /* macro key was hit, macro is next */
    static int wasundef = 0;    /* to remove the blurb about undefined macro */
    int     c = -1;
    int     result;             /* result returned from waitnextevent() */


    /*
     * Throughout this function, if we are currently running with a child
     * process we don't want to do anything with standard input, we are
     * only * concerned with passing along anything that might be coming
     * over the net during this time (connection going down, broadcast
     * message from wizards, etc.)  That's the reason for all the
     * references to '!childpid' The same is true when 'check' is set, it
     * is used when entering the edit menu (abort, save, etc.) to check
     * back with the BBS for any X messages that may have arrived -- it is
     * added purely to make things compatible between the BBS and the
     * client. 
     */

    for (;;) {

        /*
         * If we're busy going to our save buffers, targetbyte is
         * non-zero.  We continue to take characters from our save
         * buffers until bytep catches up to targetbyte -- then we
         * should be synced with the bbs once again. 
         */
        if (targetbyte && !childpid && !flags.check) {
            if (bytep == targetbyte) {
                targetbyte = 0;
            }
            else if (bytep > targetbyte) {
                std_printf ("[Internal error: byte synch lost!  %s > %s]\r\n", bytep, targetbyte);
                exit (1);
            }
            else {
                lastcr = 0;
                return save[bytep++ % sizeof save];
            }
        }
        /*
         * Main processing loop for processing a macro or characters
         * in the stdin buffers. 
         */
        while ((macrop || INPUT_LEFT ()) && !childpid && !flags.check) {
            /* macrop > 0 when we are getting out input from a macro key */
            if (macrop > 0) {
                if ((c = macro[macron][macrop++])) {
                    lastcr = 0;
                    return c;
                }
                else {
                    macrop = 0;
                    continue;
                }
            }
            if (!macrop) {
                c = ptyget () & 0x7f;
            }
            else {
                macrop = 0;
            }
            if (c > 0 && away == 1) {
                away = 0;
                std_printf ("\r\n[No longer away]\r\n");
            }
            /* Did we hit commandkey last?  Then the next key hit is the macro */
            if (macronext) {
                printf ("\b\b\b\b\b\b\b\b\b         \b\b\b\b\b\b\b\b\b");
                macronext = macrop = 0;

                if (c == awaykey) {
                    away ^= 1;
                    std_printf ("\r\n[%s away]\r\n", (away) ? "Now" : "No longer");
                    continue;
                }
                if (c == quitkey) {
                    std_printf ("\r\n[Quitting]\r\n");
                    myexit ();
                }
                else if (c == suspkey) {
                    if (!login_shell) {
                        printf ("\r\n[Suspended]\r\n");
                        fflush (stdout);
                        suspend ();
                    }
                    continue;
                }
                else if (c == shellkey) {
                    if (!login_shell) {
                        printf ("\r\n[New shell]\r\n");
                        run (shell, 0);
                        printf ("\r\n[Continue]\r\n");
                    }
                    continue;
                }
                else if (c == browserkey && !login_shell) {
                    open_browser ();
                    continue;
                }
                else if (c == capturekey) {
                    if (capture < 0 || flags.posting) {
                        printf ("[ Cannot capture! ]");
                        wasundef = 1;
                        continue;
                    }
                    capture ^= 1;
                    printf ("\r\n[Capture to temp file turned O%s]\r\n", capture ? "N" : "FF");
                    if (capture) {
                        rewind (tempfile);
                        if (flags.lastsave) {
                            (void) freopen (tempfilename, "w+", tempfile);
                            flags.lastsave = 0;
                        }
                        else if (getc (tempfile) >= 0) {
                            printf ("There is text in your edit file.  Do you wish to erase it? (Y/N) -> ");
                            capture = -1;
                            if (yesno ())
                                (void) freopen (tempfilename, "w+", tempfile);
                            else
                                fseek (tempfile, 0L, SEEK_END);
                            capture = 1;
                        }
                        flags.lastsave = 0;
                    }
                    else
                        fflush (tempfile);
                    continue;
                }
                else if (c > 127 || !*macro[macron = c]) {
                    printf ("[Undefined command]");
                    wasundef = 1;
                    continue;
                }
                else
                    return macro[macron][macrop++];
            }
            /* If we just printed the undefined command blurb, let's erase it */
            else if (wasundef) {
                wasundef = 0;
                printf
                    ("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b                   \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
            }
            /* Did the user just hit the command key? */
            if (c == commandkey && !flags.configflag) {
                macronext = 1;
                printf ("[Command]");
            }
            else
                return c;       /* Return the next character to the caller */
        }

        /* Handle any incoming traffic in the network input buffer */
        if (NET_INPUT_LEFT ()) {
            while (NET_INPUT_LEFT ())
                if (telrcv (netget ()) < 0)
                    return -1;
            continue;
        }
        /* Flush out any output buffers */
        if (netflush () < 0) {
            std_printf ("\r\n");
            fatalperror ("send", "Network error");
        }
        if (fflush (stdout) < 0) {
            std_printf ("\r\n");
            fatalperror ("write", "Local error");
        }

        /*
         * Wait for the next event from either the network or the user,
         * note that if we are running with a child process, we'll be
         * ignoring the user input entirely -- we're only concerned
         * with network traffic until we no longer have a child process
         * to swallow up our data for us. 
         */
        result = waitnextevent ();

        /* The user has input waiting for us to process */
        if (result & 1) {
            c = ptyget ();
            if (c < 0) {
                std_printf ("\r\n");
                fatalperror ("read", "Local error");
            }
            c &= 0x7f;
            macrop = -1;
            continue;
        }
        /* The network has input waiting for us to process */
        if (result & 2) {
            errno = 0;
            if ((c = netget ()) < 0) {
                if (errno) {
                    std_printf ("\r\n");
                    fatalperror ("recv", "Network error");
                }
                else {
                    if (childpid)
                        std_printf ("\r\n\n\007[DISCONNECTED]\r\n\n\007");
                    else
                        std_printf ("\r\n[Disconnected]\r\n");
                    myexit ();
                }
            }
            /* Handle net traffic */
            if (telrcv (c) < 0)
                return -1;
        }
    }
}
