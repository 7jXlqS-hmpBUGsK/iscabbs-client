/** @copyright GNU Public License. See COPYING. */
/*
 * This file handles configuration of the bbsrc file.  Its somewhat sloppy but
 * it should do the job.  Someone else can put a nicer interface on it. 
 */
#include "defs.h"
#include "ext.h"

#define GREETING	\
"\r\nWelcome to IO ERROR's ISCA BBS Client!  Please take a moment to familiarize\r\nyourself with some of our new features.\r\n\n"
#define UPGRADE		\
"Thank you for upgrading to the latest version of IO ERROR's ISCA BBS Client!\r\nPlease take a moment to familiarize yourself with our new features."
#define DOWNGRADE	\
"You appear to have downgraded your version of IO ERROR's ISCA BBS Client.\r\nIf you continue running this client, you may lose some of your preferences and\r\nfeatures you are accustomed to.  Please visit the above web site to upgrade\r\nto the latest version of IO ERROR's ISCA BBS Client."
#define BBSRC_INFO	\
"IO ERROR's ISCA BBS Client integrates the contents of the .bbsrc and\r\n.bbsfriends file into a single file.  This change is fully compatible with\r\nolder clients, however those clients might re-create the .bbsfriends file.\r\nThis should not be a problem for most people; however, we recommend making a\r\nbackup copy of your .bbsrc and .bbsfriends files.  If for some reason you NEED\r\nthe .bbsrc and .bbsfriends files separated, DO NOT RUN THIS CLIENT."
#define COLOR_INFO	\
"IO ERROR's ISCA BBS Client allows you to choose what colors posts and express\r\nmessages are displayed with.  Use the <C>olor menu in the client configuration\r\nmenu to create your customized color scheme."
#define ENEMY_INFO	\
"You can now turn off the notification of killed posts and express messages\r\nfrom people on your enemy list.\r\n\nSelect Yes to be notified, or No to not be notified."
#define SELECT_URL	\
"You can now go directly to a Web site address you see in a post or express\r\nmessage by pressing the command key and <w>.  You can also change this key in\r\nthe client configuration.  You can define a browser in the client configuration\r\nor otherwise I will try to start Netscape."
#define ADVANCEDOPTIONS	\
"Advanced users may wish to use the configuration menu now to change options\r\nbefore logging in."

static void editusers (const char *name);
static void express_config (void);
static const char *strctrl (int c);
static int newkey (int oldkey);
static void newmacro (int which);
static void newawaymsg (void);

/*
 * First time setup borrowed from Client 9 with permission.
 */

/*
 * Performs first time setup for new features.
 */
void
setup (int newVersion)
{
    setterm ();
    if (newVersion < 1) {
        std_printf (GREETING);
    }
    else if (newVersion > INTVERSION) {
        if (!s_prompt (DOWNGRADE, "Continue running this client?", 0))
            myexit ();
    }
    else {
        s_info (UPGRADE, "Upgrade");
    }
    fflush (stdout);

    /* bbsrc file */
    if (newVersion < 5) {
        if (!s_prompt (BBSRC_INFO, "Continue running this client?", 1))
            myexit ();
    }
    if (newVersion < 220) {
        if (s_prompt (ENEMY_INFO, "Notify when posts and express messages from enemies are killed?", 1)) {
            flags.squelchpost = 0;
            flags.squelchexpress = 0;
        }
        else {
            flags.squelchpost = 1;
            flags.squelchexpress = 1;
        }

        fflush (stdout);
        s_info (COLOR_INFO, "Colors");
    }
    if (newVersion < 237)
        s_info (SELECT_URL, "Web sites");
    if (s_prompt (ADVANCEDOPTIONS, "Configure the client now?", 0))
        configbbsrc ();
    else
        writebbsrc (bbsrc);
    resetterm ();
}


/*
 * Changes settings in bbsrc file and saves it. 
 */
