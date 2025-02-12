/*
 * This handles getting of short strings or groups of strings of information
 * and sending them off to the BBS.  Even though you might think doing this for
 * an 8 character password is a waste, it does cut down network traffic (and
 * the lag to the end user) to not have to echo back each character
 * individually, and it was easier to do them all rather than only some of
 * them.  Again, this is basically code from the actual BBS with slight
 * modification to work here. 
 */
#include "defs.h"
#include "ext.h"

#define MAXALIAS	19
#define MAXNAME		40


/*
 * Used for getting X's and profiles.  'which' tells which of those two we are
 * wanting, to allow the special commands for X's, like PING and ABORT. When
 * we've got what we need, we send it immediately over the net. 
 */
void
get_five_lines (int which)
{
    int     i;
    int     j;
    char    send_string[21][80];
    int     override = 0;
    int     local = 0;

#if DEBUG
    std_printf (" %d SX == %d}\r\n", which, SendingX);
#endif
    if (away && SendingX == SX_SENT_NAME) {
        SendingX = SX_REPLYING;
#ifdef DEBUG
        std_printf ("get_five_lines 1 SendingX is %d, xland is %d\r\n", SendingX, xland);
#endif
        replymessage ();
        return;
    }
    if (xland)
        SendingX = SX_SEND_NEXT;
    else
        SendingX = SX_NOT;
#ifdef DEBUG
    std_printf ("get_five_lines 2 SendingX is %d, xland is %d\r\n", SendingX, xland);
#endif
    if (flags.useansi)
        std_printf ("\033[3%cm", color.input1);
    for (i = 0; i < (20 + override + local) && (!i || *send_string[i - 1]); i++) {
        std_printf (">");
        get_string (78, send_string[i], i);
        if ((which && !strcmp (send_string[i], "ABORT"))
            || (!(which & 16) && !i && !strcmp (*send_string, "PING"))) {
            i++;
            break;
        }
        else if ((which & 2) && !(which & 8) && !strcmp (send_string[i], "OVERRIDE")) {
            std_printf ("Override on.\r\n");
            override++;
        }
        else if ((which & 4) && !i && !strcmp (*send_string, "BEEPS")) {
            i++;
            break;
        }
        else if ((which & 8) && !strcmp (send_string[i], "LOCAL")) {
            std_printf ("Only broadcasting to local users.\r\n");
            local++;
        }
    }
    sendblock ();
    if (!strcmp (*send_string, "PING"))
        strcpy ((char *) send_string[0], "\0"); /* please let this go away soon */
    for (j = 0; j < i; j++) {
        net_putstring (send_string[j]);
        net_putchar ('\n');
    }
    if (flags.useansi)
        std_printf ("\033[3%cm", lastcolor = color.text);   /* assignment */
}


/*
 * Find a unique matching name to the input entered so far by the user.
 */
static  bool
smartname (char *buf, size_t prefix_len)
{
    UserEntry *pf = NULL;

    // search whoList
    for (size_t i = 0; i < whoList.sz; ++i) {
        UserEntry *cur = whoList.arr[i];

        if (strncmp (cur->name, buf, prefix_len) == 0) {    /* Partial match */
            /* have we already seen a match? */
            if (pf != NULL)
                return false;
            pf = cur;
        }
    }

    // search friendList
    for (size_t i = 0; i < friendList.sz; ++i) {
        UserEntry *cur = friendList.arr[i];

        if (strncmp (cur->name, buf, prefix_len) == 0) {    /* Partial match */
            /* have we already seen a DIFFERENT match? */
            if (pf != NULL && strcmp (pf->name, cur->name))
                return false;
            pf = cur;
        }
    }

    if (!pf)                    // none found.
        return false;

    strcpy (buf, pf->name);

    return true;
}


// (pe-buf) is the matched prefix length so far.
static void
smartprint (const char *buf, const char *const pe)
{
    // We logically divide the buffer into to substrings at pe.
    const size_t len1 = pe - buf;
    const size_t len2 = strlen (pe);

    // backup over the first part and write it in color.
    putnchars ('\b', len1);
    if (flags.useansi)
        std_printf ("\033[3%cm", color.input1);
    fwrite (buf, 1, len1, stdout);

    // Write the remainder.
    if (flags.useansi)
        std_printf ("\033[3%cm", color.input2);
    fwrite (pe, 1, len2, stdout);

    // Now backup to the middle where we started and restore the color.
    putnchars ('\b', len2);
    if (flags.useansi)
        std_printf ("\033[3%cm", color.input1);
}


