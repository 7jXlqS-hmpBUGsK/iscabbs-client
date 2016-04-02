#include "defs.h"
#include "ext.h"

#ifdef HAVE_ICONV_H
#include <iconv.h>
#endif

bool
valid_post_char (int c)
{
    return c == '\r' || c == '\n' || c == '\t' || (c >= 32 && c < DEL);
}

static void
discard_invalid_chars (string * s)
{
    // Discard all invalid chars.
    char   *p = str_data(s);
    char   *d;

    for (d = p; *p; ++p)
        if (valid_post_char (*p))
            *d++ = *p;
    *d = '\0';
    str_resize (s,d - str_data(s));
}

static void
convert_newlines (string * s)
{
    // Replace \r or \r\n with \n
    char   *d;
    char   *p = str_data (s);

    for (d = p; *p; ++p)
        if (*p == '\r' && p[1] == '\n') ;
        else if (*p == '\r')
            *d++ = '\n';
        else
            *d++ = *p;
    *d = '\0';
    str_resize (s, d - str_data (s));
}

// Attempt to map the source buffer to ASCII.
// Do not use src after this call, use the return value instead.
// returns the string, possibly re-alloc'd.
static void
convert_to_ascii (string * buf)
{
#ifdef HAVE_ICONV_H
    // We use iconv to convery to ASCII.
    // TODO: handle other encodings without going down the rabbit hole.
    iconv_t cd = iconv_open ("ASCII//TRANSLIT", "UTF-8");

    if (cd == (iconv_t) - 1)
        printf ("\r\nError: iconv_open failed.\r\n");
    else {
        // We convert from buf to scratch.
        str_clear (scratch);
        str_reserve (scratch, (str_length(buf) / 3 + 16)); 

        // These four vars are required and maintained by iconv().
        // There's no clean way to do this. the iconv API just plain sucks.
        const char *inbuf = str_data (buf);
        char   *outbuf = str_data (scratch);
        size_t  inbytesleft = str_length(buf);
        size_t  outbytesleft = str_capacity (scratch);

        for (;;) {
            size_t  rc = iconv (cd, (char **) &inbuf, &inbytesleft, &outbuf, &outbytesleft);

            // whatever happened, keep the string buffer in sync.
            *outbuf = '\0';

            if (rc != (size_t) - 1 && inbytesleft == 0) {   // success.
                str_assignr (buf, str_cdata(scratch), outbuf);
                break;
            }

            if (rc == (size_t) - 1 && errno == E2BIG) { // need to increase outbuf
                size_t got = outbuf - str_data(scratch);
                str_reserve (scratch, str_capacity (scratch) + inbytesleft);
                outbuf = str_data (scratch) + got;
                outbytesleft = str_capacity (scratch) - got;
                continue;
            }

            // Otherwise, the conversion failed due to an invalid or incomplete multibyte sequence
            // or because we guessed the encoding wrong. Either way, we bail out.
            printf ("\r\nError: invalid or incomplete sequence in iconv()\r\n");
            break;
        }
        iconv_close (cd);
    }
#endif
}

static void
expand_tabs (string * s)
{
    // Count tabs so we can allocate the destination buffer.
    size_t  t = 0, len = 0;

    for (char *p = str_data (s); *p; ++p, ++len)
        if (*p == '\t')
            ++t;

    // Over-allocate, at most 8 spaces per tab.
    str_clear (scratch);
    str_reserve (scratch, str_length(s) + t * 8);

    // expand tabs with spaces to the next 8-column position.
    size_t  col = 0;

    for (const char *p = str_cdata (s); *p; ++p)
        if (*p == '\t') {
            for (short n = 8 - (col % 8); n != 0; --n, ++col)
                str_pushc (scratch, ' ');
        }
        else if (*p == '\n') {
            str_pushc (scratch, '\n');
            col = 0;
        }
        else {
            str_pushc (scratch, *p);
            ++col;
        }

    str_assign (s, scratch);
}


// wrap long lines to the next line, indenting if possible.
static void
wrap_long_lines (string * src)
{
    assert (src);

    // The destination buffer.
    str_clear (scratch);

    const char *p0 = str_cdata (src); // start of current line.
    int     col = 0;
    int     indent = 0;         // only used when wrapping.

    for (const char *p = str_cdata (src); *p;) {
        if (*p == '\n') {
            ++p;
            // flush the line buffer in range (p0,p) half-open.
            str_pushr (scratch, p0, p);
            // start fresh.
            p0 = p;
            col = indent = 0;
        }
        else if (col == 79) {
            // Break this long line.
            const char *b = p - 1;

            // scan backward for a boundary
            for (; b > p0; --b)
                if (!isalnum (*b)) {
                    ++b;
                    break;
                }
            if (b == p0)        // nowhere clean to break. Just break at p and be done with it.
                b = p;

            // we're about to flush (p0,p) as the current line.
            // Before we do, detect indentation if it's zero. Otherwise we use the previous value.
            if (indent == 0)
                while (p0[indent] == ' ')
                    ++indent;

            // if the indent is severe, then it's probably a paste mistake.
            if (indent >= 72)   // arbitrary limit
                indent = 0;

            // flush (p0,b)
            str_pushr (scratch, p0, b);

            // append a newline and indent number of spaces followed
            str_pushc (scratch, '\n');
            for (int i = 0; i != indent; ++i)
                str_pushc (scratch, ' ');

            // start a new line with logical indentation.
            col = indent;
            p0 = p = b;

            // Skip leading whitespace after an indent.
            while (*p == ' ')
                ++p, ++p0;
        }
        else {
            ++p;
            ++col;
        }
    }
    str_assign (src, scratch);
}

// fix non-ASCII chars and wrap long lines.
void
autofix_posts (FILE * fp)
{
    rewind (fp);
    string *buf = new_string (0);

    slurp_stream (fp, buf);

    if (!str_empty(buf)) {
        convert_to_ascii (buf);
        convert_newlines (buf);
        discard_invalid_chars (buf);
        expand_tabs (buf);
        wrap_long_lines (buf);

        // Write the result.
        rewind (fp);
        fputs (str_cdata (buf), fp);
        fflush (fp);
        ftruncate (fileno (fp), ftell (fp));
    }
    delete_string (buf);
}

/* vim:set expandtab cindent tabstop=4 softtabstop=4 shiftwidth=4 textwidth=0: */