void
configbbsrc (void)
{
    char    tmp[80];
    int     c;
    int     i;
    int     j;
    unsigned int invalid;
    int     lines;

    flags.configflag = 1;
    if (bbsrcro)
        std_printf ("\r\nConfiguration file is read-only, cannot save configuration for next session.\r\n");
    else if (!bbsrc)
        std_printf ("\r\nNo configuration file, cannot save configuration for next session.\r\n");
    for (;;) {
        if (flags.useansi)
            colorize
                ("\r\n@YC@Color  @YE@Cnemy list  @YF@Criend list  @YH@Cotkeys\r\n@YI@Cnfo  @YM@Cacros  @YO@Cptions  @YX@Cpress  @YQ@Cuit@Y");
        else
            std_printf
                ("\r\n<C>olor <E>nemy list <F>riend list <H>otkeys\r\n<I>nfo  <M>acros <O>ptions <X>press <Q>uit");
        colorize ("\r\nClient config -> @G");
        for (invalid = 0;;) {
            c = tolower (inkey ());
            if (!strchr ("CcEeFfHhIiKkMmOoQqXx \n", c)) {
                if (invalid++)
                    flush_input (invalid);
                continue;
            }
            break;
        }
        switch (c) {
        case 'c':
            color_config ();
            break;

        case 'x':
            express_config ();
            break;

        case 'i':
            information ();
            break;

        case 'o':
            std_printf ("Options\r\n");
            if (!login_shell) {
                std_printf ("\r\nEnter name of local editor to use (%s) -> ", editor);
                get_string (72, tmp, -999);
                if (*tmp)
                    strcpy (editor, tmp);
            }
            std_printf ("Show long who list by default? (%s) -> ", (keymap['w'] == 'w') ? "No" : "Yes");
            if (yesnodefault ((keymap['w'] != 'w') ? 1 : 0)) {
                keymap['w'] = 'W';
                keymap['W'] = 'w';
            }
            else {
                keymap['w'] = 'w';
                keymap['W'] = 'W';
            }
            std_printf ("Show full profile by default? (%s) -> ", (keymap['p'] == 'p') ? "No" : "Yes");
            if (yesnodefault ((keymap['p'] != 'p') ? 1 : 0)) {
                keymap['p'] = 'P';
                keymap['P'] = 'p';
            }
            else {
                keymap['p'] = 'p';
                keymap['P'] = 'P';
            }
            std_printf ("Enter name of site to connect to (%s) -> ", bbshost);
            get_string (64, tmp, -999);
            if (*tmp)
                strcpy (bbshost, tmp);
#if 0
            std_printf ("Use secure (SSL) connection to this site? (%s) -> ", want_ssl ? "Yes" : "No");
            if (yesnodefault (want_ssl))
                want_ssl = 1;
            else
#endif
                want_ssl = false;
            if ((!bbsport || bbsport == BBSPORT) && want_ssl)
                bbsport = SSLPORT;
            else if ((!bbsport || bbsport == SSLPORT) && !want_ssl)
                bbsport = BBSPORT;
            std_printf ("Enter port number to connect to (%d) -> ", bbsport);
            get_string (5, tmp, -999);
            if (*tmp)
                bbsport = (unsigned short) atoi (tmp);
            if (!bbsport) {
                if (want_ssl) {
                    bbsport = SSLPORT;
                }
                else {
                    bbsport = BBSPORT;
                }
            }
#ifndef USE_CYGWIN
            /* The browser setting is ignored in Windows */
            std_printf ("Enter the Web browser to use (%s) -> ", browser);
            get_string (80, tmp, -999);
            if (*tmp)
                strncpy (browser, tmp, 80);
            std_printf ("Does %s run in a separate window? (%s) -> ", browser,
                        flags.browserbg ? "Yes" : "No");
            flags.browserbg = yesnodefault (flags.browserbg);
#else
            flags.browserbg = 1;
#endif
#ifdef USE_CYGWIN
            std_printf ("Use graphical controls such as dialog boxes? (%s) -> ", textonly ? "Yes" : "No");
            textonly = yesnodefault (textonly);
#endif
            std_printf ("Auto-fix posts before sending? (%s) -> ", flags.autofix_posts ? "Yes" : "No");
            flags.autofix_posts = yesnodefault (flags.autofix_posts);
            break;

        case 'h':
            std_printf ("Hotkeys\r\n\n");
            std_printf ("Enter command key (%s) -> ", strctrl (commandkey));
            for (;;) {
                std_printf ("%s\r\n", strctrl (commandkey = newkey (commandkey)));
                if (commandkey < ' ')
                    break;
                std_printf ("You must use a control character for your command key, try again -> ");
            }
            std_printf ("Enter key to quit client (%s) -> ", strctrl (quitkey));
            std_printf ("%s\r\n", strctrl (quitkey = newkey (quitkey)));
            if (!login_shell) {
                std_printf ("Enter key to suspend client (%s) -> ", strctrl (suspkey));
                std_printf ("%s\r\n", strctrl (suspkey = newkey (suspkey)));
                std_printf ("Enter key to start a new shell (%s) -> ", strctrl (shellkey));
                std_printf ("%s\r\n", strctrl (shellkey = newkey (shellkey)));
            }
            std_printf ("Enter key to toggle capture mode (%s) -> ", strctrl (capturekey));
            std_printf ("%s\r\n", strctrl (capturekey = newkey (capturekey)));
            std_printf ("Enter key to enable away from keyboard (%s) -> ", strctrl (awaykey));
            std_printf ("%s\r\n", strctrl (awaykey = newkey (awaykey)));
            std_printf ("Enter key to browse a Web site (%s) -> ", strctrl (browserkey));
            std_printf ("%s\r\n", strctrl (browserkey = newkey (browserkey)));
            break;

        case 'f':
            std_printf ("Friend list\r\n");
            editusers ("Friend");
            break;

        case 'e':
            std_printf ("Enemy list\r\n");
            editusers ("Enemy");
            break;

        case 'm':
            std_printf ("Macros\r\n");
            for (; c != 'q';) {
                if (flags.useansi)
                    colorize ("\r\n@YE@Cdit  @YL@Cist  @YQ@Cuit\r\n@YMacro config -> @G");
                else
                    std_printf ("\r\n<E>dit <L>ist <Q>uit\r\nMacro config -> ");
                for (invalid = 0;;) {
                    c = tolower (inkey ());
                    if (!strchr ("EeLlQq \n", c)) {
                        if (invalid++)
                            flush_input (invalid);
                        continue;
                    }
                    break;
                }
                switch (c) {
                case 'e':
                    std_printf ("Edit\r\n");
                    for (;;) {
                        std_printf ("\r\nMacro to edit (%s to end) -> ", strctrl (commandkey));
                        c = newkey (-1);
                        if (c == commandkey || c == ' ' || c == '\n' || c == '\r')
                            break;
                        std_printf ("%s\r\n", strctrl (c));
                        newmacro (c);
                    }
                    std_printf ("Done\r\n");
                    break;

                case 'l':
                    std_printf ("List\r\n\n");
                    for (i = 0, lines = 1; i < 128; i++)
                        if (*macro[i]) {
                            std_printf ("'%s': \"", strctrl (i));
                            for (j = 0; macro[i][j]; j++)
                                std_printf ("%s", strctrl (macro[i][j]));
                            std_printf ("\"\r\n");
                            if (++lines == rows - 1 && more (&lines, -1) < 0)
                                break;
                        }
                    break;

                case 'q':
                case ' ':
                case '\n':
                    std_printf ("Quit\r\n");
                    c = 'q';    // this forces the end of the for-loop.
                    break;
                }
            }
            break;

        case 'q':
        case ' ':
        case '\n':
            std_printf ("Quit\r\n");
            flags.configflag = 0;
            if (!bbsrcro && bbsrc)
                writebbsrc (bbsrc);
            return;
            /* NOTREACHED */

        default:
            break;
        }
    }
}


