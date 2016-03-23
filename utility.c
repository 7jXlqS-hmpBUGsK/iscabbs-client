/*
 * Various utility routines that didn't really belong elsewhere.  Yawn. 
 */
#include "defs.h"
#include "ext.h"

/* replyaway routines to reply to X's when you are away from keyboard */
/* these globals used only in this file, so let 'em stay here */
  /* Please do not change this message; it's used for reply suppression
   * (see below).  If you alter this, you will draw the ire of the ISCA
   * BBS programmers.  Trust me, I know.  :)
   */
char    replymsg[5] = "+!R ";


void
send_an_x (void)
{
    /* get the ball rolling with the bbs */
    SendingX = SX_WANT_TO;
#ifdef DEBUG
    std_printf ("send_an_x 1 SendingX is %d, xland is %d\r\n", SendingX, xland);
#endif
    net_putchar ('x');
    byte++;
    SendingX = SX_SENT_x;
#ifdef DEBUG
    std_printf ("send_an_x 2 SendingX is %d, xland is %d\r\n", SendingX, xland);
#endif
}


/* fake get_five_lines for the bbs */
void
replymessage ()
{
    int     i, k, control = 0;
    char    send;

    sendblock ();
    for (i = 0; replymsg[i]; i++)
        net_putchar (replymsg[i]);
    byte += i;
    for (i = 0; i < 5 && *awaymsg[i]; i++) {
        for (k = 0; awaymsg[i][k]; k++)
            net_putchar (awaymsg[i][k]);
        net_putchar ('\n');
        byte += k + 1;
        std_printf ("%s\r\n", awaymsg[i]);
    }
    if (i < 5) {                /* less than five lines */
        net_putchar ('\n');
        byte++;
    }
    SendingX = SX_NOT;
#ifdef DEBUG
    std_printf ("replymessage 1 SendingX is %d, xland is %d\r\n", SendingX, xland);
#endif
}


void
fatalperror (const char *error, const char *heading)
{
    fflush (stdout);
    s_perror (error, heading);
    myexit ();
}


void
fatalexit (const char *message, const char *heading)
{
    fflush (stdout);
    s_error (message, heading);
    myexit ();
}


void
myexit ()
{
    fflush (stdout);
    if (childpid) {
        /* Wait for child to terminate */
        sigoff ();
        childpid = (-childpid);
        while (childpid)
#ifndef __EMX__
            sigsuspend (0);
#else
            sleep (1);
#endif
    }
    resetterm ();
#ifdef HAVE_OPENSSL
    if (is_ssl)
        killSSL ();
#endif
    if (flags.lastsave)
        (void) freopen (tempfilename, "w+", tempfile);
    deinitialize ();
    exit (0);
}


void
looper ()
{
    register int c;
    unsigned int invalid = 0;

    for (;;) {
        if ((c = inkey ()) < 0)
            return;
        /* Don't bother sending stuff to the bbs it won't use anyway */
        if ((c >= 32 && c <= 127) || mystrchr ("\3\4\5\b\n\r\27\30\32", c)) {
            invalid = 0;
            net_putchar (keymap[c]);
            if (byte)
                save[byte++ % sizeof save] = c;
        }
        else if (invalid++)
            flush_input (invalid);
    }
}


int
yesno ()
{
    register int c;
    unsigned int invalid = 0;

    while (!mystrchr ("nNyY", c = inkey ()))
        if (invalid++)
            flush_input (invalid);
    if (c == 'y' || c == 'Y') {
        std_printf ("Yes\r\n");
        return (1);
    }
    else {
        std_printf ("No\r\n");
        return (0);
    }
}

int
yesnodefault (def)
     int     def;
{
    register int c;
    unsigned int invalid = 0;

    while (!mystrchr ("nNyY\n ", c = inkey ()))
        if (invalid++)
            flush_input (invalid);
    if (c == '\n' || c == ' ')
        c = (def ? 'Y' : 'N');
    if (c == 'y' || c == 'Y') {
        std_printf ("Yes\r\n");
        return (1);
    }
    else if (c == 'n' || c == 'N') {
        std_printf ("No\r\n");
        return (0);
    }
    else {                      /* This should never happen, means bug in mystrchr() */
        char    buf[160];

        std_printf ("\r\n");
        sprintf (buf, "yesnodefault: 0x%x\r\n" "Please report this to IO ERROR\r\n", c);
        fatalexit (buf, "Internal error");
    }
}


