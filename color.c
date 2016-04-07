/** @copyright GNU Public License. See COPYING. */
#include "defs.h"
#include "ext.h"

static char background_picker (void);
static char color_picker (void);
static char express_color_menu (void);
static char post_color_menu (void);
static char user_or_friend (void);
static void color_options (void);
static void express_color_config (void);
static void express_friend_color_config (void);
static void express_user_color_config (void);
static void general_color_config (void);
static void input_color_config (void);
static void post_color_config (void);
static void post_friend_color_config (void);
static void post_user_color_config (void);

/*
 * default_colors is called once with an arg of 1 before the bbsrc file is
 * read.  This initializes all the color variables.  It is then called again
 * after the bbsrc file is read, with an arg of 0.  This helps with reserved
 * fields which might become used after a user upgrades to a later version
 * which might use those reserved fields - they will get their default values
 * instead of zero, which would render as black.
 */
#define ifzero(x)	if ((x) < '1' || (x) > '7' || clearall)
void
default_colors (bool clearall)
{
    ifzero (color.text) color.text = '2';
    ifzero (color.forum) color.forum = '3';
    ifzero (color.number) color.number = '6';
    ifzero (color.err) color.err = '1';
    color.reserved1 = '0';
    color.reserved2 = '0';
    color.reserved3 = '0';
    ifzero (color.postdate) color.postdate = '5';
    ifzero (color.postname) color.postname = '6';
    ifzero (color.posttext) color.posttext = '2';
    ifzero (color.postfrienddate) color.postfrienddate = '5';
    ifzero (color.postfriendname) color.postfriendname = '1';
    ifzero (color.postfriendtext) color.postfriendtext = '2';
    ifzero (color.anonymous) color.anonymous = '3';
    ifzero (color.moreprompt) color.moreprompt = '3';
    color.reserved4 = '0';
    color.reserved5 = '0';
    if (clearall)
        color.background = '0';
    ifzero (color.input1) color.input1 = '2';
    ifzero (color.input2) color.input2 = '6';
    ifzero (color.expresstext) color.expresstext = '2';
    ifzero (color.expressname) color.expressname = '2';
    ifzero (color.expressfriendname) color.expressfriendname = '2';
    ifzero (color.expressfriendtext) color.expressfriendtext = '2';
}


char
ansi_transform (char c)
{
    switch (c) {
    case '6':
        return color.number;
    case '3':
        return color.forum;
    case '2':
        return color.text;
    case '1':
        return color.err;
    default:
        break;
    }

    return c;
}


// colorize the header of an xmsg
void
ansi_transform_express (char *s)
{
    /* Insert color only when ANSI is being used */
    if (!flags.useansi)
        return;

    /* Verify this is an X message and set up pointers */
    char   *sp1 = strstr (s, ") to ");
    char   *sp2 = strstr (s, ") from ");

    if (!sp1 && !sp2)
        return;
    if ((sp2 && sp2 < sp1) || !sp1)
        sp1 = sp2 + 2;

    sp2 = strstr (s, " at ");
    if (!sp2)
        return;

    sp1 += 4;
    *(sp1++) = 0;
    *(sp2++) = 0;

    if (ulist_find (&friendList, sp1))
        str_sprintf (scratch, "\033[3%cm%s \033[3%cm%s\033[3%cm %s\033[3%cm",
                     color.expressfriendtext, s, color.expressfriendname, sp1,
                     color.expressfriendtext, sp2, color.text);
    else
        str_sprintf (scratch, "\033[3%cm%s \033[3%cm%s\033[3%cm %s\033[3%cm",
                     color.expresstext, s, color.expressname, sp1, color.expresstext, sp2, color.text);
    lastcolor = color.text;
    strcpy (s, str_cdata (scratch));
}

char
ansi_transform_post (char c, bool isFriend)
{
    switch (c) {
    case '3':
        c = color.moreprompt;
        break;
    case '2':
        if (isFriend)
            c = color.postfriendtext;
        else
            c = color.posttext;
        break;
    case '1':
        c = color.err;
        break;
    default:
        break;
    }
    return c;
}