static void
smarterase (const char *pe)
{
    const size_t n = strlen (pe);

    putnchars (' ', n);
    putnchars ('\b', n);
}


/*
 * Used for getting names (user names, room names, etc.)  Capitalizes first
 * letter of word automatically)  Does different things depending on the value
 * of quit_priv (that stuff should be left alone)  The name is then returned to
 * the caller. 
 */
char   *
get_name (int quit_priv)
{
    char   *p;
    static char pbuf[MAXNAME + 1];  // Note: static buffer is returned to caller.
    bool    smart = false;
    bool    upflag;             // uppercase the next lowercase char
    bool    fflag;
    unsigned int invalid = 0;
    static char junk[21];       // Note: static buffer is returned to caller.

#if DEBUG
    std_printf (" %d SX = %d} ", quit_priv, SendingX);
#endif
    int     lastptr = 0;

    if (flags.useansi)
        std_printf ("\033[3%cm", color.input1);
    if (quit_priv == 1 && *autoname && strcmp (autoname, "NONE") && !autologgedin) {
        autologgedin = true;
        strcpy (junk, autoname);
        std_printf ("%s\r\n", junk);
        return junk;
    }
    if ((away || xland) && quit_priv == 2 && SendingX == SX_SENT_x) {
        SendingX = SX_SENT_NAME;
#ifdef DEBUG
        std_printf ("get_name 1 SendingX is %d, xland is %d\r\n", SendingX, xland);
#endif
        if (!pop_queue (junk, xlandQueue))
            std_printf ("ACK!  It didn't pop.\r\n");
        if (flags.useansi)
            std_printf ("\033[3%cm", lastcolor);
        std_printf ("\rAutomatic reply to %s                     \r\n", junk);
        return junk;
    }
    SendingX = SX_NOT;
#ifdef DEBUG
    std_printf ("get_name 2 SendingX is %d, xland is %d\r\n", SendingX, xland);
#endif
    for (;;) {
        upflag = fflag = true;
        p = pbuf;
        for (;;) {
            int     c = inkey ();

            if (c == '\n')
                break;
            if (c == CTRL_D && quit_priv == 1) {
                pbuf[0] = CTRL_D;
                pbuf[1] = 0;
                return pbuf;
            }
            if (c == '_')
                c = ' ';
            if (c == 14 /* CTRL_N */ ) {
                if (smart) {
                    smarterase (p);
                    smart = false;
                }
                for (; p > pbuf; --p)
                    printf ("\b \b");
                printf ("%s", lastname[lastptr]);
                strcpy (pbuf, lastname[lastptr]);
                if (++lastptr == 20 || lastname[lastptr][0] == 0)
                    lastptr = 0;
                for (p = pbuf; *p != '\0'; p++) ;
                continue;
            }
            if (c == 16 /* CTRL_P */ ) {
                if (smart) {
                    smarterase (p);
                    smart = false;
                }
                for (; p > pbuf; --p)
                    printf ("\b \b");
                if (--lastptr < 0)
                    for (lastptr = 19; lastptr > 0; --lastptr)
                        if (lastname[lastptr][0] != 0)
                            break;
                if (--lastptr < 0)
                    for (lastptr = 19; lastptr > 0; --lastptr)
                        if (lastname[lastptr][0] != 0)
                            break;
                printf ("%s", lastname[lastptr]);
                strcpy (pbuf, lastname[lastptr]);
                if (++lastptr == 20 || lastname[lastptr][0] == 0)
                    lastptr = 0;
                for (p = pbuf; *p != 0; p++) ;
                continue;
            }
            if (c == ' ' && (fflag || upflag))
                continue;
            if (c == '\b' || c == CTRL_X || c == CTRL_W || c == CTRL_R || c == ' ' || isalpha (c)
                || (isdigit (c) && quit_priv == 3))
                invalid = 0;
            else {
                if (invalid++)
                    flush_input (invalid);
                continue;
            }
            if (c == CTRL_R) {
                *p = 0;
                printf ("\r\n%s", pbuf);
                continue;
            }
            do
                if ((c == '\b' || c == CTRL_X || c == CTRL_W) && p > pbuf) {
                    printf ("\b \b");
                    --p;
                    if (smart) {
                        smarterase (pbuf);
                        smart = false;
                    }
                    upflag = (p == pbuf || *(p - 1) == ' ');
                    if (upflag && c == CTRL_W)
                        break;
                    if (p == pbuf)
                        fflag = true;
                }
                else if (p < &pbuf[!quit_priv || quit_priv == 3 ? MAXNAME : MAXALIAS]
                         && (isalpha (c) || c == ' ' || (isdigit (c) && quit_priv == 3))) {
                    fflag = false;
                    if (upflag && isupper (c))
                        upflag = false;
                    if (upflag && islower (c)) {
                        c = toupper (c);
                        upflag = false;
                    }
                    if (c == ' ')
                        upflag = true;
                    *p++ = c;
                    putchar (c);
                    if (quit_priv == 2 || quit_priv == -999) {
                        if (smartname (pbuf, p - pbuf)) {
                            smartprint (pbuf, p);
                            smart = true;
                        }
                        else if (smart) {
                            smarterase (p);
                            smart = false;
                        }
                    }
                }
            while ((c == CTRL_X || c == CTRL_W) && p > pbuf) ;
        }
        if (!smart)
            *p = 0;
        else {
            if (flags.useansi)
                std_printf ("\033[3%cm", color.input1);
            fputs (p, stdout);
            p += strlen (p);
        }
        break;
    }
    cap_puts (pbuf);
    if (p > pbuf || quit_priv >= 2)
        std_printf ("\r\n");

    if (p > pbuf && p[-1] == ' ')
        p[-1] = 0;

    if (quit_priv == 1 && strcmp (pbuf, "Guest") && strcmp (autoname, "NONE")) {
        strcpy (autoname, pbuf);
        writebbsrc (bbsrc);
    }
    return pbuf;
}


