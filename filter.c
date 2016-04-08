/** @copyright GNU Public License. See COPYING. */
#include "defs.h"
#include "ext.h"
#include "telnet.h"

static void continued_data_helper (void);
static bool is_automatic_reply (const char *message);
static void filter_url (char *line);
static void not_replying_transform_express (char *s);
static void replycode_transform_express (char *s);

static char thisline[320];      /* Copy of the current line */

static void
parse_wholist_ (string * buf)
{
    time_t  now = time (NULL);

    // each iteration parses one who list entry.
    // note we shave 1 of the \0 bytes from the end.
    for (char *i = str_begin (buf), *E = str_end (buf) - 1; i != E;) {

        // Parse the time indicator. There are two formats: A single byte, or,
        // 0xFE followed by a BCD string, where each digit is offset by 1, followed by a \0 terminator.
        unsigned long t = (unsigned char) *i++;

        if (t == 0xFE) {
            for (t = 0; *i; ++i) {
                assert (*i >= 1 && *i <= 11);
                t = t * 10 + *i - 1;
            }
            assert (*i == '\0');
            ++i;

            // NOTE: I'm seeing the first character of the name duplicated,
            // but only after the extended time format.  Like this,
            // [0xFE,4,4,4,0,'J','J','o','h','n',0]
            // I don't know what it means. Let's just detect it and accept it.
            if (isupper (i[0] & 0x7f) && i[0] == i[1])
                ++i;            // skip the duplicate char.
        }

        // i now points to a \0-terminated username.
        // if the first char has the high bit set, then xmsg are disabled.
        assert (isupper ((*i & 0x7f)));
        char   *name = i;
        const bool xmsg_disabled = ((name[0] & 0x80) == 0x80);

        name[0] &= 0x7f;        // clear high bit.

        UserEntry *u = ulist_insert (&whoList, name);

        u->login_tm = now - t * 60;
        u->xmsg_disabled = xmsg_disabled;

        // move to next entry
        i += strlen (i) + 1;
    }
}

static void
print_friends_online_ (bool was_updated)
{
    ulist_sort_by_time (&whoList);
    const time_t now = time (NULL);
    string *buf = new_string (100);

    // Pass one:  count them.
    size_t  tot = 0;

    for (size_t i = 0; i != whoList.sz; ++i)
        if (ulist_find (&friendList, whoList.arr[i]->name))
            ++tot;

    // Print the header.
    std_printf ("\r\n%zd friends online (%s)\r\n\n", tot, was_updated ? "updated" : "cached");

    // Pass two: print them.
    for (size_t i = 0; i != whoList.sz; ++i) {
        const UserEntry *w = whoList.arr[i];
        const UserEntry *f = ulist_find (&friendList, w->name);

        if (f) {
            // Extract into days, hours, minutes, and seconds.
            const long t = now - w->login_tm;
            const int days = t / (24 * 60 * 60);
            const int hours = (t % (24 * 60 * 60)) / (60 * 60);
            const int mins = (t % (60 * 60)) / 60;

            // Be sure to use the whoList entry for the login_tm and xmsg_disabled,
            // and the friendList entry for the info. Bleh.

            char    day_buf[20];

            day_buf[0] = 0;
            if (days)
                sprintf (day_buf, "%d %s ", days, (days == 1 ? "day" : "days"));

            str_sprintf (buf, flags.useansi ?
                         "@Y%-19s%c @R%6s%02d:%02d@G  @C%s\r\n" :
                         "%-19s%c %6s%02d:%02d %s\r\n",
                         w->name, (w->xmsg_disabled ? '*' : ' '), day_buf, hours, mins, f->info);
            colorize (str_cdata (buf));
        }
    }
    delete_string (buf);
}

/* The bbs is sending a who-list, and it will continue to call this function while
 * the global variable recving_wholist is set.
 */
void
filter_wholist (int c)
{
    static string *buf = NULL;

    if (!buf)
        buf = new_string (512);

    // buffer until we hit \0 (two nulls in a row), marking the end of the who list.
    str_pushc (buf, c);
    if (str_length (buf) >= 2 && str_cend (buf)[-1] == '\0' && str_cend (buf)[-2] == '\0') {
        // We have the complete list buffered.

        // The bbs sends an empty list as a way to signal no-change.
        if (str_length (buf) == 2) {
            // no change in wholist.
            print_friends_online_ (false);
        }
        else {
            // discard the old list.
            ulist_clear (&whoList);
            parse_wholist_ (buf);
            print_friends_online_ (true);
        }


        // finish up.
        recving_wholist = false;
        delete_string (buf);
        buf = NULL;
    }
}