static void
express_config (void)
{

    std_printf ("Express\r\n");

    for (;;) {
        if (flags.useansi)
            colorize ("\r\n@YA@Cway  @YX@CLand  @YQ@Cuit\r\n@YExpress config -> @G");
        else
            std_printf ("\r\n<A>way <X>Land <Q>uit\r\nExpress config -> ");

        unsigned int invalid = 0;
        char    c;

        for (;;) {
            c = inkey ();
            if (!strchr ("AaXxQq \n", c)) {
                if (invalid++)
                    flush_input (invalid);
                continue;
            }
            break;
        }

        switch (c) {
        case 'a':
        case 'A':
            std_printf ("Away from keyboard\r\n\n");
            newawaymsg ();
            break;

        case 'x':
        case 'X':
            std_printf ("XLand\r\n\nAutomatically reply to X messages you receive? (%s) -> ",
                        xland ? "Yes" : "No");
            xland = yesnodefault (xland);
            break;

        case 'q':
        case 'Q':
        case ' ':
        case '\n':
            std_printf ("Quit\r\n");
            return;
            /* NOTREACHED */

        default:
            break;
        }
    }
}


static void
newawaymsg (void)
{
    int     i;

    if (**awaymsg) {
        std_printf ("Current away from keyboard message is:\r\n\n");
        for (i = 0; i < 5 && *awaymsg[i]; i++)
            std_printf (" %s\r\n", awaymsg[i]);
        std_printf ("\r\nDo you wish to change this? -> ");
        if (!yesno ())
            return;
        std_printf ("\r\nOk, you have five lines to do something creative.\r\n\n");
    }
    else {
        std_printf ("Enter a message, up to 5 lines\r\n\n");
    }
    for (i = 0; i < 5; i++)
        *awaymsg[i] = 0;
    for (i = 0; i < 5 && (!i || *awaymsg[i - 1]); i++) {
        std_printf (">");
        get_string (i ? 78 : 74, awaymsg[i], i);
    }
}

