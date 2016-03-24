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
    int     k;
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
        for (k = 0; send_string[j][k]; k++)
            net_putchar (send_string[j][k]);
        net_putchar ('\n');
        byte += k + 1;
    }
    if (flags.useansi)
        std_printf ("\033[3%cm", lastcolor = color.text);   /* assignment */
}



/*
 * Find a unique matching name to the input entered so far by the user.
 */
int
smartname (char *buf, char *pe)
{
    int     i, found = -1;
    Friend *pf = NULL, *pg;
    char    hold = *pe;
    slist  *listToUse;

    *pe = 0;
    listToUse = whoList;
    for (i = 0; i < listToUse->nitems; i++) {
        pf = listToUse->items[i];
        if (!strncmp ((const char *) pf, buf, strlen (buf))) {  /* Partial match? */
            /* Partial match unique? */
            if (i + 1 >= listToUse->nitems) {
                found = i;
                break;
            }
            else {
                pg = listToUse->items[i + 1];
                if (strncmp ((const char *) pg, buf, strlen (buf))) {
                    found = i;
                    break;
                }
                else
                    break;
            }
        }
    }
    if (found == -1) {
        *pe = hold;
        return 0;
    }
    else
        strcpy (buf, (const char *) pf);
    return 1;
}


void
smartprint (char *buf, char *pe)
{
    char   *pc = pe;

    for (; pc > buf; pc--)
        putchar ('\b');
    if (flags.useansi)
        std_printf ("\033[3%cm", color.input1);
    for (; *pc != 0; pc++) {
        if (pc == pe && flags.useansi)
            std_printf ("\033[3%cm", color.input2);
        putchar (*pc);
    }
    for (; pc != pe; pc--)
        putchar ('\b');
    if (flags.useansi)
        std_printf ("\033[3%cm", color.input1);
}


void
smarterase (char *pe)
{
    char   *pc = pe;

    for (; *pc != 0; pc++)
        putchar (' ');
    for (; pc != pe; pc--)
        putchar ('\b');
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
    static char pbuf[MAXNAME + 1];
    int     c;
    int     smart = 0;
    int     upflag;
    int     fflag;
    unsigned int invalid = 0;
    static char junk[21];

#if DEBUG
    std_printf (" %d SX = %d} ", quit_priv, SendingX);
#endif
    lastptr = 0;
    if (flags.useansi)
        std_printf ("\033[3%cm", color.input1);
    if (quit_priv == 1 && *autoname && strcmp (autoname, "NONE") && !autologgedin) {
        autologgedin = 1;
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
        return (junk);
    }
    SendingX = SX_NOT;
#ifdef DEBUG
    std_printf ("get_name 2 SendingX is %d, xland is %d\r\n", SendingX, xland);
#endif
    for (;;) {
        upflag = fflag = 1;
        p = pbuf;
        for (;;) {
            c = inkey ();
            if (c == '\n')
                break;
            if (c == CTRL_D && quit_priv == 1) {
                pbuf[0] = CTRL_D;
                pbuf[1] = 0;
                return (pbuf);
            }
            if (c == '_')
                c = ' ';
            if (c == 14 /* CTRL_N */ ) {
                if (smart) {
                    smarterase (p);
                    smart = 0;
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
                    smart = 0;
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
                    if (smart == 1) {
                        smarterase (pbuf);
                        smart = 0;
                    }
                    upflag = (p == pbuf || *(p - 1) == ' ');
                    if (upflag && c == CTRL_W)
                        break;
                    if (p == pbuf)
                        fflag = 1;
                }
                else if (p < &pbuf[!quit_priv || quit_priv == 3 ? MAXNAME : MAXALIAS]
                         && (isalpha (c) || c == ' ' || (isdigit (c) && quit_priv == 3))) {
                    fflag = 0;
                    if (upflag && isupper (c))
                        --upflag;
                    if (upflag && islower (c)) {
                        c -= 32;
                        --upflag;
                    }
                    if (c == ' ')
                        upflag = 1;
                    *p++ = c;
                    putchar (c);
                    if (quit_priv == 2 || quit_priv == -999) {
                        if (smartname (pbuf, p)) {
                            smartprint (pbuf, p);
                            smart = 1;
                        }
                        else if (smart == 1) {
                            smarterase (p);
                            smart = 0;
                        }
                    }
                }
            while ((c == CTRL_X || c == CTRL_W) && p > pbuf) ;
        }
        if (smart == 0)
            *p = 0;
        else {
            if (flags.useansi)
                std_printf ("\033[3%cm", color.input1);
            for (; *p != 0; p++)
                putchar (*p);
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
        writebbsrc ();
    }
    return (pbuf);
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
    int     c;
    int     hidden;
    unsigned int invalid = 0;


    if (line <= 0)
        *wrap = 0;
    else if (*wrap) {
        printf ("%s", wrap);
        strcpy (result, wrap);
        p = result + strlen (wrap);
        *wrap = 0;
    }
    hidden = 0;
    if (length < 0)
        hidden = length = 0 - length;
    /* Kludge here, since some C compilers too stupid to understand 'signed' */
    if (length > 128)
        hidden = length = 256 - length;
#ifdef ENABLE_SAVE_PASSWORD
    if (hidden != 0 && *autopasswd) {
        if (!autopasswdsent) {
            jhpdecode (result, autopasswd, strlen (autopasswd));
            for (c = 0; c < strlen (result); c++)
                std_putchar ('.');
            std_printf ("\r\n");
            autopasswdsent = 1;
            return;
        }
    }
#endif
    for (;;) {
        c = inkey ();
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
            for (c = q == p; p > result && (!c || p[-1] != ' '); p--) {
                if (p[-1] != ' ')
                    c = 1;
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
        for (c = 0; c < strlen (result); c++)
            cap_putchar ('.');
#ifdef ENABLE_SAVE_PASSWORD
    if (hidden != 0) {
        jhpencode (autopasswd, result, strlen (result));
        writebbsrc ();
    }
#endif
    std_printf ("\r\n");
}
