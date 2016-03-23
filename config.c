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

/*
 * First time setup borrowed from Client 9 with permission.
 */

/*
 * Performs first time setup for new features.
 */
void setup(int newVersion)
{
    setterm();
    if (newVersion < 1) {
	std_printf(GREETING);
    } else if (newVersion > INTVERSION) {
	if (!s_prompt(DOWNGRADE, "Continue running this client?", 0))
	    myexit();
    } else {
	s_info(UPGRADE, "Upgrade");
    }
    fflush(stdout);

    /* bbsrc file */
    if (newVersion < 5) {
	if (!s_prompt(BBSRC_INFO, "Continue running this client?", 1))
	    myexit();
    }
    if (newVersion < 220) {
	if (s_prompt(ENEMY_INFO, "Notify when posts and express messages from enemies are killed?", 1)) {
	    flags.squelchpost = 0;
	    flags.squelchexpress = 0;
	} else {
	    flags.squelchpost = 1;
	    flags.squelchexpress = 1;
	}

	fflush(stdout);
	s_info(COLOR_INFO, "Colors");
    }
    if (newVersion < 237)
	s_info(SELECT_URL, "Web sites");
    if (s_prompt(ADVANCEDOPTIONS, "Configure the client now?", 0)) 
	configbbsrc();
    else
	writebbsrc();
    resetterm();
    return;
}


/*
 * Changes settings in bbsrc file and saves it. 
 */