void
writebbsrc (FILE * fp)
{
    char    junk[40];
    int     i, j;

    unlink (bbsfriendsname);
    rewind (fp);
    fprintf (fp, "editor %s\n", editor);
    /* Change:  site line will always be written */
    fprintf (fp, "site %s %d%s\n", bbshost, bbsport, want_ssl ? " secure" : "");
    fprintf (fp, "commandkey %s\n", strctrl (commandkey));
    fprintf (fp, "quit %s\n", strctrl (quitkey));
    fprintf (fp, "susp %s\n", strctrl (suspkey));
    fprintf (fp, "shell %s\n", strctrl (shellkey));
    fprintf (fp, "capture %s\n", strctrl (capturekey));
    fprintf (fp, "awaykey %s\n", strctrl (awaykey));
    fprintf (fp, "squelch %d\n", (flags.squelchpost ? 2 : 0) + (flags.squelchexpress ? 1 : 0));
    fprintf (fp, "autofix_posts %d\n", (flags.autofix_posts ? 1 : 0));
    fprintf (fp, "browser %d %s\n", flags.browserbg ? 1 : 0, browser);
    if (*autoname)
        fprintf (fp, "autoname %s\n", autoname);
    if (*autopasswd)
        fprintf (fp, "autopass %s\n", autopasswd);
    bcopy ((void *) &color, junk, sizeof color);
    junk[sizeof color] = 0;
    fprintf (fp, "color %s\n", junk);
    if (flags.ansiprompt)
        fprintf (fp, "autoansi\n");
    if (**awaymsg) {
        for (i = 0; i < 5 && *awaymsg[i]; i++) {
            fprintf (fp, "a%d %s\n", i + 1, awaymsg[i]);
        }
    }
    fprintf (fp, "version %d\n", version);
    if (flags.usebold)
        fprintf (fp, "bold\n");
    if (textonly)
        fprintf (fp, "textonly\n");
    if (!xland)
        fprintf (fp, "xland\n");
    for (size_t i = 0; i < friendList.sz; ++i) {
        const UserEntry *pf = friendList.arr[i];

        fprintf (fp, "friend %-20s   %s\n", pf->name, pf->info);
    }
    for (size_t i = 0; i < enemyList.sz; ++i)
        fprintf (fp, "enemy %s\n", enemyList.arr[i]->name);
    for (size_t i = 0; i < 128; i++)
        if (*macro[i]) {
            fprintf (fp, "macro %s ", strctrl (i));
            for (j = 0; macro[i][j]; j++)
                fprintf (fp, "%s", strctrl (macro[i][j]));
            fprintf (fp, "\n");
        }
    for (int i = 33; i < 128; i++)
        if (keymap[i] != i)
            fprintf (fp, "keymap %c %c\n", i, keymap[i]);
    fflush (fp);
    truncfp (fp, ftell (fp));
}