/*
 * Gets a generic string of length length, puts it in result and returns a a
 * pointer to result.  If the length given is negative, the string is echoed
 * with '.' instead of the character typed (used for passwords) 
 */
void
get_string (int length, char *result, int line)
{
    static char wrap[80];
    char   *rest;
    char   *p = result;
    char   *q;
    unsigned int invalid = 0;


    if (line <= 0)
        *wrap = 0;
    else if (*wrap) {
        printf ("%s", wrap);
        strcpy (result, wrap);
        p = result + strlen (wrap);
        *wrap = 0;
    }
    const bool hidden = (length < 0);

    length = abs (length);

    ///* Kludge here, since some C compilers too stupid to understand 'signed' */
    //if (length > 128)
    //    hidden = length = 256 - length;

    if (hidden && *autopasswd) {
        if (!autopasswdsent) {
            jhpdecode (result, autopasswd, strlen (autopasswd));
            for (size_t c = 0, E = strlen (result); c != E; c++)
                std_putchar ('.');
            std_printf ("\r\n");
            autopasswdsent = true;
            return;
        }
    }
    for (;;) {
        int     c = inkey ();

        if (c == ' ' && length == 29 && p == result)
            break;
        if (c == '\n')
            break;
        if (c < ' ' && c != '\b' && c != CTRL_X && c != CTRL_W && c != CTRL_R) {
            if (invalid++)
                flush_input (invalid);
            continue;
        }
        else
            invalid = 0;
        if (c == CTRL_R) {
            *p = 0;
            if (!hidden)
                printf ("\r\n%s", result);
            continue;
        }
        if (c == '\b' || c == CTRL_X) {
            if (p == result)
                continue;
            else
                do {
                    printf ("\b \b");
                    --p;
                }
                while (c == CTRL_X && p > result);
        }
        else if (c == CTRL_W) {
            for (q = result; q < p; q++)
                if (*q != ' ')
                    break;
            // TODO: ugly reuse of c.  
            for (c = q == p; p > result && (!c || p[-1] != ' '); p--) {
                if (p[-1] != ' ')
                    c = 1;      // TODO: this is weird, and is repeated in the loop condition.
                printf ("\b \b");
            }
        }
        else if (p < result + length && isprint (c)) {
            *p++ = c;
            if (!hidden)
                putchar (c);
            else
                putchar ('.');
        }
        else if (line < 0 || line == 4)
            continue;
        else {
            if (c == ' ')
                break;
            for (q = p - 1; *q != ' ' && q > result; q--) ;
            if (q > result) {
                *q = 0;
                for (rest = wrap, q++; q < p; printf ("\b \b"))
                    *rest++ = *q++;
                *rest++ = c;
                *rest = 0;
            }
            else {
                *wrap = c;
                *(wrap + 1) = 0;
            }
            break;
        }
    }
    *p = 0;
    if (!hidden)
        cap_puts (result);
    else
        // emits a sequence of N '.'
        for (size_t c = 0, E = strlen (result); c != E; c++)
            cap_putchar ('.');
    if (hidden != 0) {
        jhpencode (autopasswd, result, strlen (result));
        writebbsrc (bbsrc);
    }
    std_printf ("\r\n");
}