void
filter_express (int c)
{
    static char *bp;            /* comparison pointer */
    static struct {
        int     crlf:1;         /* Needs initial CR/LF */
        int     prochdr:1;      /* Needs killfile processing */
        int     ignore:1;       /* Ignore the remainder of the X */
    } needs;
    char   *thisline = xmsgbufp;    /* Pointer to the current line */

    if (c == -1) {              /* signal from IAC to begin/end X */
        if (xmsgnow) {          /* Need to re-init for new X */
            xmsgbufp = xmsgbuf;
            *xmsgbuf = 0;
            needs.ignore = 0;
            needs.crlf = 0;
            needs.prochdr = 1;
            return;
        }
        else if (!needs.ignore) {   /* Finished this X, dump it out */
            /* Process for automatic reply */
            int     i = ExtractNumber (xmsgbuf);

            /* Don't queue if it's an outgoing X */
            /* Only send 'x' if it's a new incoming X */
            if ((away || xland) && !is_queued (bp, xlandQueue) &&
                i > highxmsg && !is_automatic_reply (xmsgbuf) && bp) {
                push_queue (bp, xlandQueue);
                needx = true;
            }
            else if (is_automatic_reply (xmsgbuf) && i > highxmsg) {
                not_replying_transform_express (xmsgbuf);
            }
            /* Update highxmsg with greatest number seen */
            if (i > highxmsg)
                highxmsg = i;
            replycode_transform_express (xmsgbuf);
            ansi_transform_express (xmsgbuf);
            std_printf ("%s%s", (needs.crlf) ? "\r\n" : "", xmsgbuf);
            return;
        }
    }
    /* If the message is killed, don't bother doing anything. */
    if (needs.ignore)
        return;

    if (xmsgbuf == xmsgbufp) {  /* Check for initial CR/LF pair */
        if (c == '\r' || c == '\n') {
            needs.crlf = 1;
            return;
        }
    }
    /* Insert character into the buffer */
    *xmsgbufp++ = c;
    *xmsgbufp = 0;

    /* Extract URLs if any */
    if (!needs.prochdr && c == '\r') {
        filter_url (thisline);
        thisline = xmsgbufp;
    }

    /* If reached a \r it's time to do header processing */
    if (needs.prochdr && c == '\r') {
        needs.prochdr = 0;
        thisline = xmsgbufp;

        /* Process for kill file */
        //sp = strstr (xmsgbuf, " from ");
        //if (!strstr (xmsgbuf, "*** Message ") && !strstr (xmsgbuf, "%%% Question "))
        //    sp = NULL;
        /* get name for ^N function */
        bp = ExtractName (xmsgbuf);
        /* FIXME: move this down to where the msg is printed! so that enemies are
           not added to the ^N scroll */
        if (ulist_find (&enemyList, bp)) {
            if (!flags.squelchexpress)
                std_printf ("\r\n[X message by %s killed]\r\n", bp);
            needs.ignore = 1;
            return;
        }
    }
    return;
}

