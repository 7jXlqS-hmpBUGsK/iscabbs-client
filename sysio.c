/* System I/O routines.
 */

#include "defs.h"
#include "ext.h"
#include <stdarg.h>

// TODO: There was a note in the old code about these buffers being required due to VMS.
//       Since we no longer support VMS, can we use system network buffering?
static int netilen;             /* length of current input buffer from net */
static int ptyilen;             /* length of current input buffer from pty */
static unsigned char netibuf[2048]; /* buffer for input from net */
static unsigned char *netifp = netibuf; /* buffer pointer for input from net */
static unsigned char ptyibuf[1024]; /* buffer for input from pty */
static unsigned char *ptyifp = ptyibuf; /* buffer pointer for input from pty */

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

/** write a byte */
void
net_putchar_unsyncd (int c)
{
    fputc (c, netofp);
}

/** write a byte and update the sync_byte counter */
void
net_putchar (int c)
{
    net_putchar_unsyncd (c);
    ++sync_byte;
}

/** write a byte and update the sync_byte counter */
void
net_putbytes_unsyncd (const char *s, size_t n)
{
    if (s)
        fwrite (s, 1, n, netofp);
}

static void
net_putbytes (const char *s, size_t n)
{
    if (s) {
        fwrite (s, 1, n, netofp);
        sync_byte += n;
    }
}

void
net_putstring (const char *s)
{
    net_putbytes (s, strlen (s));
}

/* stripansi removes ANSI escape sequences from a string.  */
static void
stripansi (const char *p, FILE * out)
{
    while (*p)
        if (*p == '\033')
            for (++p; *p && !isalpha (*p); ++p) ;
        else
            fputc (*p++, out);
}

void
cap_puts (const char *c)
{
    if (capture > 0 && !flags.posting && !flags.moreflag) {
        stripansi (c, tempfile);
        fflush (tempfile);
    }
}

/* std_printf and cap_printf print a formatted string to stdout, exactly as
 * libc *printf.
 */
void
std_printf (const char *format, ...)
{
    str_reserve (scratch, BUFSIZ);
    const size_t sz = str_capacity (scratch);
    va_list ap;

    va_start (ap, format);
    vsnprintf (str_data (scratch), sz, format, ap);
    va_end (ap);
    fputs (str_data (scratch), stdout);
    fflush (stdout);
    cap_puts (str_cdata (scratch));
}
