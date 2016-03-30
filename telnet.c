/*
 * This handles the information flowing into the client from the BBS.  It
 * basically understands a very limited subset of the telnet protocol (enough
 * to "fake it" with the BBS) with some extensions to allow the extra features
 * the client provides.  The telnet stuff is unimportant, but it'll confuse the
 * BBS if you alter it. 
 *
 * The client tells the BBS it is a client rather a telnet program when it first
 * connects, after that the BBS acts differently, letting the client know when
 * something in particular is being done (entering an X message, posting, etc.)
 * to allow the client to handle that however it wants, as well as providing
 * extra protocol for the start/end of messages/X's and the special who list. 
 *
 * This is made more complex by the fact that the client doesn't know it should
 * handle (for example) an X message differently until it receives word from
 * the BBS that an X message should be entered -- but by this time the client
 * may have already sent some of the X message over the network instead of
 * gathering it up locally.  So when the BBS tells the client to go into the
 * local X message mode it also tells the client how many bytes have been
 * passed to it (they count them in the same manner) and throws away the excess
 * on its side.  The client has buffered this excess on its side and therefore
 * make it available locally instead of having it lost forever.  Boy, that was
 * a pisser when I realized I'd have to do that! 
 */
#include "defs.h"
#include "ext.h"
#include "telnet.h"


int
telrcv (int c)
{
    static enum Telnet_State state = TS_DATA;   /* Current state of telnet state machine */
    static unsigned char buf[80];   /* Generic buffer */
    static int bufp = 0;        /* Pointer into generic buffer */
    static int numposts = 0;    /* Count of # of posts we've received so far */
    int     i;
    char   *sp;

    switch (state) {
    case TS_DATA:              /* normal data */
        if (c == IAC) {         /* telnet Is A Command (IAC) byte */
            state = TS_IAC;
            break;
        }
        if (recving_wholist)    /* We are currently receiving a wholist */
            filter_wholist (c);
        else if (xmsgnow)       /* We are currently receiving an X message */
            filter_express (c);
        else if (postnow)       /* We are currently receiving a post */
            filter_post (c);
        else                    /* Garden-variety data (I hope!) */
            filter_data (c);
        break;

        /* handle various telnet and client-specific IAC commands */
    case TS_IAC:
        switch (c) {

            /*
             * This is sent/received when the BBS thinks the client is
             * 'inactive' as per the normal 10 minute limit in the BBS -- the
             * actual inactive time limit for a client is an hour, this is done
             * to make sure the client is still alive on this end so dead
             * connections can be timed out and not be ghosted for an hour. 
             */
        case CLIENT:
#if DEBUG
            std_printf ("{IAC CLIENT}");
#endif
            state = TS_DATA;
            net_putchar (IAC);
            net_putchar (CLIENT);
            break;

        case S_WHO:            /* start who list transfer */
#if DEBUG
            std_printf ("{IAC S_WHO}");
#endif
            state = TS_DATA;
            recving_wholist = true;
            break;

        case G_POST:           /* get post */
        case G_FIVE:           /* get five lines (X or profile info) */
        case G_NAME:           /* get name */
        case G_STR:            /* get string */
        case CONFIG:           /* do configuration */
#if DEBUG
            std_printf ("{IAC %s",
                        c == G_POST ? "G_POST" :
                        c == G_FIVE ? "G_FIVE" :
                        c == G_NAME ? "G_NAME" : c == G_STR ? "G_STR" : c == CONFIG ? "CONFIG" : "huh?");
#endif
            state = TS_GET;
            buf[bufp++] = c;
            break;

            /*
             * This code is used by the bbs to signal the client to synchronize
             * its count of the current byte we are on.  We then send back a
             * START to the bbs so it can synchronize with us.  NOTE:  If it
             * becomes necessary in the future to introduce incompatible changes
             * in the BBS and the client, this will be how the BBS can
             * differentiate between new clients that understand the new stuff,
             * and old clients which the BBS will probably still want to be
             * compatible with. Instead of START a new code can be sent to let
             * the BBS know we understand the new stuff and will operate
             * accordingly.  If there are multiple different versions of the BBS
             * to worry about as well (gag!) then more logic would be needed, I
             * refuse to worry about this case, if it comes about it'll be after
             * I ever have to worry about or maintain that BBS code! 
             */
        case START:
#if DEBUG
            std_printf ("{IAC START}");
#endif
            state = TS_DATA;
            byte = 1;
            numposts = 1;
            net_putchar (IAC);
            net_putchar (START3);
            break;

        case POST_S:           /* Start of post transfer */
#if DEBUG
            std_printf ("{IAC POST_S}");
#endif
            state = TS_DATA;
            postnow = true;
            filter_post (-1);   /* tell filter to start working */
            break;

        case POST_E:           /* End of post transfer */
#if DEBUG
            std_printf ("{IAC POST_E}");
#endif
            state = TS_DATA;
            numposts++;
            postnow = false;
            filter_post (-1);   /* Tell filter to end working */
            break;

        case MORE_M:           /* More prompt marker */
#if DEBUG
            printf ("{IAC MORE_M}");
#endif
            state = TS_DATA;
            flags.moreflag ^= 1;
            if (!flags.moreflag && flags.useansi)
                moreprompt_helper ();   /* KLUDGE */
            break;

        case XMSG_S:           /* Start of X message transfer */
#if DEBUG
            std_printf ("{IAC XMSG_S}");
#endif
            state = TS_DATA;
            *parsing = 0;
            xmsgnow = true;
            filter_express (-1);    /* tell filter to start working */
            break;

        case XMSG_E:           /* End of X message transfer */
#if DEBUG
            std_printf ("{IAC XMSG_E}");
#endif
            state = TS_DATA;
            *parsing = 0;
            xmsgnow = false;
            xmsgbufp = xmsgbuf;
            filter_express (-1);    /* Tell filter to end working */
            if (needx) {
                send_an_x ();
                needx = false;
            }
            break;

            /* telnet DO/DONT/WILL/WONT option negotiation commands (ignored) */
        case DO:
        case DONT:
        case WILL:
        case WONT:
#if DEBUG
            std_printf ("{IAC %s ",
                        c == DO ? "DO" :
                        c == DONT ? "DONT" : c == WILL ? "WILL" : c == WONT ? "WONT" : "wtf?");
#endif
            state = TS_VOID;
            break;

        default:
#if DEBUG
            std_printf ("{IAC 0x%2X}", c);
#endif
            state = TS_DATA;
            break;
        }
        break;

        /* Get local mode strings/lines/posts */
    case TS_GET:
        buf[bufp++] = c;
        if (bufp == 5) {
            targetbyte = byte;
            /* Decode the bbs' idea of what the current byte is */
            byte = bytep = ((long) buf[2] << 16) + ((long) buf[3] << 8) + buf[4];

            /*
             * If we are more out of sync than our buffer size, we can't
             * recover.  If we are out of sync but not so far out of sync we
             * haven't overrun our buffers, we just go back into our buffer and
             * find out what we erroneously sent over the network, and reuse it.
             */
            if (byte < targetbyte - (int) (sizeof save) - 1)
                std_printf ("\r\n[Error:  characters lost during transmission]\r\n");
            state = TS_DATA;
            bufp = 0;
            switch (*buf) {
            case G_POST:       /* get post */
                if (flags.posting) {
                    flags.check = 0;
                    return -1;
                }
#if DEBUG
                std_printf ("}\r\n");
#endif
                makemessage (buf[1]);
                break;

            case G_FIVE:       /* get five lines (X message, profile) */
                get_five_lines (buf[1]);
                if (buf[1] == 1 && xlandQueue->nobjs > 0)
                    send_an_x ();
                break;

            case G_NAME:       /* get name */
                sendblock ();
                sp = get_name (buf[1]);
                for (i = 0; sp[i]; i++)
                    net_putchar (sp[i]);
                if (*sp != CTRL_D) {
                    net_putchar ('\n');
                    byte += i + 1;
                }
                else
                    byte++;
                break;

            case G_STR:        /* get string */
#if DEBUG
                std_printf (" 0x%X} ", buf[1]);
#endif
                sendblock ();

                // We cast the unsigned char at buf[1] because a negative value is meaningful.
                get_string ((char) buf[1], (char *) buf, -1);
                for (i = 0; buf[i]; i++)
                    net_putchar (buf[i]);
                net_putchar ('\n');
                byte += i + 1;
                break;

            case CONFIG:       /* do configuration */
#if DEBUG
                std_printf ("}");
#endif
                sendblock ();
                configbbsrc ();
                net_putchar ('\n');
                byte++;
                break;
            }
        }
        break;

        /* Ignore next byte (used for ignoring negotations we don't care about) */
    case TS_VOID:
#if DEBUG
        std_printf ("0x%X}", c);
#endif
        /*
         * This patch sends IAC WONT in response to a telnet negotiation;
         * this provides compatibility with a standard telnet daemon, e.g.
         * Heinous BBS.  Added by IO ERROR.
         */
        net_putchar (IAC);
        net_putchar (WONT);
        net_putchar (c);
        /* Fall through */
    default:
        state = TS_DATA;
        break;
    }
    return 0;
}