/*
 * Gets a new hotkey value or returns the old value if the default is taken. If
 * the old value is specified as -1, no checking is done to see if the new
 * value doesn't conflict with other hotkeys.  Calls getkey() instead of
 * inkey() to avoid the character translation (since the hotkey values are
 * checked within inkey() instead of getkey()) 
 */
static int
newkey (int oldkey)
{
    for (;;) {
        int     c = getkey ();

        if (((c == ' ' || c == '\n' || c == '\r') && oldkey >= 0) || c == oldkey)
            return oldkey;
        if (oldkey >= 0
            && (c == commandkey || c == suspkey || c == quitkey || c == shellkey || c == capturekey
                || c == awaykey || c == browserkey))
            std_printf ("\r\nThat key is already in use for another hotkey, try again -> ");
        else
            return c;
    }
}



/*
 * Gets a new value for macro 'which'. 
 */
static void
newmacro (int which)
{

    if (*macro[which]) {
        std_printf ("\r\nCurrent macro for '%s' is: \"", strctrl (which));
        for (int i = 0; macro[which][i]; i++)
            std_printf ("%s", strctrl (macro[which][i]));
        std_printf ("\"\r\nDo you wish to change this? (Y/N) -> ");
    }
    else
        std_printf ("\r\nNo current macro for '%s'.\r\nDo you want to make one? (Y/N) -> ", strctrl (which));
    if (!yesno ())
        return;
    std_printf ("\r\nEnter new macro (use %s to end)\r\n -> ", strctrl (commandkey));
    for (int i = 0;; i++) {
        int     c = inkey ();

        if (c == '\b') {
            if (i) {
                if (macro[which][i - 1] < ' ')
                    printf ("\b \b");
                i--;
                printf ("\b \b");
            }
            i--;
            continue;
        }
        if (c == commandkey) {
            macro[which][i] = 0;
            for (i = 0; macro[which][i]; i++)   /* Shut up!! */
                cap_puts (strctrl (macro[which][i]));
            std_printf ("\r\n");
            return;
        }
        else if (i == 70) {
            i--;
            continue;
        }
        printf ("%s", strctrl (macro[which][i] = c));
    }
}



/*
 * Returns a string representation of c suitable for printing.  If c is a
 * regular character it will be printed normally, if it is a control character
 * it is printed as in the Unix ctlecho mode (i.e. ctrl-A is printed as ^A) 
 */
static const char *
strctrl (int c)
{
    static char ret[3] = { 0, 0, 0 };

    if (c <= 31 || c == DEL) {
        ret[0] = '^';
        ret[1] = c == 10 ? 'M' : c ^ 0x40;
    }
    else {
        ret[0] = c;
        ret[1] = 0;
    }
    return ret;
}



/*
 * Does the editing of the friend and enemy lists. 
 */