void
tempfileerror ()
{
    if (errno == EINTR)
        return;
    fprintf (stderr, "\r\n");
    s_perror ("writing tempfile", "Local error");
}



int
more (line, pct)
     int    *line;
     int     pct;
{
    register int c;
    unsigned int invalid = 0;

    if (pct >= 0)
        printf ("--MORE--(%d%%)", pct);
    else
        printf ("--MORE--");
    for (;;) {
        c = inkey ();
        if (c == ' ' || c == 'y' || c == 'Y')
            *line = 1;
        else if (c == '\n')
            -- * line;
        else if (mystrchr ("nNqsS", c))
            *line = -1;
        else if (invalid++) {
            flush_input (invalid);
            continue;
        }
        printf ("\r              \r");
        break;
    }
    return (*line < 0 ? -1 : 0);
}


/*
 * Not all systems have strstr(), so I roll my own...
 */
char   *
mystrstr (str, substr)
     const char *str;
     const char *substr;
{
    register char *s;

    for (s = (char *) str; *s; s++)
        if (*s == *substr && !strncmp (s, substr, strlen (substr)))
            break;
    if (!*s)
        return ((char *) NULL);
    else
        return (s);
}



/*
 * Not all systems have strchr() either (they usually have index() instead, but
 * I don't want to count on that or check for it)
 */
char   *
mystrchr (str, ch)
     const char *str;
     int     ch;
{
    register char *s;

    s = (char *) str;
    while (*s && ch != *s)
        s++;
    if (*s)
        return (s);
    else
        return ((char *) NULL);
}


/* ExtractName -- get the username out of a post or X message header */
/* returns pointer to username as stored in the array */
char   *
ExtractName (header)
     char   *header;
{
    char   *hp, *ours;
    int     lastspace, i, which = -1;

    hp = mystrstr (header, " from ");
    if (!hp)                    /* This isn't an X message or a post */
        return NULL;
    hp += 6;
    if (*hp == '\033')
        hp += 5;
    /* Now should be pointing to the user name */
    lastspace = 1;
    ours = mystrdup (hp);
    for (i = 0; i < strlen (ours); i++) {
        if (ours[i] == '\033')
            break;
        if (lastspace && !isupper (ours[i]))
            break;
        if (ours[i] == ' ')
            lastspace = 1;
        else
            lastspace = 0;
    }
    ours[i] = '\0';
    i--;
    /* \r courtesy of Sbum, fixed enemy list in non-ANSI mode 2/9/2000 */
    if (ours[i] == ' ' || ours[i] == '\r')
        ours[i] = '\0';
    /* Is the name empty? */
    if (*ours == 0)
        return NULL;
    /* check for dupes first */
    for (i = 0; i < MAXLAST; i++)
        if (!strcmp (lastname[i], ours))
            which = i;
    /* insert the name */
    if (which != 0) {
        for (i = (which > 0) ? which - 1 : MAXLAST - 2; i >= 0; --i)
            strcpy (lastname[i + 1], lastname[i]);
        strcpy (lastname[0], ours);
    }
    free (ours);
    return (char *) lastname[0];
}


/*
 * ExtractNumber - extract the X message number from an X message header.
 */
int
ExtractNumber (header)
     char   *header;
{
    char   *p;
    int     number = 0;

    p = mystrstr (header, "(#");
    if (!p)                     /* This isn't an X message */
        return 0;

    for (p += 2; *p != ')'; p++)
        number += number * 10 + (*p - '0');

    return number;
}


char   *
mystrdup (s)
     const char *s;
{
    int     i;
    char   *p;

    i = strlen (s) + 2;
    p = (char *) calloc (1, (unsigned) i);
    if (p)
        strcpy (p, s);
    return p;
}

#define ifansi	if (flags.useansi)