/*
 * Send signal that block of data follows -- this is a signal to the bbs that
 * it should stop ignoring what we send it, since it begins ignoring and
 * throwing away everything it receives from the time it sends an IAC G_*
 * command until the time it receives an IAC BLOCK command. 
 */
void
sendblock (void)
{
    net_putchar (IAC);
    net_putchar (BLOCK);
}


/*
 * Send a NAWS command to the bbs to tell it what our window size is. 
 */
void
sendnaws (void)
{
    char    s[10];

    if (oldrows != getwindowsize ()) {
        /* Old window max was 70 */
        if (rows > 110 || rows < 10)
            rows = 24;
        else
            oldrows = rows;
        sprintf (s, "%c%c%c%c%c%c%c%c%c", IAC, SB, TELOPT_NAWS, 0, 0, 0, rows, IAC, SE);
        for (int i = 0; i < 9; i++)
            net_putchar (s[i]);
    }
}


/*
 * Initialize telnet negotations with the bbs -- we don't really do the
 * negotations, we just tell the bbs what it needs to hear, since we don't need
 * to negotiate because we know the correct state to put the terminal in. The
 * BBS (the queue daemon actually) is kludged on its end as well by the IAC
 * CLIENT command. 
 */
void
telinit (void)
{
    char    s[39];
    int     i;


    sprintf (s, "%c%c%c%c%c%c%cUSER%c%s%c%c", IAC, CLIENT2, IAC, SB, TELOPT_ENVIRON, 0, 1, 0, username, IAC,
             SE);
    for (i = 0; i < (int) strlen (username) + 14; i++)
        net_putchar (s[i]);
    sendnaws ();
}
