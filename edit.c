/*
 * This is just a hacked-up version of the editor from the BBS...ugly, isn't
 * it?  You don't need to mess with this unless you have a lot of time to waste
 * on a lost cause.  Use a real editor, that is what '.edit' is for! 
 */
#include "defs.h"
#include "ext.h"
static int checkfile (FILE * fp);
static int prompt (FILE * fp, int *old, int cmd);


void
makemessage (int upload)
{                               /* 0 = normal, 1 = upload (end w/^D) */
    int     chr;
    FILE   *fp = tempfile;
    int     i;
    int     lnlngth;            /* line length */
    int     lastspace;          /* position of last space encountered */
    int     cancelspace;        /* true when last character is space */
    char    thisline[81];       /* array to save current line */
    int     old = '\n';
    unsigned int invalid = 0;
    int     tabcount = 0;


    flags.posting = 1;
    if (away) {
        printf ("[No longer away]\r\n");
        away = false;
    }
    if (capture) {
        printf ("[Capture to temp file turned OFF]\r\n");
        capture = 0;
        fflush (tempfile);
    }
    rewind (fp);
    if (flags.lastsave) {
        (void) freopen (tempfilename, "w+", tempfile);
        flags.lastsave = 0;
    }
    if (getc (fp) >= 0) {
        rewind (fp);
        printf ("There is text in your edit file.  Do you wish to erase it? (Y/N) -> ");
        if (yesno ())
            (void) freopen (tempfilename, "w+", tempfile);
        else {
            (void) checkfile (fp);
            old = -1;
        }
    }
    lnlngth = 0;
    lastspace = 0;
    cancelspace = 0;

    for (;;) {
        if (ftell (fp) > 48700) {
            if (old != -1) {
                printf ("\r\nMessage too long, must Abort or Save\r\n\n");
                fflush (stdout);
                mysleep (1);
                flush_input (0);
                tabcount = 0;
            }
            chr = CTRL_D;
            old = '\n';
        }
        else if (old < 0)
            chr = 'P';
        else if (tabcount) {
            tabcount--;
            chr = ' ';
        }
        else {
            chr = inkey ();
            if (old >= 0 && iscntrl (chr) && chr == CTRL_D && !upload)
                chr = 1;        /* Just a random invalid character */

            if (old >= 0 && chr != CTRL_D && chr != TAB && chr != '\b' && chr != '\n' && chr != CTRL_X
                && chr != CTRL_W && chr != CTRL_R && !isprint (chr)) {
                if (invalid++)
                    flush_input (invalid);
                continue;
            }
            invalid = 0;
        }

        if (chr == CTRL_R) {
            thisline[lnlngth + 1] = 0;
            printf ("\r\n%s", thisline + 1);
            continue;
        }
        if (chr == TAB) {
            tabcount = 7 - (lnlngth & 7);
            chr = ' ';
        }
        if (chr == '\b') {
            if (lnlngth) {
                putchar ('\b');
                putchar (' ');
                putchar ('\b');
                if (lnlngth-- == lastspace)
                    for (; --lastspace && thisline[lastspace] != ' ';) ;
                if (!lnlngth)
                    old = '\n';
            }
            continue;
        }
        if (chr == CTRL_W) {    /* ctrl-W is 'word erase' from Unix */
            for (i = 0; i < lnlngth; i++)
                if (thisline[i + 1] != ' ')
                    break;
            for (i = i == lnlngth; lnlngth && (!i || thisline[lnlngth] != ' '); lnlngth--) {
                if (thisline[lnlngth] != ' ')
                    i = 1;
                putchar ('\b');
                putchar (' ');
                putchar ('\b');
            }
            if (!lnlngth || thisline[lnlngth] == ' ')
                lastspace = lnlngth;
            if (!lnlngth)
                old = '\n';
            continue;
        }
        if (chr == CTRL_X) {    /* ctrl-X works like in normal Unix */
            for (; lnlngth; lnlngth--) {
                putchar ('\b');
                putchar (' ');
                putchar ('\b');
            }
            lastspace = 0;
            old = '\n';
            continue;
        }
        /*
         * Ignore space when the last character typed on a the previous line
         * and the first character typed on this line are both spaces.  This
         * makes free flow typing easier and properly formatted. 
         */
        if (cancelspace && !(cancelspace = 0) && chr == ' ')
            continue;

        if (chr != '\n' && chr != CTRL_D && old != -1)
            lnlngth++;

        if (chr == ' ' && (lastspace = lnlngth) == 80) {
            cancelspace = 1;
            chr = '\n';
            for (; --lnlngth && thisline[lnlngth] == ' ';) ;
        }
        if (lnlngth == 80) {
            if (lastspace > (80 / 2)) { /* don't autowrap past 40th column */
                for (i = 80 - 1; i && (i > lastspace || thisline[i] == ' '); i--)
                    if (i > lastspace) {
                        putchar ('\b');
                        putchar (' ');
                        putchar ('\b');
                    }
                for (lnlngth = 1; lnlngth <= i; lnlngth++)
                    if (putc (thisline[lnlngth], fp) < 0)
                        tempfileerror ();
                if (putc ('\n', fp) < 0)
                    tempfileerror ();
                printf ("\r\n");
                for (lnlngth = 1; (lnlngth + lastspace) < 80; lnlngth++) {
                    old = thisline[lnlngth + lastspace];
                    putchar (old);
                    thisline[lnlngth] = old;
                }
                lastspace = 0;
            }
            else {
                for (i = 1; i < 80; i++)
                    if (putc (thisline[i], fp) < 0)
                        tempfileerror ();
                if (putc ('\n', fp) < 0)
                    tempfileerror ();
                printf ("\r\n");
                old = '\n';
                lastspace = 0;
                lnlngth = 1;
            }
        }
        if (chr != CTRL_D && chr != '\n' && old != -1) {
            putchar (chr);      /* echo user's input to screen */
            thisline[lnlngth] = chr;
        }
        else if (lnlngth && chr == CTRL_D) {    /* simulate LF */
            for (i = 1; i <= lnlngth; i++)
                if (putc (thisline[i], fp) < 0)
                    tempfileerror ();
            if (putc ('\n', fp) < 0)
                tempfileerror ();
            printf ("\r\n");
            lastspace = lnlngth = 0;
        }
        if ((old != '\n' || chr != '\n' || upload) && chr != CTRL_D && old != -1) {
            old = chr;
            if (chr == '\n') {
                for (i = 1; i <= lnlngth; i++)
                    if (putc (thisline[i], fp) < 0)
                        tempfileerror ();
                if (putc ('\n', fp) < 0)
                    tempfileerror ();
                printf ("\r\n");
                lastspace = lnlngth = 0;
            }
            continue;           /* go back and get next character */
        }
        else {                  /* 2 LFs in a rows (or a ctrl-D) */
            if (fflush (fp) < 0)    /* make sure we've written it all */
                tempfileerror ();

            if (prompt (fp, &old, chr) < 0)
                return;
        }
    }
}



