/*
 * This file handles parsing of the bbsrc file, and setting of all the options
 * it allows.  It is just generic C code, easily extensible to allow the
 * addition of features in the future. 
 */
#include "defs.h"
#include "ext.h"


/*
 * Given a pointer to a string, this function evaluates it to a control
 * character, translating '^z', '^Z', or an actual ctrl-Z to all be ctrl-Z, If
 * the character is not a control character, it is simply returned as is. 
 */
static int
ctrl (const char *s)
{
    int     c = *s;

    if (c == '^') {
        if (((c = *++s) >= '@' && c <= '_') || c == '?') {
            c ^= 0x40;
        }
        else if (c >= 'a' && c <= 'z') {
            c ^= 0x60;
        }
    }
    if (c == '\r')
        c = '\n';
    return c;
}


/*
 * Parses the bbsrc file, setting necessary globals depending on the content of
 * the bbsrc, or returning an error if the bbsrc couldn't be properly parsed. 
 */
#define MAXLINELEN 83
void
readbbsrc (void)
{
    char    tmp[MAXLINELEN + 1];
    char    tmps[MAXLINELEN + 1];
    int     c;
    char   *s, *m;
    int     l = 0;
    int     reads = 0;
    int     tmpVersion = 0;

    version = INTVERSION;
    commandkey = shellkey = capturekey = suspkey = quitkey = awaykey = -1;
    browserkey = -1;

    for (c = 0; c <= 127; c++) {
        keymap[c] = (char) c;
        *macro[c] = 0;
    }
    xland = true;

    autologgedin = false;
    *autoname = 0;
    autopasswdsent = false;
    *autopasswd = 0;

    str_clear (editor);
    *bbshost = 0;
    bbsrc = findbbsrc ();
    bbsfriends = findbbsfriends ();
    flags.useansi = flags.usebold = flags.offbold = flags.moreflag = 0;
    flags.ansiprompt = 0;
    flags.browserbg = 0;
    flags.autofix_posts = 1;

    default_colors (true);

    recving_wholist = 0;
    highxmsg = 0;
    needx = false;
    xmsgnow = false;
    postnow = false;
    eatline = 0;
    textonly = want_ssl = false;
    xmsgbufp = xmsgbuf;

    while (bbsrc && fgets (tmp, MAXLINELEN + 1, bbsrc)) {
        reads++;
        l++;
        if ((int) strlen (tmp) >= MAXLINELEN) {
            std_printf ("Line %d in .bbsrc too long, ignored.\n", l);
            while ((int) strlen (tmp) >= MAXLINELEN && tmp[MAXLINELEN - 1] != '\n')
                fgets (tmp, MAXLINELEN + 1, bbsrc);
            continue;
        }
        for (c = strlen (tmp) - 1; c >= 0; c--)
            if (tmp[c] == ' ' || tmp[c] == '\t' || tmp[c] == '\n' || tmp[c] == '\r')
                tmp[c] = 0;
            else
                break;

        /* Just ignore these for now, they'll be quietly erased... */
        if (!strncmp (tmp, "reread ", 7)) ;
        else if (!strncmp (tmp, "xwrap ", 6)) ;

        /* Client configuration options (current) */
        else if (!strncmp (tmp, "bold", 4))
            flags.usebold = 1;

        else if (!strncmp (tmp, "autofix_posts", 13))
            flags.autofix_posts = 1;

        else if (!strncmp (tmp, "textonly", 8))
            textonly = true;

        else if (!strncmp (tmp, "xland", 5))
            xland = false;

        else if (!strncmp (tmp, "version ", 8))
            tmpVersion = atoi (tmp + 8);

        else if (!strncmp (tmp, "squelch ", 8))
            switch (atoi (tmp + 8)) {
            case 3:
                flags.squelchpost = flags.squelchexpress = 1;
                break;
            case 2:
                flags.squelchpost = 1;
                break;
            case 1:
                flags.squelchexpress = 1;
                break;
            default:
                break;
            }
        else if (!strncmp (tmp, "color ", 6))
            if (strlen (tmp) != 6 + sizeof color)
                std_printf ("Invalid 'color' scheme on line %d, ignored.\n", l);
            else {
                bcopy (tmp + 6, (void *) &color, sizeof color);
            }
        else if (!strncmp (tmp, "autoname ", 9)) {
            if (strncmp (tmp + 9, "Guest", 5)) {
                strncpy (autoname, tmp + 9, 21);
                autoname[20] = 0;
            }
        }
        else if (!strncmp (tmp, "autoansi", 9)) {
            if (strlen (tmp) <= 9 || tmp[9] != 'N') {
                flags.ansiprompt = 1;
            }
        }
        else if (!strncmp (tmp, "autopass ", 9)) {
            strncpy (autopasswd, tmp + 9, 21);
            autopasswd[20] = 0;
        }
        else if (!strncmp (tmp, "browser ", 8)) {
            if (strlen (tmp) < 11)
                std_printf ("Invalid definition of 'browser' ignored.\n");
            else {
                flags.browserbg = (tmp[8] == '0') ? 0 : 1;
                strncpy (browser, tmp + 10, 80);
            }
        }
        else if (!strncmp (tmp, "editor ", 7))
            str_assigns (editor, tmp + 7);

        else if (!strncmp (tmp, "site ", 5))
            if (*bbshost)
                std_printf ("Multiple definition of 'site' ignored.\n");
            else {
                for (c = 5; (bbshost[c - 5] = tmp[c]) && tmp[c] != ' ' && c < 68; c++) ;
                if (c == 68 || c == 5) {
                    std_printf ("Illegal hostname in 'site', using default.\n");
                    *bbshost = 0;
                }
                else {
                    bbshost[c - 5] = 0;
                    if (tmp[c])
                        bbsport = (unsigned short) atoi (tmp + c + 1);
                    else
                        bbsport = BBSPORT;
                    for (; tmp[c] && tmp[c] != ' '; c++) ;
                    if (!strncmp (tmp + c, "secure", 6))
                        want_ssl = 1;
                }
                if (!strcmp (bbshost, "128.255.200.69") || !strcmp (bbshost, "128.255.85.69") || !strcmp (bbshost, "128.255.95.69") || !strcmp (bbshost, "128.255.3.160") || !strcmp (bbshost, "bbs.iscabbs.info")) /* Old addresses */
                    strcpy (bbshost, BBSHOST);  /* changed to new */
            }
        else if ((!strncmp (tmp, "friend ", 7)) && (!bbsfriends || !fgets (tmps, MAXLINELEN + 1, bbsfriends))) {
            // Syntax is one of the following:
            // friend [7..Name..27] [30..Info..83]
            // friend [7..Name..27]
            if (!strncmp (tmp, "friend ", 7)) {
                if (strlen (tmp) == 7)
                    std_printf ("Empty username in 'friend'.\n");
                else {
                    char    name[21];

                    memset (name, 0, sizeof (name));
                    strncpy (name, tmp + 7, 20);
                    rtrim (name);
                    UserEntry *pf = ulist_insert (&friendList, name);

                    if (strlen (tmp) > 30) {
                        strncpy (pf->info, tmp + 30, 53);
                        rtrim (pf->info);
                    }
                    else
                        strcat (pf->info, "(None)");
                }
            }
        }
        else if (!strncmp (tmp, "enemy ", 6)) {
            char    name[21];

            memset (name, 0, sizeof (name));
            strncpy (name, tmp + 6, 20);
            rtrim (name);
            ulist_insert (&enemyList, name);
        }
        else if (!strncmp (tmp, "commandkey ", 11) || !strncmp (tmp, "macrokey ", 9)) {
            if (commandkey >= 0)
                std_printf ("Additional definition for 'commandkey' ignored.\n");
            else {
                if (!strncmp (tmp, "macrokey ", 9)) {
                    commandkey = ctrl (tmp + 9);
                }
                else {
                    commandkey = ctrl (tmp + 11);
                }
                if (strchr ("\0x01\0x03\0x04\0x05\b\n\r\0x11\0x13\0x15\0x17\0x18\0x19\0x1a\0x7f", commandkey)
                    || commandkey >= ' ') {
                    std_printf ("Illegal value for 'commandkey', using default of 'Esc'.\n");
                    commandkey = 0x1b;
                }
            }
        }
        else if (!strncmp (tmp, "awaykey ", 8)) {
            if (awaykey >= 0)
                std_printf ("Additional definition for 'awaykey' ignored.\n");
            else
                awaykey = ctrl (tmp + 8);
        }
        else if (!strncmp (tmp, "quit ", 5)) {
            if (quitkey >= 0)
                std_printf ("Additional definition for 'quit' ignored.\n");
            else
                quitkey = ctrl (tmp + 5);
        }
        else if (!strncmp (tmp, "susp ", 5)) {
            if (suspkey >= 0)
                std_printf ("Additional definition for 'susp' ignored.\n");
            else
                suspkey = ctrl (tmp + 5);
        }
        else if (!strncmp (tmp, "capture ", 8)) {
            if (capturekey >= 0)
                std_printf ("Additional definition for 'capture' ignored.\n");
            else
                capturekey = ctrl (tmp + 8);
        }
        else if (!strncmp (tmp, "keymap ", 7)) {
            c = *(tmp + 7);
            s = tmp + 8;
            if (*s++ == ' ' && c > 32 && c < 128)
                keymap[c] = *s;
            else
                std_printf ("Invalid value for 'keymap' ignored.\n");
        }
        else if (!strncmp (tmp, "url ", 4)) {
            if (browserkey >= 0)
                std_printf ("Additional definition for 'url' ignored.\n");
            else
                browserkey = ctrl (tmp + 4);
        }
        else if (!strncmp (tmp, "macro ", 6)) {
            c = ctrl (tmp + 6);
            s = tmp + 7 + (tmp[6] == '^');
            if (*s++ == ' ') {
                if (*macro[c])
                    std_printf ("Additional definition of same 'macro' value ignored.\n");
                else {
                    /* Import 'i' macro to awaymsg */
                    if (c == 'i' && !awaykey && tmpVersion < 220) {
                        int     q = 0;

                        m = awaymsg[0];
                        awaykey = 'i';
                        while ((c = *s++)) {
                            if (c == '^' && *s != '^')
                                c = ctrl (s++ - 1);
                            if (c == '\r')
                                c = '\n';
                            if (c == '\n')
                                m = awaymsg[++q];
                            else if (iscntrl (c))
                                continue;
                            else
                                *m++ = c;
                        }
                    }
                    else {
                        m = macro[c];
                        while ((c = *s++)) {
                            if (c == '^' && *s != '^')
                                c = ctrl (s++ - 1);
                            if (c == '\r')
                                c = '\n';
                            *m++ = c;
                        }
                    }
                }
            }
            else
                std_printf ("Syntax error in 'macro', ignored.\n");
        }
        else if (!strncmp (tmp, "awaymsg ", 8)) {
            /* Import old away messages */
            int     q = 0;

            m = awaymsg[q];
            s = tmp + 8;
            while ((c = *s++)) {
                if (c == '^' && *s != '^')
                    c = ctrl (s++ - 1);
                if (c == '\r')
                    c = '\n';
                if (c == '\n')
                    m = awaymsg[++q];
                else if (iscntrl (c))
                    continue;
                else
                    *m++ = c;
            }
        }
        else if (tmp[0] == 'a' && tmp[1] >= '1' && tmp[1] <= '5' && tmp[2] == ' ') {
            /* New away messages */
            strcpy (awaymsg[tmp[1] - '1'], tmp + 3);
        }
        else if (!strncmp (tmp, "shell ", 6)) {
            if (shellkey >= 0)
                std_printf ("Additional definition for 'shell' ignored.\n");
            else
                shellkey = ctrl (tmp + 6);
        }
        else if (*tmp != '#' && *tmp && strncmp (tmp, "friend ", 7))
            std_printf ("Syntax error in .bbsrc file in line %d.\n", l);
    }

    if (bbsfriends)
        rewind (bbsfriends);
    while (bbsfriends && fgets (tmp, MAXLINELEN + 1, bbsfriends)) {
        reads++;
        l++;
        if (strlen (tmp) >= MAXLINELEN) {
            std_printf ("Line %d in .bbsfriends too long, ignored.\n", l);
            while (strlen (tmp) >= MAXLINELEN && tmp[MAXLINELEN - 1] != '\n')
                fgets (tmp, MAXLINELEN + 1, bbsfriends);
            continue;
        }
        for (c = strlen (tmp) - 1; c >= 0; c--)
            if (tmp[c] == ' ' || tmp[c] == '\t' || tmp[c] == '\n' || tmp[c] == '\r')
                tmp[c] = 0;
            else
                break;

        // Syntax is one of the following:
        // friend [7..Name..27] [30..Info..83]
        // friend [7..Name..27]
        if (!strncmp (tmp, "friend ", 7)) {
            if (strlen (tmp) == 7)
                std_printf ("Empty username in 'friend'.\n");
            else {
                char    name[21];

                memset (name, 0, sizeof (name));
                strncpy (name, tmp + 7, 20);
                UserEntry *pf = ulist_insert (&friendList, name);

                if (strlen (tmp) > 30) {
                    strncpy (pf->info, tmp + 30, 53);
                    rtrim (pf->info);
                }
                else
                    strcat (pf->info, "(None)");
            }
        }
    }

    /*
       if (!bbsrc || !reads) {
       commandkey = ESC;
       awaykey = 'a';
       quitkey = CTRL_D;
       suspkey = CTRL_Z;
       capturekey = 'c';
       shellkey = '!';
       }
     */
    if (commandkey == -1)
        commandkey = ESC;
    if (awaykey == -1)
        awaykey = 'a';
    if (quitkey == -1)
        quitkey = CTRL_D;
    if (suspkey == -1)
        suspkey = CTRL_Z;
    if (capturekey == -1)
        capturekey = 'c';
    if (shellkey == -1)
        shellkey = '!';
    if (browserkey == -1)
        browserkey = 'w';

    if (!**awaymsg) {
        strcpy (awaymsg[0], "I'm away from my keyboard right now.");
        *awaymsg[1] = 0;
    }

    default_colors (false);

    if (quitkey >= 0 && *macro[quitkey])
        std_printf ("Warning: duplicate definition of 'macro' and 'quit'\n");
    if (suspkey >= 0 && *macro[suspkey])
        std_printf ("Warning: duplicate definition of 'macro' and 'susp'\n");
    if (capturekey >= 0 && *macro[capturekey])
        std_printf ("Warning: duplicate definition of 'macro' and 'capture'\n");
    if (shellkey >= 0 && *macro[capturekey])
        std_printf ("Warning: duplicate definition of 'macro' and 'shell'\n");
    if (quitkey >= 0 && quitkey == suspkey)
        std_printf ("Warning: duplicate definition of 'quit' and 'susp'\n");
    if (quitkey >= 0 && quitkey == capturekey)
        std_printf ("Warning: duplicate definition of 'quit' and 'capture'\n");
    if (quitkey >= 0 && quitkey == shellkey)
        std_printf ("Warning: duplicate definition of 'quit' and 'shell'\n");
    if (suspkey >= 0 && suspkey == capturekey)
        std_printf ("Warning: duplicate definition of 'susp' and 'capture'\n");
    if (suspkey >= 0 && suspkey == shellkey)
        std_printf ("Warning: duplicate definition of 'susp' and 'shell'\n");
    if (capturekey >= 0 && capturekey == shellkey)
        std_printf ("Warning: duplicate definition of 'capture' and 'shell'\n");

    // The 2.3.9 code copied the friends list to the who list here.
    // I don't understand why, so I removed it.
    ulist_sort_by_name (&friendList);
    ulist_sort_by_name (&enemyList);
    ulist_sort_by_time (&whoList);  // TODO: the whoList is empty here, right?

    if (!*bbshost) {
        strcpy (bbshost, BBSHOST);
        bbsport = BBSPORT;
    }
    if (str_empty (editor))
        str_assign (editor, myeditor);
    if (version != tmpVersion) {
        if (reads) {
            setup (tmpVersion);
        }
        else {
            setup (-1);
        }
    }
    if (login_shell) {
        setterm ();
        configbbsrc ();
        resetterm ();
    }
}



/*
 * Opens the bbsrc file, warning the user if it can't be opened or can't be
 * opened for write, returning the file pointer if it was opened successfully. 
 */
FILE   *
openbbsrc (void)
{
    int     e;

    FILE   *f = fopen (bbsrcname, "r+");

    if (!f) {
        e = errno;
        f = fopen (bbsrcname, "w+");
    }
    if (!f) {
        f = fopen (bbsrcname, "r");
        if (f) {
            bbsrcro = true;
            errno = e;
            s_perror ("Configuration is read-only", "Warning");
        }
        else
            s_perror ("Can't open configuration file", "Warning");
    }
    return f;
}


/*
 * Opens the bbsfriends file, warning the user if it can't be opened or can't
 * be opened for write, returning the file pointer if it was opened
 * successfully.
 */
FILE   *
openbbsfriends (void)
{
    return fopen (bbsfriendsname, "r");
}
