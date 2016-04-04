#include "defs.h"
#include "ext.h"
#include <stdarg.h>

static void otherinfo (void);

static void
feed_pager (int startrow, ...)
{
    int     row;
    char   *s;
    va_list ap;

    row = startrow;
    va_start (ap, startrow);
    for (;;) {
        s = va_arg (ap, char *);

        if (!s)
            break;
        std_printf ("%s", s);
        row++;
        if (row == rows) {
            /* More prompt */
            printf ("--MORE-- ");
            fflush (stdout);
            switch (inkey ()) {
            case 'n':
            case 'N':
            case 's':
            case 'S':
            case 'q':
            case 'Q':
                printf ("\r        \r");
                va_end (ap);
                return;
                /* NOTREACHED */
            case '\n':
                row--;
                printf ("\r        \r");
                break;
            default:
                row = 1;
                printf ("\r        \r");
                break;
            }
        }
    }
    va_end (ap);
    return;
}

static void
techinfo (void)
{
    std_printf ("Technical information\r\n\n");

    feed_pager (3, "ISCA BBS Client " VERSION " (Unix)\r\n", "Compiled on: " HOSTTYPE "\r\n", "With: "
#ifdef __STDC__
                "ANSI "
#endif
#ifdef __cplusplus
                "C++ "
#endif
#ifdef __GNUC__
                "gcc "
#endif
#ifdef _POSIX_SOURCE
                "POSIX "
#endif
                "save-password "
#ifdef USE_POSIX_SIGSETJMP
                "sigsetjmp "
#endif
                "\r\n", NULL);
}


static void
copyright (void)
{
    std_printf ("Copyright\r\n\n");

    feed_pager (3, "Copyright (C)\r\n",
                "  1995-2003 Michael Hampton.      (GPL: Cool stuff; current maintainer)\r\n",
                "  1993-1994 Doug Siebert.         (GPL: Client core)\r\n", "Portions Copyright (C)\r\n",
                "  1995 Jonathan Pickard.          (GPL: Saved-password algorithm)\r\n",
                "  1994 David Bailey.              (GPL: Expanded friend list)\r\n",
                "  1994 Marc Dionne.               (GPL: Early patches to client core)\r\n",
                "  Above portions used with permission.\r\n", NULL);
}


static void
license (void)
{
    std_printf ("License\r\n\n");

    feed_pager (3, "                    GNU GENERAL PUBLIC LICENSE\r\n",
                "                       Version 3, 29 June 2007\r\n", "\r\n",
                " Copyright (C) 2007 Free Software Foundation, Inc. <http://fsf.org/>\r\n",
                " Everyone is permitted to copy and distribute verbatim copies\r\n",
                " of this license document, but changing it is not allowed.\r\n", "\r\n",
                "See https://gnu.org/licenses/gpl.txt for details.\r\n", NULL);
}


static void
warranty (void)
{
    std_printf ("Warranty\r\n\n");

    feed_pager (3, "  THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY\r\n",
                "APPLICABLE LAW.  EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT\r\n",
                "HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY\r\n",
                "OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,\r\n",
                "THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR\r\n",
                "PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM\r\n",
                "IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF\r\n",
                "ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\r\n", "\r\n",
                "See https://gnu.org/licenses/gpl.txt for details.\r\n", NULL);
}


/*
 * information() displays general info about the client
 */
void
information (void)
{
    char    c;
    int     invalid = 0;

    std_printf ("Information\r\n");

    for (;;) {
        if (flags.useansi)
            colorize
                ("\r\n@YC@Copyright  @YL@Cicense  @YW@Carranty  @YT@Cechnical  @YQ@Cuit\r\n@YClient information -> @G");
        else
            std_printf ("\r\n<C>opyright <L>icense <W>arranty  <T>echnical <Q>uit\r\nClient information -> ");
        for (invalid = 0;;) {
            c = inkey ();
            if (!strchr ("CcLlOoWwTtQq \n", c)) {
                if (invalid++)
                    flush_input (invalid);
                continue;
            }
            break;
        }
        switch (c) {
        case 'c':
        case 'C':
            copyright ();
            break;
        case 'l':
        case 'L':
            license ();
            break;
        case 'o':
        case 'O':
            otherinfo ();
            break;
        case 't':
        case 'T':
            techinfo ();
            break;
        case 'w':
        case 'W':
            warranty ();
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
otherinfo (void)
{
    feed_pager (1,
                "\r\n",
                "\r\n",
                "Important things to remember about ISCA BBS:\r\n",
                "\r\n",
                "1.  The content in Babble> strictly follows the Webster's dictionary\r\n",
                "    definition.\r\n",
                "2.  Regarding ISCA BBS women, the odds may be good, but the goods may\r\n",
                "    be odd.\r\n",
                "3.  Houston used to be gay.\r\n",
                "4.  Non Sequitur> usually isn't.  And neither was Weird>.\r\n",
                "5.  To get into Kama Sutra>, you have to be 18 or have a good fake ID.\r\n",
                "6.  If you make your information public, God Man Megatron will post it\r\n",
                "    on the Web within 24 hours.  Guaranteed.\r\n",
                "7.  The Policy Board is your friend.  The Policy Board has always been\r\n",
                "    your friend.  The Policy Board will always be your friend.\r\n",
                "8.  The BBS is never moving to Gestalt.\r\n",
                "9.  The performers may change, but the song is always the same.\r\n", NULL);
}