/*
 * This function used to be part of edit(), it was broken out because stupid
 * DEC optimizers found edit() too long to optimize without a warning, and that
 * warning made people think something went wrong in the compilation.  This
 * also might even make this stuff easier for others to understand, but I doubt
 * it.
 */
static int
prompt (FILE * fp, int *old, int cmd)
{
    FILE   *cp;
    int     i;
    int     chr = cmd;
    int     lnlngth;
    unsigned int invalid = 0;
    int     size;
    int     lines;
    char    thisline[80];

    for (i = 0;;) {
        if (*old != -1) {
            if (i != 1) {
                sendblock ();
                net_putchar (CTRL_D);
                net_putchar ('c');
                flags.check = 1;
                (void) inkey ();
                if (flags.useansi)
                    colorize ("@YA@Cbort  @YC@Continue  @YE@Cdit  @YP@Crint  @YS@Cave  @YX@Cpress -> @G ");
                else
                    printf ("<A>bort <C>ontinue <E>dit <P>rint <S>ave <X>press -> ");
                fflush (stdout);
            }
            i = 0;
            /* Make 'x' work at this prompt for xland function */
            if (!(xland && xlandQueue->nobjs)) {
                while (!strchr (" \naAcCeEpPsSQtTx?/", chr = inkey ()))
                    if (invalid++)
                        flush_input (invalid);
                invalid = 0;
            }
            else
                chr = 'x';
        }
        switch (chr) {
        case ' ':
        case '\n':
            if (!i++)
                continue;
            flush_input ((unsigned) i); /* FIXME: figure out what the hell this code is doing! */
            printf ("\r\n");
            continue;

        case 'a':
        case 'A':
            printf ("Abort: are you sure? ");
            if (yesno ()) {
                sendblock ();
                net_putchar (CTRL_D);
                net_putchar ('a');
                flags.posting = 0;
                return -1;
            }
            continue;

        case 'c':
        case 'C':
            printf ("Continue...\r\n");
            if (flags.useansi)
                continued_post_helper ();   /* KLUDGE */
            break;

        case 'p':
        case 'P':
            if (*old == -1)
                *old = '\n';
            else
                printf ("Print formatted\r\n\n%s", saveheader);
            fseek (fp, 0L, SEEK_END);
            size = ftell (fp);
            rewind (fp);
            lines = 2;
            lnlngth = i = 0;
            while ((chr = getc (fp)) > 0) {
                i++;
                if (chr == TAB) {
                    do
                        putchar (' ');
                    while (++lnlngth & 7);
                }
                else {
                    if (chr == '\n')
                        putchar ('\r');
                    putchar (chr);
                    lnlngth++;
                }
                if (chr == '\n' && !(lnlngth = 0) && ++lines == rows && more (&lines, i * 100 / size) < 0)
                    break;
            }
            fseek (fp, 0L, SEEK_END);
            break;

        case 's':
        case 'S':
            printf ("Save message\r\n");
            if (checkfile (fp))
                continue;
            rewind (fp);
            sendblock ();
            while ((chr = getc (fp)) > 0) {
                net_putchar (chr);
            }
            net_putchar (CTRL_D);
            net_putchar ('s');
            flags.lastsave = 1;
            flags.posting = 0;
            return -1;

        case 'Q':
        case 't':
        case 'T':
        case 'x':
        case '?':
        case '/':
            sendblock ();
            net_putchar (CTRL_D);
            net_putchar (chr);
            looper ();
            net_putchar ('c');
            // The old code did not increment sync_byte here after calling net_putchar(). It was probably a bug,
            // Uncommenting this will maintain the old behavior.
            //--sync_byte;
            continue;

        case 'e':
        case 'E':
            printf ("Edit\r\n");
            if (!*editor)
                printf ("[Error:  no editor available]\r\n");
            else {
                if (chr == 'E') {
                    fseek (fp, 0L, SEEK_END);
                    if (ftell (fp)) {
                        printf ("\r\nThere is text in your edit file.  Do you wish to erase it? (Y/N) -> ");
                        if (yesno ())
                            (void) freopen (tempfilename, "w+", tempfile);
                        else
                            continue;
                    }
                    printf ("\r\nFilename -> ");
                    get_string (67, thisline, -999);
                    if (!*thisline)
                        continue;
                    if (!(cp = fopen (thisline, "r"))) {
                        printf ("\r\n[Error:  named file does not exist]\r\n\n");
                        continue;
                    }
                    else {
                        while ((i = getc (cp)) >= 0)
                            if (putc (i, fp) < 0) {
                                tempfileerror ();
                                break;
                            }
                        if (feof (cp) && fflush (fp) < 0)
                            tempfileerror ();
                        fclose (cp);
                    }
                }
                /* We have to close and reopen the tempfile due to locking */
                fclose (tempfile);
                run (editor, tempfilename);
                if (!(tempfile = fopen (tempfilename, "a+")))
                    fatalperror ("opentmpfile: fopen", "Local error");
                if (flags.useansi)
                    printf ("\033[%cm\033[3%cm", flags.usebold ? '1' : '0', lastcolor);
                printf ("[Editing complete]\r\n");
                (void) freopen (tempfilename, "r+", tempfile);
                if (checkfile (fp)) {
                    fflush (stdout);
                    mysleep (1);
                }
            }
            continue;
        }
        return 0;
    }
}



/*
 * Checks the file for lines longer than 79 characters, unprintable characters,
 * or the file itself being too long.  Returns 1 if the file has problems and
 * cannot be saved as is, 0 otherwise. 
 */
static int
checkfile (FILE * fp)
{
    int     i;
    int     count = 0;
    int     line = 1;
    int     total = 0;

    rewind (fp);
    while (!feof (fp))
        if ((i = getc (fp)) != '\r' && i != '\n') {
            if ((i >= 0 && i < 32 && i != TAB) || i >= DEL) {
                printf ("\r\n[Warning:  illegal character in line %d, edit file before saving]\r\n\n", line);
                return 1;
            }
            else if ((count = i == TAB ? (count + 8) & 0xf8 : count + 1) > 79) {
                printf ("\r\n[Warning:  line %d too long, edit file before saving]\r\n\n", line);
                return 1;
            }
        }
        else {
            total += count;
            count = 0;
            line++;
        }
    if (total > 48800) {
        printf ("\r\n[Warning:  message too long, edit file before saving]\r\n\n");
        return 1;
    }
    return 0;
}