// re-map the existing color codes within s.
void
ansi_transform_posthdr (char *s, bool isFriend)
{
    /* Would have been easier with strtok() but can't guarantee it exists. */
    for (char *sp = s; *sp; sp++) {
        /* Find an ANSI code */
        if (*sp == 27) {
            /* transform ANSI code */
            sp += 3;

            switch (*sp) {
            case '6':
                if (isFriend)
                    *sp = color.postfriendname;
                else
                    *sp = color.postname;
                break;
            case '5':
                *sp = color.postdate;
                break;
            case '3':
                *sp = color.anonymous;
                break;
            case '2':
                if (isFriend)
                    *sp = color.postfriendtext;
                else
                    *sp = color.posttext;
                break;
            default:
                break;
            }
            lastcolor = *sp;
        }
    }
}

// note: forces lowercase
static char
get_choice_ (const char *accept)
{
    char    c;
    unsigned int invalid = 0;

    while (!strchr (accept, c = tolower (inkey ())))
        if (invalid++)
            flush_input (invalid);
    return c;
}

void
color_config (void)
{

    std_printf ("Color\r\n");
    if (!flags.useansi) {
        std_printf ("\r\nWARNING:  Color is off.  You will not be able to preview your selections.");
    }
    for (;;) {
        colorize
            ("\r\n@YG@Ceneral  @YI@Cnput  @YP@Costs  @YX@Cpress  @YO@Cptions  @YR@Ceset  @YQ@Cuit\r\n@YColor config -> @G");

        const char c = get_choice_ ("gipxorq \n");

        switch (c) {
        case 'g':
            std_printf ("General\r\n\n");
            general_color_config ();
            break;
        case 'i':
            std_printf ("Input\r\n\n");
            input_color_config ();
            break;
        case 'o':
            std_printf ("Options\r\n\n");
            color_options ();
            break;

        case 'p':
            std_printf ("Post colors\r\n\n");
            post_color_config ();
            break;

        case 'r':
            std_printf ("Reset colors\r\n");
            default_colors (true);
            break;

        case 'x':
        case 'X':
            std_printf ("Express colors\r\n\n");
            express_color_config ();
            break;

        case 'q':
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
color_options (void)
{
    std_printf ("Automatically answer the ANSI terminal question? (%s) -> ", flags.ansiprompt ? "Yes" : "No");
    flags.ansiprompt = yesnodefault (flags.ansiprompt);
    std_printf ("Use bold ANSI colors when ANSI is enabled? (%s) -> ", flags.usebold ? "Yes" : "No");
    flags.usebold = yesnodefault (flags.usebold);
    if (flags.useansi)
        printf ("\033[%cm\033[3%c;4%cm", flags.usebold ? '1' : '0', lastcolor, color.background);
}


#define A_USER		"Example User"
#define A_FRIEND	"Example Friend"
#define GEN_FMT_STR	"\033[4%c;3%cmLobby> \033[3%cmEnter message\r\n\n\033[3%cmOnly Sysops may post to the lobby\r\n\n\033[3%cmLobby> \033[3%cmGoto \033[3%cm[Babble]  \033[3%cm150\033[3%cm messages, \033[3%cm1\033[3%cm new\r\n"
#define POST_FMT_STR	"\033[3%cmJan  1, 2000 11:01\033[3%cm from \033[3%cm%s\033[3%cm\r\nHi there!\r\n\033[3%cm[Lobby> msg #1]\r\n"
#define EXPRESS_FMT_STR	"\033[3%cm*** Message (#1) from \033[3%cm%s\033[3%cm at 11:01 ***\r\n>Hi there!\r\n"
#define INPUT_FMT_STR	"\033[3%cmMessage eXpress\r\nRecipient: \033[3%cmExam\033[3%cmple User\r\n\033[3%cm>Hi there!\r\n\033[3%cmMessage received by Example User.\r\n"

static void
general_color_config (void)
{
    for (;;) {
        std_printf (GEN_FMT_STR, color.background, color.forum,
                    color.text, color.err, color.forum,
                    color.text, color.forum, color.number, color.text, color.number, color.text);

        colorize ("\r\n@YB@Cackground  @YE@Crror  @YF@Corum  @YN@Cumber  @YT@Cext  @YQ@Cuit@Y -> @G");

        const char opt = get_choice_ ("befntq \n");

        switch (opt) {
        case 'q':
        case ' ':
        case '\n':
            std_printf ("Quit\r\n");
            return;
            /* NOTREACHED */
        case 'b':
            std_printf ("Background\r\n\n");
            color.background = background_picker ();
            break;
        case 'e':
            std_printf ("Error\r\n\n");
            color.err = color_picker ();
            break;
        case 'f':
            std_printf ("Forum\r\n\n");
            color.forum = color_picker ();
            break;
        case 'n':
            std_printf ("Number\r\n\n");
            color.number = color_picker ();
            break;
        case 't':
            std_printf ("Text\r\n\n");
            color.text = color_picker ();
            break;
        default:
            break;
        }
    }
}


static void
input_color_config (void)
{
    for (;;) {
        std_printf (INPUT_FMT_STR, color.text, color.input1, color.input2, color.input1, color.text);

        colorize ("\r\n@YT@Cext  @YC@Completion  @YQ@Cuit@Y -> @G");

        const char opt = get_choice_ ("ctq \n");

        switch (opt) {
        case 'q':
        case ' ':
        case '\n':
            std_printf ("Quit\r\n");
            return;
            /* NOTREACHED */
        case 'c':
            std_printf ("Completion\r\n\n");
            color.input2 = color_picker ();
            break;
        case 't':
            std_printf ("Text\r\n\n");
            color.input1 = color_picker ();
            break;
        default:
            break;
        }
    }
}


static void
post_color_config (void)
{
    for (;;) {
        switch (user_or_friend ()) {
        case 'u':
            post_user_color_config ();
            break;
        case 'f':
            post_friend_color_config ();
            break;
        default:
            return;
        }
    }
}


static void
post_user_color_config (void)
{
    for (;;) {
        std_printf (POST_FMT_STR, color.postdate, color.posttext,
                    color.postname, A_USER, color.posttext, color.forum);
        const char c = post_color_menu ();

        switch (c) {
        case 'q':
        case ' ':
        case '\n':
            return;
            /* NOTREACHED */
        case 'd':
            color.postdate = color_picker ();
            break;
        case 'n':
            color.postname = color_picker ();
            break;
        case 't':
            color.posttext = color_picker ();
            break;
        default:
            break;
        }
    }
}


static void
post_friend_color_config (void)
{
    for (;;) {
        std_printf (POST_FMT_STR, color.postfrienddate, color.postfriendtext,
                    color.postfriendname, A_FRIEND, color.postfriendtext, color.forum);
        const char c = post_color_menu ();

        switch (c) {
        case 'q':
        case ' ':
        case '\n':
            return;
            /* NOTREACHED */
        case 'd':
            color.postfrienddate = color_picker ();
            break;
        case 'n':
            color.postfriendname = color_picker ();
            break;
        case 't':
            color.postfriendtext = color_picker ();
            break;
        default:
            break;
        }
    }
}


static char
post_color_menu (void)
{
    colorize ("\r\n@YD@Cate  @YN@Came  @YT@Cext  @YQ@Cuit@Y -> @G");

    const char c = get_choice_ ("dntq \n");

    switch (c) {
    case 'd':
        std_printf ("Date\r\n\n");
        break;
    case 'n':
        std_printf ("Name\r\n\n");
        break;
    case 't':
        std_printf ("Text\r\n\n");
        break;
    case 'q':
    case ' ':
    case '\n':
        std_printf ("Quit\r\n\n");
        break;
    default:
        break;
    }

    return c;
}


static void
express_color_config (void)
{
    for (;;) {
        switch (user_or_friend ()) {
        case 'u':
            express_user_color_config ();
            break;
        case 'f':
            express_friend_color_config ();
            break;
        default:
            return;
        }
    }
}


static void
express_user_color_config (void)
{
    for (;;) {
        std_printf (EXPRESS_FMT_STR, color.expresstext, color.expressname, A_USER, color.expresstext);
        const char c = express_color_menu ();

        switch (c) {
        case 'q':
        case ' ':
        case '\n':
            return;
            /* NOTREACHED */
        case 'n':
            color.expressname = color_picker ();
            break;
        case 't':
            color.expresstext = color_picker ();
            break;
        default:
            break;
        }
    }
}

static void
express_friend_color_config (void)
{
    for (;;) {
        std_printf (EXPRESS_FMT_STR, color.expressfriendtext,
                    color.expressfriendname, A_FRIEND, color.expressfriendtext);
        const char opt = express_color_menu ();

        switch (opt) {
        case 'q':
        case 'Q':
        case ' ':
        case '\n':
            return;
            /* NOTREACHED */
        case 'n':
        case 'N':
            color.expressfriendname = color_picker ();
            break;
        case 't':
        case 'T':
            color.expressfriendtext = color_picker ();
            break;
        default:
            break;
        }
    }
}


static char
express_color_menu (void)
{

    colorize ("\r\n@YN@Came  @YT@Cext  @YQ@Cuit@Y -> @G");

    const char c = get_choice_ ("ntq \n");

    switch (c) {
    case 'n':
        std_printf ("Name\r\n\n");
        break;
    case 't':
        std_printf ("Text\r\n\n");
        break;
    case 'q':
    case ' ':
    case '\n':
        std_printf ("Quit\r\n\n");
        break;
    default:
        break;
    }

    return c;
}


static char
user_or_friend (void)
{
    colorize ("@GConfigure for @YU@Cser @Gor @YF@Criend @Y-> @G");
    const char c = get_choice_ ("ufq \n");

    switch (c) {
    case 'u':
        std_printf ("User\r\n\n");
        break;
    case 'f':
        std_printf ("Friend\r\n\n");
        break;
    default:
        std_printf ("Quit\r\n");
        break;
    }

    return c;
}


static char
color_picker (void)
{
    colorize
        ("@CBlac@Yk  @YR@Red  @YG@Green  @WY@Yellow  @YB@Blue  @YM@Magenta  @YC@Cyan  @YW@White @Y-> @G");

    const char c = get_choice_ ("krgybmcw");

    switch (c) {
    case 'r':
        std_printf ("Red\r\n\n");
        return '1';
    case 'g':
        std_printf ("Green\r\n\n");
        return '2';
    case 'y':
        std_printf ("Yellow\r\n\n");
        return '3';
    case 'b':
        std_printf ("Blue\r\n\n");
        return '4';
    case 'm':
    case 'p':                  /* Some people call it purple */
        std_printf ("Magenta\r\n\n");
        return '5';
    case 'c':
        std_printf ("Cyan\r\n\n");
        return '6';
    case 'w':
        std_printf ("White\r\n\n");
        return '7';
    case 'k':
        std_printf ("Black\r\n\n");
        return '0';
    default:
        break;                  /* If your text goes black it's a bug here */
    }

    return '0';
}


static char
background_picker (void)
{
    string *work = new_string (160);

    str_sprintf (work,
                 "@C@kBlac@Yk @r @WR@Ced @g @WG@Yreen @y @WY@Cellow @b @YB@Ylue @m @WM@Yagenta @c @WC@Yyan @w @YW@Bhite @d @YD@Cefault \033[4%cm @Y-> @G",
                 color.background);
    colorize (str_cdata (work));
    delete_string (work);

    const char c = get_choice_ ("krgybmpcwd");
    char    x = '0';

    switch (c) {
    case 'k':
        std_printf ("Black\r\n");
        x = '0';
        break;
    case 'r':
        std_printf ("Red\r\n");
        x = '1';
        break;
    case 'g':
        std_printf ("Green\r\n");
        x = '2';
        break;
    case 'y':
        std_printf ("Yellow\r\n");
        x = '3';
        break;
    case 'b':
        std_printf ("Blue\r\n");
        x = '4';
        break;
    case 'm':
    case 'p':                  /* Some people call it purple */
        std_printf ("Magenta\r\n");
        x = '5';
        break;
    case 'c':
        std_printf ("Cyan\r\n");
        x = '6';
        break;
    case 'w':
        std_printf ("White\r\n");
        x = '7';
        break;
    case 'd':
        std_printf ("Default\r\n");
        x = '9';
        break;
    default:
        x = '0';                /* If your text goes black it's a bug here */
        break;
    }
    std_printf ("\033[4%cm\n", x);

    return x;
}
