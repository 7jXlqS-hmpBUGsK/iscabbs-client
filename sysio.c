/* System I/O routines.
 */

#include "defs.h"
#include "ext.h"
#include <stdarg.h>

char    swork[BUFSIZ];          /* temp buffer for color stripping */

bool
NET_INPUT_LEFT (void)
{
    return (netifp - netibuf) < netilen;
}

bool
INPUT_LEFT ()
{
    return (ptyifp - ptyibuf) < ptyilen;
}

int
ptyget (void)
{
    if (INPUT_LEFT ())
        return *ptyifp++;

    if ((ptyilen = read (0, ptyibuf, sizeof ptyibuf)) < 0)
        return -1;
    ptyifp = ptyibuf;
    return *ptyifp++;
}

int
netget (void)
{
    if (NET_INPUT_LEFT ())
        return *netifp++;
    if ((netilen = read (net, netibuf, sizeof netibuf)) <= 0)
        return -1;
    netifp = netibuf;
    return *netifp++;
}

static int
netput (int c)
{
    return putc (c, netofp);
}

/* std_putchar() and cap_putchar() write a single character to stdout and the
 * capture file, respectively.  On error, they terminate the client.
 */
int
std_putchar (int c)
{
    if (putchar (c) < 0)
        fatalperror ("std_putchar", "Local error");
    cap_putchar (c);
    return c;
}

int
cap_putchar (int c)
{
    static int skipansi = 0;    /* Counter for avoidance of capturing ANSI */

    if (skipansi) {
        skipansi--;
        if (skipansi == 1) {
            if (flags.offbold && c == 109) {    /* Damned weird kludge */
                printf ("\033[0m");
                skipansi--;
            }
            else {
                lastcolor = c;
            }
        }
    }
    else if (c == '\033') {
        skipansi = 4;
    }
    else if (capture > 0 && !flags.posting && !flags.moreflag && c != '\r') {
        if (putc (c, tempfile) < 0) {
            tempfileerror ();
        }
    }
    return c;
}

int
netflush (void)
{
    return fflush (netofp);
}

int
net_putchar (int c)
{
    return netput (c);
}

/* stripansi removes ANSI escape sequences from a string.  Limits: string
 * buffer space is BUFSIZ bytes, should not overflow this!!
 */
static char *
stripansi (char *c)
{
    char   *p, *q;

    q = swork;
    for (p = c; *p != '\0'; p++) {
        if (*p != '\033')
            *q++ = *p;
        else
            for (; *p != '\0' && !isalpha (*p); p++) ;
    }
    if (*p == '\r')             /* strip ^M too while we're here */
        q--;
    *q = '\0';
    strcpy (c, swork);
    return c;
}

/* std_puts and cap_puts write a string to stdout.  They differ from libc *puts
 * in that they do NOT write a trailing \n to the stream.  On error, they
 * terminate the client.
 */
static int
std_puts (char *c)
{
    printf ("%s", c);
    fflush (stdout);
    cap_puts (c);
    return 1;
}

int
cap_puts (char *c)
{
    if (capture > 0 && !flags.posting && !flags.moreflag) {
        stripansi (c);
        fprintf (tempfile, "%s", c);
        fflush (tempfile);
    }
    return 1;
}

static int
net_puts (char *c)
{
    char   *i;

    for (i = c; *i; i++)
        netput (*i);
    return 1;
}

/* std_printf and cap_printf print a formatted string to stdout, exactly as
 * libc *printf.
 */
int
std_printf (const char *format, ...)
{
/* Know what sucks?  I can't really call cap_printf directly... */
    char    string[BUFSIZ];
    va_list ap;

    va_start (ap, format);
    (void) vsprintf (string, format, ap);
    va_end (ap);
    return std_puts (string);
}

int
cap_printf (const char *format, ...)
{
    char    string[BUFSIZ];
    va_list ap;

    if (capture) {
        va_start (ap, format);
        (void) vsprintf (string, format, ap);
        va_end (ap);
        return cap_puts (string);
    }
    return 1;
}

int
net_printf (const char *format, ...)
{
    va_list ap;
    static char work[BUFSIZ];

    va_start (ap, format);
    (void) vsprintf (work, format, ap);
    va_end (ap);
    net_puts (work);
    return 1;
}