void
filter_post (int c)
{
    static int ansistate = 0;   /* ANSI state count */
    static int numposts = 0;    /* count of the # of posts received so far */
    static char posthdr[140];   /* store the post header here */
    static char *posthdrp;      /* pointer into posthdr */
    static struct {
        int     crlf:1;         /* need to add a CR/LF */
        int     prochdr:1;      /* process post header */
        int     ignore:1;       /* kill this post */
        int     second_n:1;     /* send a second n */
    } needs;
    static char *bp;            /* misc. pointer */
    static char junk[160];
    static bool isFriend;       /* Current post is by a friend */

    if (c == -1) {              /* control: begin/end of post */
        if (postnow) {          /* beginning of post */
            posthdrp = posthdr;
            *posthdr = 0;
            needs.crlf = 0;
            needs.prochdr = 1;
            isFriend = false;
            needs.ignore = 0;
            needs.second_n = 0;
        }
        else {                  /* end of post */
            if (needs.second_n)
                net_putchar ('n');
            filter_url (" ");
            numposts++;
        }
        return;
    }
    /* If post is killed, do no further processing. */
    if (needs.ignore)           /* ignore our stupid enemies :) */
        return;

    if (posthdr == posthdrp) {  /* Check for initial CR/LF pair */
        if (c == '\r' || c == '\n') {
            needs.crlf = 1;
            return;
        }
    }
    /* At this point we should either insert the character into the post
     * buffer, or echo it to the screen.
     */
    if (!needs.prochdr) {
        /* Store this line for processing */
        if (c == '\n') {
            filter_url (thisline);
            thisline[0] = 0;
        }
        else {
            /* This is a whole lot faster than calling strcat() */
            char   *p = thisline;

            for (; *p; p++)     /* Find end of string */
                ;

            *p++ = c;           /* Copy character to end of string */
            *p = 0;
        }

        /* Process ANSI codes in the middle of a post */
        if (ansistate) {
            ansistate--;
            if (ansistate == 1) {
                if (flags.offbold && c == 109) {    /* Turn boldface off */
                    printf ("m\033[0");
                    ansistate--;
                }
                else {
                    lastcolor = ansi_transform_post ((char) c, isFriend);
                    c = lastcolor;
                }
            }
        }
        else if (c == '\033') { /* Escape character */
            ansistate = 4;
            if (!flags.useansi) {
                flags.useansi = 1;
                if (!flags.usebold)
                    flags.offbold = 1;
            }
        }
        /* Change color for end of more prompt */
        if (flags.useansi && flags.moreflag && c == ' ')
            std_printf ("\033[3%cm", lastcolor = color.text);   /* assignment */

        /* Output character */
        std_putchar (c);
    }
    else {
        *posthdrp++ = c;        /* store character in buffer */
        *posthdrp = 0;

        /* If reached a \r it's time to do header processing */
        if (c == '\r') {
            needs.prochdr = 0;

            /* Process for enemy list kill file */
            strcpy (junk, posthdr);
            /* FIXME: name of enemy should not be added to ^N list */
            bp = ExtractName (junk);
/*      strcpy(junk, posthdr);  * Why did I do this? */
            isFriend = (ulist_find (&friendList, bp) != NULL);
            ansi_transform_posthdr (posthdr, isFriend);
            strcpy (saveheader, posthdr);
            strcat (saveheader, "\r\n");
            if (ulist_find (&enemyList, bp)) {
                needs.ignore = 1;
                postnow = true; // this was -1. Not sure what -1 was supposed to signify.
                net_putchar_unsyncd (IAC);
                net_putchar_unsyncd (POST_K);
                net_putchar_unsyncd (numposts & 0xFF);
                net_putchar_unsyncd (17);
                netflush ();
                if (!flags.squelchpost)
                    std_printf ("%s[Post by %s killed]\r\n", *posthdr == '\n' ? "\r\n" : "", bp);
                else {
                    eatline = (needs.crlf ? 1 : 2);
                    net_putchar ('n');
                    netflush ();
                }
                return;
            }
            std_printf ("%s%s\r", (needs.crlf) ? "\r\n" : "", posthdr);
        }
    }
}


static void
filter_url (char *line)
{
    static int multiline = 0;
    static char url[1024];
    char   *p, *q;
    int     c;

    if (!urlQueue)              /* Can't store URLs for some reason */
        return;

    if (!multiline)
        strcpy (url, line);
    else
        strcat (url, line);

    for (c = strlen (url) - 1; c >= 0; c--)
        if (url[c] == ' ' || url[c] == '\t' || url[c] == '\r')
            url[c] = 0;
        else
            break;
    strcat (url, " ");

    if (!(p = strstr (url, "http://"))) {
        if (!(p = strstr (url, "ftp://"))) {
            url[0] = 0;
            multiline = 0;
            return;
        }
    }

    for (q = p; *q; q++) {
        if (strchr (":/abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789$-_@.&+,=;?%~{|}", *q))
            continue;
        *q = 0;

        /* Oops, looks like a multi-line URL */
        if ((!multiline && p == line && q > p + 77) || (multiline && strlen (line) > 77))
            if (strlen (line) > 77)
                break;

        if (!is_queued (p, urlQueue)) {
            if (!push_queue (p, urlQueue)) {
                // the queue was full. pop something and discard it.
                string *junk = new_string (1024);

                pop_queue (str_data (junk), urlQueue);
                delete_string (junk);
                push_queue (p, urlQueue);
            }
        }
/*	    printf("\r\nSnarfed URL: <%s>\r\n", urls[nurls - 1]); */
        multiline = 0;
        return;
    }

    multiline = 1;
/*	printf("Multiline URL, got <%s> so far.\r\n", url); */
    return;
}


