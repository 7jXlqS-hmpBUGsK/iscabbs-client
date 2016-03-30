/* System I/O routines.
 */

#include "defs.h"
#include "ext.h"
#include <stdarg.h>
static int netilen;             /* length of current input buffer from net */
static int ptyilen;             /* length of current input buffer from pty */

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

/* std_putchar() and cap_putchar() write a single character to stdout and the
 * capture file, respectively.  On error, they terminate the client.
 */
void
std_putchar (int c)
{
    if (putchar (c) < 0)
        fatalperror ("std_putchar", "Local error");
    cap_putchar (c);
}

void
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
}

int
netflush (void)
{
    return fflush (netofp);
}

void
net_putchar (int c)
{
    fputc (c, netofp);
}

/* stripansi removes ANSI escape sequences from a string.  */
static void
stripansi (char *p)
{
    char   *q = p;

    while (*p)
        if (*p == '\033')
            for (++p; *p && !isalpha (*p); ++p) ;
        else
            *q++ = *p++;

    *q = '\0';
}

void
cap_puts (char *c)
{
    if (capture > 0 && !flags.posting && !flags.moreflag) {
        stripansi (c);
        fprintf (tempfile, "%s", c);
        fflush (tempfile);
    }
}

/* std_printf and cap_printf print a formatted string to stdout, exactly as
 * libc *printf.
 */
void
std_printf (const char *format, ...)
{
    static char string[BUFSIZ];
    va_list ap;

    va_start (ap, format);
    vsnprintf (string, BUFSIZ, format, ap);
    va_end (ap);
    fputs (string, stdout);
    fflush (stdout);
    cap_puts (string);
}