int
colorize (str)
     const char *str;
{
    char   *p;

    for (p = (char *) str; *p; p++)
        if (*p == '@')
            if (!*(p + 1))
                p--;
            else
                switch (*++p) {
                case '@':
                    putchar ((int) '@');
                    break;
                case 'k':
                    ifansi printf ("\033[40m");
                    break;
                case 'K':
                    ifansi printf ("\033[30m");
                    break;
                case 'r':
                    ifansi printf ("\033[41m");
                    break;
                case 'R':
                    ifansi printf ("\033[31m");
                    break;
                case 'g':
                    ifansi printf ("\033[42m");
                    break;
                case 'G':
                    ifansi printf ("\033[32m");
                    break;
                case 'y':
                    ifansi printf ("\033[43m");
                    break;
                case 'Y':
                    ifansi printf ("\033[33m");
                    break;
                case 'b':
                    ifansi printf ("\033[44m");
                    break;
                case 'B':
                    ifansi printf ("\033[34m");
                    break;
                case 'm':
                case 'p':
                    ifansi printf ("\033[45m");
                    break;
                case 'M':
                case 'P':
                    ifansi printf ("\033[35m");
                    break;
                case 'c':
                    ifansi printf ("\033[46m");
                    break;
                case 'C':
                    ifansi printf ("\033[36m");
                    break;
                case 'w':
                    ifansi printf ("\033[47m");
                    break;
                case 'W':
                    ifansi printf ("\033[37m");
                    break;
                case 'd':
                    ifansi printf ("\033[49m");
                    break;
                case 'D':
                    ifansi printf ("\033[39m");
                    break;
                default:
                    break;
                }
        else
            std_putchar ((int) *p);
    return 1;
}


/*
 * Process command line arguments.  argv[1] is an alternate host, if present,
 * and argv[2] is an alternate port, if present, and argv[1] is also present.
 */
void
arguments (argc, argv)
     int     argc;
     char  **argv;
{
    if (argc > 1) {
        strcpy (cmdlinehost, argv[1]);
    }
    else {
        *cmdlinehost = 0;
    }
    if (argc > 2) {
        cmdlineport = atoi (argv[2]);
    }
    else {
        cmdlineport = 0;
    }
    if (argc > 3) {
        if (!strncmp (argv[3], "secure", 6) || !strncmp (argv[3], "ssl", 6)) {
            want_ssl = 1;
        }
        else {
            want_ssl = 0;
        }
    }
}


/*
 * strcmp() wrapper for friend entries; grabs the correct entry from the
 * struct, which is arg 2.
 */
int
fstrcmp (a, b)
     char   *a;
     friend *b;
{
    return strcmp (a, b->name);
}



/* 
 * strcmp() wrapper for char entries. 
 */
int
sortcmp (a, b)
     char  **a, **b;
{
    return strcmp (*a, *b);
}


/*
 * strcmp() wrapper for friend entries; takes two friend * args.
 */
int
fsortcmp (a, b)
     friend **a, **b;
{
    assert ((*a)->magic == 0x3231);
    assert ((*b)->magic == 0x3231);

    return strcmp ((*a)->name, (*b)->name);
}

#ifdef ENABLE_SAVE_PASSWORD
/* 
 * Encode/decode password with a simple algorithm.
 * jhp 5Feb95 (Marx Marvelous)
 *
 * This code is horribly insecure.  Don't use it for any passwords
 * you care about!  Also note it's closely tied to ASCII and won't
 * work with a non-ASCII system.  - IO
 */
char   *
jhpencode (char *dest, const char *src, size_t len)
{
    char   *di;                 /* dest iterator */
    char    x;                  /* a single character */

    di = dest;
    while ((x = *src++) != 0) {
        *di++ = (x - 32 - len + 95) % 95 + 32;
        len = x - 32;
    }
    *di = 0;
    return dest;
}

char   *
jhpdecode (char *dest, const char *src, size_t len)
{
    char   *di;                 /* dest iterator */
    char    x;                  /* a single character */

    di = dest;
    while ((x = *src++) != 0) {
        *di++ = (len = (len + x - 32) % 95) + 32;
    }
    *di = 0;
    return dest;
}
#endif