void configbbsrc()
{
    char tmp[80];
    register int c;
    register int i;
    register int j;
    unsigned int invalid;
    int lines;

    flags.configflag = 1;
    if (bbsrcro)
	std_printf("\r\nConfiguration file is read-only, cannot save configuration for next session.\r\n");
    else if (!bbsrc)
	std_printf("\r\nNo configuration file, cannot save configuration for next session.\r\n");
    for (;;) {
#ifdef ENABLE_SOCKS
	if (flags.useansi)
	    colorize("\r\n@YC@Color  @YE@Cnemy list  @YF@Criend list  @YH@Cotkeys\r\n@YI@Cnfo  @YM@Cacros  @YO@Cptions  @YP@Croxy  @YX@Cpress  @YQ@Cuit@Y");
	else
	    std_printf("\r\n<C>olor <E>nemy list <F>riend list <H>otkeys\r\n<I>nfo  <M>acros <O>ptions <P>roxy <X>press <Q>uit");
#else
	if (flags.useansi)
	    colorize("\r\n@YC@Color  @YE@Cnemy list  @YF@Criend list  @YH@Cotkeys\r\n@YI@Cnfo  @YM@Cacros  @YO@Cptions  @YX@Cpress  @YQ@Cuit@Y");
	else
	    std_printf("\r\n<C>olor <E>nemy list <F>riend list <H>otkeys\r\n<I>nfo  <M>acros <O>ptions <X>press <Q>uit");
#endif
	colorize("\r\nClient config -> @G");
	for (invalid = 0;;) {
	    c = inkey();
#ifdef ENABLE_SOCKS
	    if (!mystrchr("CcEeFfHhIiKkMmOoPpQqXx \n", c)) {
#else
	    if (!mystrchr("CcEeFfHhIiKkMmOoQqXx \n", c)) {
#endif
		if (invalid++)
		    flush_input(invalid);
		continue;
	    }
	    break;
	}
	switch (c) {
	case 'c':
	case 'C':
	    color_config();
	    break;

	case 'x':
	case 'X':
	    express_config();
	    break;

	case 'i':
	case 'I':
	    information();
	    break;

#ifdef ENABLE_SOCKS
	case 'p':
	case 'P':
	    std_printf("Proxy\r\n");
	    std_printf("\nUse SOCKS firewall to connect? (%s) -> ", use_socks ? "Yes" : "No");
	    use_socks = yesnodefault(use_socks);
	    if (use_socks) {
		std_printf("Enter name of SOCKS server (%s) -> ", socks_fw);
		get_string(64, tmp, -999);
		if (*tmp) {
		    strcpy(socks_fw, tmp);
		    std_printf("Enter SOCKS port number (%d) ->", socks_fw_port ? socks_fw_port : 1080);
		    get_string(5, tmp, -999);
		    if (*tmp)
			socks_fw_port = (unsigned short) atoi(tmp);
		    if (!socks_fw_port)
			socks_fw_port = 1080;
		} else {
		    use_socks = 0;
		}
	    }
	    break;
#endif

	case 'o':
	case 'O':
	    std_printf("Options\r\n");
	    if (!login_shell) {
		std_printf("\r\nEnter name of local editor to use (%s) -> ", editor);
		get_string(72, tmp, -999);
		if (*tmp)
		    strcpy(editor, tmp);
	    }
	    std_printf("Show long who list by default? (%s) -> ", (keymap['w'] == 'w') ? "No" : "Yes");
	    if (yesnodefault((keymap['w'] != 'w') ? 1 : 0)) {
		keymap['w'] = 'W';
		keymap['W'] = 'w';
	    } else {
		keymap['w'] = 'w';
		keymap['W'] = 'W';
	    }
	    std_printf("Show full profile by default? (%s) -> ", (keymap['p'] == 'p') ? "No" : "Yes");
	    if (yesnodefault((keymap['p'] != 'p') ? 1 : 0)) {
		keymap['p'] = 'P';
		keymap['P'] = 'p';
	    } else {
		keymap['p'] = 'p';
		keymap['P'] = 'P';
	    }
	    std_printf("Enter name of site to connect to (%s) -> ", bbshost);
	    get_string(64, tmp, -999);
	    if (*tmp)
		strcpy(bbshost, tmp);
#if 0
	    std_printf("Use secure (SSL) connection to this site? (%s) -> ",
			    want_ssl ? "Yes" : "No");
	    if (yesnodefault(want_ssl))
		want_ssl = 1;
	    else
#endif
		want_ssl = 0;
	    if ((!bbsport || bbsport == BBSPORT) && want_ssl)
		bbsport = SSLPORT;
	    else if ((!bbsport || bbsport == SSLPORT) && !want_ssl)
		bbsport = BBSPORT;
	    std_printf("Enter port number to connect to (%d) -> ", bbsport);
	    get_string(5, tmp, -999);
	    if (*tmp)
		bbsport = (unsigned short) atoi(tmp);
	    if (!bbsport) {
		if (want_ssl) {
		    bbsport = SSLPORT;
		} else {
		    bbsport = BBSPORT;
		}
	    }
#ifndef USE_CYGWIN
	    /* The browser setting is ignored in Windows */
	    std_printf("Enter the Web browser to use (%s) -> ", browser);
	    get_string(80, tmp, -999);
	    if (*tmp)
		strncpy(browser, tmp, 80);
	    std_printf("Does %s run in a separate window? (%s) -> ", browser,
			    flags.browserbg ? "Yes" : "No");
	    flags.browserbg = yesnodefault(flags.browserbg);
#else
	    flags.browserbg = 1;
#endif
#ifdef USE_CYGWIN
	    std_printf("Use graphical controls such as dialog boxes? (%s) -> ",
			    textonly ? "Yes" : "No");
	    textonly = yesnodefault(textonly);
#endif
	    break;

	case 'h':
	case 'H':
	    std_printf("Hotkeys\r\n\n");
	    std_printf("Enter command key (%s) -> ", strctrl(commandkey));
	    for (;;) {
		std_printf("%s\r\n", strctrl(commandkey = newkey(commandkey)));
		if (commandkey < ' ')
		    break;
		std_printf("You must use a control character for your command key, try again -> ");
	    }
	    std_printf("Enter key to quit client (%s) -> ", strctrl(quitkey));
	    std_printf("%s\r\n", strctrl(quitkey = newkey(quitkey)));
	    if (!login_shell) {
		std_printf("Enter key to suspend client (%s) -> ", strctrl(suspkey));
		std_printf("%s\r\n", strctrl(suspkey = newkey(suspkey)));
		std_printf("Enter key to start a new shell (%s) -> ", strctrl(shellkey));
		std_printf("%s\r\n", strctrl(shellkey = newkey(shellkey)));
	    }
	    std_printf("Enter key to toggle capture mode (%s) -> ", strctrl(capturekey));
	    std_printf("%s\r\n", strctrl(capturekey = newkey(capturekey)));
	    std_printf("Enter key to enable away from keyboard (%s) -> ", strctrl(awaykey));
	    std_printf("%s\r\n", strctrl(awaykey = newkey(awaykey)));
	    std_printf("Enter key to browse a Web site (%s) -> ", strctrl(browserkey));
	    std_printf("%s\r\n", strctrl(browserkey = newkey(browserkey)));
	    break;

	case 'f':
	case 'F':
	    std_printf("Friend list\r\n");
	    editusers(friendList, (int (*)())fstrcmp, "friend");
	    break;

	case 'e':
	case 'E':
	    std_printf("Enemy list\r\n");
	    editusers(enemyList, (int (*)())strcmp, "enemy");
	    break;

	case 'm':
	case 'M':
	    std_printf("Macros\r\n");
	    for (; c != 'q';) {
		if (flags.useansi)
		    colorize("\r\n@YE@Cdit  @YL@Cist  @YQ@Cuit\r\n@YMacro config -> @G");
		else
		    std_printf("\r\n<E>dit <L>ist <Q>uit\r\nMacro config -> ");
		for (invalid = 0;;) {
		    c = inkey();
		    if (!mystrchr("EeLlQq \n", c)) {
			if (invalid++)
			    flush_input(invalid);
			continue;
		    }
		    break;
		}
		switch (c) {
		case 'e':
		case 'E':
		    std_printf("Edit\r\n");
		    for (;;) {
			std_printf("\r\nMacro to edit (%s to end) -> ", strctrl(commandkey));
			c = newkey(-1);
			if (c == commandkey || c == ' ' || c == '\n' || c == '\r')
			    break;
			std_printf("%s\r\n", strctrl(c));
			newmacro(c);
		    }
		    std_printf("Done\r\n");
		    break;

		case 'l':
		case 'L':
		    std_printf("List\r\n\n");
		    for (i = 0, lines = 1; i < 128; i++)
			if (*macro[i]) {
			    std_printf("'%s': \"", strctrl(i));
			    for (j = 0; macro[i][j]; j++)
				std_printf("%s", strctrl(macro[i][j]));
			    std_printf("\"\r\n");
			    if (++lines == rows - 1 && more(&lines, -1) < 0)
				break;
			}
		    break;

		case 'q':
		case 'Q':
		case ' ':
		case '\n':
		    std_printf("Quit\r\n");
		    c = 'q';
		    break;
		}
	    }
	    break;

	case 'q':
	case 'Q':
	case ' ':
	case '\n':
	    std_printf("Quit\r\n");
	    flags.configflag = 0;
	    if (bbsrcro || !bbsrc)
		return;
	    writebbsrc();
	    return;
	    /* NOTREACHED */

	default:
	    break;
	}
    }
}


void express_config(void)
{
    unsigned int invalid = 0;
    char c;

    std_printf("Express\r\n");

    for (;;) {
	if (flags.useansi)
	    colorize("\r\n@YA@Cway  @YX@CLand  @YQ@Cuit\r\n@YExpress config -> @G");
	else
	    std_printf("\r\n<A>way <X>Land <Q>uit\r\nExpress config -> ");

	for (invalid = 0;;) {
	    c = inkey();
	    if (!mystrchr("AaXxQq \n", c)) {
		if (invalid++)
		    flush_input(invalid);
		continue;
	    }
	    break;
	}

	switch (c) {
	case 'a':
	case 'A':
	    std_printf("Away from keyboard\r\n\n");
	    newawaymsg();
	    break;

	case 'x':
	case 'X':
	    std_printf("XLand\r\n\nAutomatically reply to X messages you receive? (%s) -> ", xland ? "Yes" : "No");
	    xland = yesnodefault(xland);
	    break;

	case 'q':
	case 'Q':
	case ' ':
	case '\n':
	    std_printf("Quit\r\n");
	    return;
	    /* NOTREACHED */

	default:
	    break;
	}
    }
}


void newawaymsg(void)
{
    int i;

    if (**awaymsg) {
	std_printf("Current away from keyboard message is:\r\n\n");
	for (i = 0; i < 5 && *awaymsg[i]; i++)
	    std_printf(" %s\r\n", awaymsg[i]);
	std_printf("\r\nDo you wish to change this? -> ");
	if (!yesno())
	    return;
	std_printf("\r\nOk, you have five lines to do something creative.\r\n\n");
    } else {
	std_printf("Enter a message, up to 5 lines\r\n\n");
    }
    for (i = 0; i < 5; i++)
	    *awaymsg[i] = 0;
    for (i = 0; i < 5 && (!i || *awaymsg[i - 1]); i++) {
	    std_printf(">");
	    get_string(i ? 78 : 74, awaymsg[i], i);
    }
}

void writebbsrc(void)
{
    char junk[40];
    int i, j;
    friend *pf;

    deletefile(bbsfriendsname);
    rewind(bbsrc);
    fprintf(bbsrc, "editor %s\n", editor);
    /* Change:  site line will always be written */
    fprintf(bbsrc, "site %s %d%s\n", bbshost, bbsport,
		    want_ssl ? " secure" : "");
    if (use_socks) {
	if (socks_fw_port == 1080) {
	    fprintf(bbsrc, "socks %s\n", socks_fw);
	} else {
	    fprintf(bbsrc, "socks %s %d\n", socks_fw, socks_fw_port);
	}
    }
    fprintf(bbsrc, "commandkey %s\n", strctrl(commandkey));
    fprintf(bbsrc, "quit %s\n", strctrl(quitkey));
    fprintf(bbsrc, "susp %s\n", strctrl(suspkey));
    fprintf(bbsrc, "shell %s\n", strctrl(shellkey));
    fprintf(bbsrc, "capture %s\n", strctrl(capturekey));
    fprintf(bbsrc, "awaykey %s\n", strctrl(awaykey));
    fprintf(bbsrc, "squelch %d\n", (flags.squelchpost ? 2 : 0) + (flags.squelchexpress ? 1 : 0));
    fprintf(bbsrc, "browser %d %s\n", flags.browserbg ? 1 : 0, browser);
    if (*autoname)
	fprintf(bbsrc, "autoname %s\n", autoname);
#ifdef ENABLE_SAVE_PASSWORD
    if (*autopasswd)
	fprintf(bbsrc, "autopass %s\n", autopasswd);
#endif
    bcopy((void *)&color, junk, sizeof color);
    junk[sizeof color] = 0;
    fprintf(bbsrc, "color %s\n", junk);
    if (flags.ansiprompt)
	fprintf(bbsrc, "autoansi\n");
    if (**awaymsg) {
	for (i = 0; i < 5 && *awaymsg[i]; i++) {
		fprintf(bbsrc, "a%d %s\n", i + 1, awaymsg[i]);
	}
    }
    fprintf(bbsrc, "version %d\n", version);
    if (flags.usebold)
	fprintf(bbsrc, "bold\n");
    if (textonly)
	fprintf(bbsrc, "textonly\n");
    if (!xland)
	fprintf(bbsrc, "xland\n");
    for (i = 0; i < friendList->nitems; i++) {
	pf = (friend *) friendList->items[i];
	fprintf(bbsrc, "friend %-20s   %s\n", pf->name, pf->info);
    }
    for (i = 0; i < enemyList->nitems; i++)
	fprintf(bbsrc, "enemy %s\n", (char *) enemyList->items[i]);
    for (i = 0; i < 128; i++)
	if (*macro[i]) {
	    fprintf(bbsrc, "macro %s ", strctrl(i));
	    for (j = 0; macro[i][j]; j++)
		fprintf(bbsrc, "%s", strctrl(macro[i][j]));
	    fprintf(bbsrc, "\n");
	}
    for (i = 33; i < 128; i++)
	if (keymap[i] != i)
	    fprintf(bbsrc, "keymap %c %c\n", i, keymap[i]);
    fflush(bbsrc);
    truncbbsrc(ftell(bbsrc));
}


/*
 * Gets a new hotkey value or returns the old value if the default is taken. If
 * the old value is specified as -1, no checking is done to see if the new
 * value doesn't conflict with other hotkeys.  Calls getkey() instead of
 * inkey() to avoid the character translation (since the hotkey values are
 * checked within inkey() instead of getkey()) 
 */
int newkey(oldkey)
int oldkey;
{
    int c;

    for (;;) {
	c = getkey();
	if (((c == ' ' || c == '\n' || c == '\r') && oldkey >= 0) || c == oldkey)
	    return (oldkey);
	if (oldkey >= 0 && (c == commandkey || c == suspkey || c == quitkey || c == shellkey || c == capturekey || c == awaykey || c == browserkey))
	    std_printf("\r\nThat key is already in use for another hotkey, try again -> ");
	else
	    return (c);
    }
}



/*
 * Gets a new value for macro 'which'. 
 */
void newmacro(which)
int which;
{
    register int i;
    register int c;

    if (*macro[which]) {
	std_printf("\r\nCurrent macro for '%s' is: \"", strctrl(which));
	for (i = 0; macro[which][i]; i++)
	    std_printf("%s", strctrl(macro[which][i]));
	std_printf("\"\r\nDo you wish to change this? (Y/N) -> ");
    } else
	std_printf("\r\nNo current macro for '%s'.\r\nDo you want to make one? (Y/N) -> ", strctrl(which));
    if (!yesno())
	return;
    std_printf("\r\nEnter new macro (use %s to end)\r\n -> ", strctrl(commandkey));
    for (i = 0;; i++) {
	c = inkey();
	if (c == '\b') {
	    if (i) {
		if (macro[which][i - 1] < ' ')
		    printf("\b \b");
		i--;
		printf("\b \b");
	    }
	    i--;
	    continue;
	}
	if (c == commandkey) {
	    macro[which][i] = 0;
	    for (i = 0; macro[which][i]; i++)	/* Shut up!! */
		cap_printf("%s", strctrl(macro[which][i]));
	    std_printf("\r\n");
	    return;
	} else if (i == 70) {
	    i--;
	    continue;
	}
	printf("%s", strctrl(macro[which][i] = c));
    }
}



/*
 * Returns a string representation of c suitable for printing.  If c is a
 * regular character it will be printed normally, if it is a control character
 * it is printed as in the Unix ctlecho mode (i.e. ctrl-A is printed as ^A) 
 */
char *strctrl(c)
int c;
{
    static char ret[3];

    if (c <= 31 || c == DEL) {
	ret[0] = '^';
	ret[1] = c == 10 ? 'M' : c ^ 0x40;
    } else {
	ret[0] = c;
	ret[1] = 0;
    }
    ret[2] = 0;
    return (ret);
}



/*
 * Does the editing of the friend and enemy lists. 
 */
void editusers(list, findfn, name)
slist *list;
int (*findfn) (const void *, const void *);
const char *name;
{
    register int c;
    register int i = 0;
    unsigned int invalid = 0;
    int lines;
    char *sp;
    char nfo[50];
    char work[80];
    char *pc;
    friend *pf;

    for (;;) {
	/* Build menu */
	if (!strncmp(name, "enemy", 5))
	    if (flags.useansi)
		colorize("\r\n@YA@Cdd  @YD@Celete  @YL@Cist  @YO@Cptions  @YQ@Cuit@Y");
	    else
		std_printf("\r\n<A>dd <D>elete <L>ist <O>ptions <Q>uit");
	else if (flags.useansi)
	    colorize("\r\n@YA@Cdd  @YD@Celete  @YE@Cdit  @YL@Cist  @YQ@Cuit@Y");
	else
	    std_printf("\r\n<A>dd <D>elete <E>dit <L>ist <Q>uit");
	sprintf(work, "\r\n%c%s list -> @G", toupper(name[0]), name+1);
	colorize(work);

	c = inkey();
	switch (c) {
	case 'a':
	case 'A':
	    std_printf("Add\r\n");
	    std_printf("\r\nUser to add to your %s list -> ", name);
	    sp = get_name(-999);
	    if (*sp) {
		if (slistFind(list, sp, findfn) != -1) {
		    std_printf("\r\n%s is already on your %s list.\r\n", sp, name);
		    i = -1;
		}
#ifdef DEBUG
		printf("{%d %s} ", i, sp);
#endif
		if (i < 0)
		    break;
		if (!strcmp(name, "friend")) {
		    if (!(pf = (friend *) calloc(1, sizeof(friend))))
			fatalexit("Out of memory adding 'friend'!\n", "Fatal error");
		    strcpy(pf->name, sp);
		    std_printf("Enter info for %s: ", sp);
		    get_string(48, nfo, -999);
		    strcpy(pf->info, (*nfo) ? nfo : "(None)");
		    pf->magic = 0x3231;
		    if (!slistAddItem(list, pf, 0))
			fatalexit("Can't add 'friend'!\n", "Fatal error");
		} else {	/* enemy list */
		    pc = (char *) calloc(1, strlen(sp) + 1);
		    strcpy(pc, sp);	/* 2.1.2 bugfix */
		    if (!pc)
			fatalexit("Out of memory adding 'enemy'!\r\n", "Fatal error");
		    if (!slistAddItem(list, pc, 0))
			fatalexit("Can't add 'enemy'!\r\n", "Fatal error");
		}
		std_printf("\r\n%s was added to your %s list.\r\n", sp, name);
	    }
	    break;

	case 'd':
	case 'D':
	    std_printf("Delete\r\n\nUser to delete from your %s list -> ", name);
	    sp = get_name(-999);
	    if (*sp) {
		i = slistFind(list, sp, findfn);
		if (i != -1) {
		    free(list->items[i]);
		    if (!slistRemoveItem(list, i))
			fatalexit("Can't remove user!\r\n", "Fatal error");
		    std_printf("\r\n%s was deleted from your %s list.\r\n", sp, name);
		} else
		    std_printf("\r\n%s is not in your %s list.\r\n", sp, name);
	    }
	    break;

	case 'e':
	case 'E':
	    if (!strncmp(name, "friend", 6)) {
		std_printf("Edit\r\nName of user to edit: ");
		sp = get_name(-999);
		if (*sp) {
		    if ((i = slistFind(list, sp, findfn)) != -1) {
			pf = list->items[i];
			if (!strcmp(pf->name, sp)) {
			    std_printf("Current info: %s\r\n", pf->info);
			    std_printf("Return to leave unchanged, NONE to erase.\r\n");
			    std_printf("Enter new info: ");
			    get_string(48, nfo, -999);
			    if (*nfo) {
				if (!strcmp(nfo, "NONE")) {
				    strcpy(pf->info, "(None)");
				} else {
				    strcpy(pf->info, nfo);
				}
			    }
			}
		    } else {
			std_printf("\r\n%s is not in your %s list.\r\n", sp, name);
		    }
		}
		break;
	    } else {
		if (invalid++)
		    flush_input(invalid);
		continue;
	    }

	case 'l':
	case 'L':
	    std_printf("List\r\n\n");
	    if (!strcmp(name, "friend"))
		for (i = 0, lines = 1; i < list->nitems; i++) {
		    pf = list->items[i];
		    sprintf(work, "@Y%-20s @C%s@G\r\n", pf->name, pf->info);
		    colorize(work);
		    lines++;
		    if (lines == rows - 1 && more(&lines, -1) < 0)
			break;
	    } else {
		for (i = 0, lines = 1; i < list->nitems; i++) {
		    std_printf("%-19s%s", list->items[i], (i % 4) == 3 ? "\r\n" : " ");
		    if ((i % 4) == 3)
			lines++;
		    if (lines == rows - 1 && more(&lines, -1) < 0)
			break;
		}
		if (i % 4)
		    std_printf("\r\n");
	    }
	    break;

	case 'q':
	case 'Q':
	case '\n':
	case ' ':
	    std_printf("Quit\r\n");
	    return;

	case 'o':
	case 'O':
	    if (!strncmp(name, "enemy", 5)) {
		std_printf("Options\r\n\nNotify when an enemy's post is killed? (%s) -> ",
			   flags.squelchpost ? "No" : "Yes");
		flags.squelchpost = !yesnodefault(!flags.squelchpost);
		std_printf("Notify when an enemy's eXpress message is killed? (%s) -> ",
			   flags.squelchexpress ? "No" : "Yes");
		flags.squelchexpress = !yesnodefault(!flags.squelchexpress);
	    }
	    /* Fall through */

	default:
	    if (invalid++)
		flush_input(invalid);
	    continue;
	}
	invalid = 0;
    }
}