static void
editusers (const char *list_name)
{
    unsigned int invalid = 0;
    char   *sp;
    char    nfo[50];
    char    work[80];

    const bool is_f = (list_name[0] == 'F');    // we are editing either the friendList or the enemyList.
    UList  *const L = is_f ? &friendList : &enemyList;
    const bool has_info = is_f; // friendList has info
    const bool has_options_menu = !is_f;    // enemyList has Options menu.

    for (;;) {
        /* Build menu */
        if (has_options_menu)
            if (flags.useansi)
                colorize ("\r\n@YA@Cdd  @YD@Celete  @YL@Cist  @YO@Cptions  @YQ@Cuit@Y");
            else
                std_printf ("\r\n<A>dd <D>elete <L>ist <O>ptions <Q>uit");
        else if (flags.useansi)
            colorize ("\r\n@YA@Cdd  @YD@Celete  @YE@Cdit  @YL@Cist  @YQ@Cuit@Y");
        else
            std_printf ("\r\n<A>dd <D>elete <E>dit <L>ist <Q>uit");
        sprintf (work, "\r\n%s list -> @G", list_name);
        colorize (work);

        const int c = tolower (inkey ());

        switch (c) {
        case 'a':
            std_printf ("Add\r\n");
            std_printf ("\r\nUser to add to your %s list -> ", list_name);
            sp = get_name (-999);
            if (*sp) {
                if (ulist_find (L, sp)) {
                    std_printf ("\r\n%s is already on your %s list.\r\n", sp, list_name);
                    break;
                }
                if (has_info) {
                    UserEntry *pf = ulist_insert (L, sp);

                    std_printf ("Enter info for %s: ", pf->name);
                    get_string (48, nfo, -999);
                    strcpy (pf->info, (*nfo) ? nfo : "(None)");
                }
                else
                    ulist_insert (L, sp);

                std_printf ("\r\n%s was added to your %s list.\r\n", sp, list_name);
            }
            break;

        case 'd':
            std_printf ("Delete\r\n\nUser to delete from your %s list -> ", list_name);
            sp = get_name (-999);
            if (*sp) {
                if (ulist_erase (L, sp))
                    std_printf ("\r\n%s was deleted from your %s list.\r\n", sp, list_name);
                else
                    std_printf ("\r\n%s is not in your %s list.\r\n", sp, list_name);
            }
            break;

        case 'e':
            if (has_info) {     // the Edit menu is really just for the info field.
                std_printf ("Edit\r\nName of user to edit: ");
                sp = get_name (-999);
                if (*sp) {
                    UserEntry *pf = ulist_find (L, sp);

                    if (pf) {
                        std_printf ("Current info: %s\r\n", pf->info);
                        std_printf ("Return to leave unchanged, NONE to erase.\r\n");
                        std_printf ("Enter new info: ");
                        get_string (48, nfo, -999);
                        if (*nfo) {
                            if (!strcmp (nfo, "NONE"))
                                strcpy (pf->info, "(None)");
                            else
                                strcpy (pf->info, nfo);
                        }
                    }
                    else
                        std_printf ("\r\n%s is not in your %s list.\r\n", sp, list_name);
                }
                break;
            }
            else {
                if (invalid++)
                    flush_input (invalid);
                continue;
            }

        case 'l':
            std_printf ("List\r\n\n");
            ulist_sort_by_name (L);
            if (has_info) {
                int     lines = 1;

                for (size_t i = 0; i < L->sz; i++) {
                    const UserEntry *pf = L->arr[i];

                    sprintf (work, "@Y%-20s @C%s@G\r\n", pf->name, pf->info);
                    colorize (work);
                    lines++;
                    if (lines == rows - 1 && more (&lines, -1) < 0)
                        break;
                }
            }
            else {
                int     lines = 1;
                size_t  i;

                for (i = 0; i < L->sz; i++) {
                    std_printf ("%-19s%s", L->arr[i]->name, (i % 4) == 3 ? "\r\n" : " ");
                    if ((i % 4) == 3)
                        lines++;
                    if (lines == rows - 1 && more (&lines, -1) < 0)
                        break;
                }
                if (i % 4)
                    std_printf ("\r\n");
            }
            break;

        case 'q':
        case '\n':
        case ' ':
            std_printf ("Quit\r\n");
            return;

        case 'o':
            if (has_options_menu) {
                std_printf ("Options\r\n\nNotify when an enemy's post is killed? (%s) -> ",
                            flags.squelchpost ? "No" : "Yes");
                flags.squelchpost = !yesnodefault (!flags.squelchpost);
                std_printf ("Notify when an enemy's eXpress message is killed? (%s) -> ",
                            flags.squelchexpress ? "No" : "Yes");
                flags.squelchexpress = !yesnodefault (!flags.squelchexpress);
            }
            /* Fall through TODO: Why do we fall through here? */

        default:
            if (invalid++)
                flush_input (invalid);
            continue;
        }
        invalid = 0;
    }
}