void
filter_data (int c)
{
    static int ansistate = 0;   /* Counter for ANSI transformations */

    /* Copy the current line (or what we have so far) */
    if (c == '\n') {
        filter_url (thisline);
        thisline[0] = 0;
    }
    else {
        /* This is faster than calling strcat() */
        char   *p = thisline;

        for (; *p; p++) ;       /* Find end of string */

        *p++ = c;               /* Copy character to end of string */
        *p = 0;
    }

    /* Auto-answer ANSI question */
    if (flags.ansiprompt && strstr (thisline, "Are you on an ANSI")) {
        net_putchar ('y');
        *thisline = 0;          /* Kill it; we don't need it */
    }

    /* Automatic X reply */
    /*
       if (SendingX == SX_SENT_NAME) {
       SendingX = SX_SEND_NEXT;
       #ifdef DEBUG
       std_printf("filter_data 1 SendingX is %d, xland is %d\r\n", SendingX, xland);
       #endif
       }
     */
    /*  if (SendingX == SX_SEND_NEXT && xlandQueue->nobjs && (away || xland))
       send_an_x();
     */
    if (SendingX == SX_SEND_NEXT && !*thisline && c == '\r') {
        SendingX = SX_NOT;
#ifdef DEBUG
        std_printf ("filter_data 2 SendingX is %d, xland is %d\r\n", SendingX, xland);
#endif
        if (queue_size (xlandQueue) && (away || xland))
            send_an_x ();
    }

    /* Change color for end of more prompt */
    if (flags.useansi && flags.moreflag && c == ' ')
        std_printf ("\033[3%cm", lastcolor = color.text);   /* assignment */

    /* Parse ANSI sequences */
    if (ansistate) {
        ansistate--;
        if (ansistate == 1) {
            if (flags.offbold && c == 109) {    /* Turn boldface off */
                std_printf ("m\033[0");
                ansistate--;
            }
            else {
                lastcolor = ansi_transform ((char) c);
                c = lastcolor;
            }
        }
    }
    else if (c == '\033') {     /* Escape character */
        std_printf ("\033[4%cm", color.background);
        ansistate = 4;
        if (!flags.useansi) {
            flags.useansi = 1;
            if (!flags.usebold)
                flags.offbold = 1;
        }
    }
    if (eatline > 0) {
        if (c == '\n') {
            eatline--;
        }
    }
    else {
        eatline = 0;            /* eatline should never be less than 0 */
        std_putchar (c);
    }
    return;
}


void
reprint_line (void)
{
    // save a copy of thisline, and send it through the filter.
    char   *sav = strdup (thisline);

    std_putchar ('\r');
    for (const char *p = sav; *p; p++)
        filter_data (*p);

    // restore the saved line
    strcpy (thisline, sav);
    free (sav);
}

static void
continued_data_helper (void)
{
    static const char junk[] = { "\033[32m" };

    for (const char *s = junk; *s; ++s)
        filter_data (*s);
}

void
moreprompt_helper (void)
{
    if (!flags.useansi)
        return;

    if (postnow)
        continued_post_helper ();
    else
        continued_data_helper ();
}


void
continued_post_helper (void)
{
    static const char junk[] = { "\033[32m" };

    for (const char *s = junk; *s; ++s)
        filter_post (*s);
}



/* Check for an automatic reply message. Return 1 if this is such a message. */
static  bool
is_automatic_reply (const char *message)
{
    /* Find first line */
    char   *p = strstr (message, ">");

    /* Wasn't a first line? - move past '>' */
    if (!p)
        return false;
    ++p;

    /* Check for valid automatic reply messages */
    if (!strncmp (p, "+!R", 3) ||
        !strncmp (p, "This message was automatically generated", 40) ||
        !strncmp (p, "*** ISCA Windows Client", 23))
        return true;

    /* Looks normal to me... */
    return false;
}


static void
not_replying_transform_express (char *s)
{
    char    junk[580];
    char   *sp;

    /* Verify this is an X message and set up pointers */
    sp = strstr (s, " at ");
    if (!sp)
        return;

    *(sp++) = 0;

    sprintf (junk, "%s (not replying) %s", s, sp);
    strcpy (s, junk);
}


static void
replycode_transform_express (char *s)
{
    char    junk[580];
    char   *sp;

    /* Verify this is an auto reply and set up pointers */
    sp = strstr (s, ">");
    if (!sp || strncmp (sp, ">+!R ", 5))
        return;

    *(++sp) = 0;
    sp += 4;

    sprintf (junk, "%s%s", s, sp);
    strcpy (s, junk);
}
