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
    char   *p = s->data;
    char   *d;

    for (d = p; *p; ++p)
        if (valid_post_char (*p))
            *d++ = *p;
    s->len = d - s->data;
    s->data[s->len] = '\0';
}

static void
convert_newlines (string * s)
{
    // Replace \r or \r\n with \n
    char   *d;
    char   *p = s->data;

    for (d = p; *p; ++p)
        if (*p == '\r' && p[1] == '\n') ;
        else if (*p == '\r')
            *d++ = '\n';
        else
            *d++ = *p;
    s->len = d - s->data;
    s->data[s->len] = '\0';
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
        // We convert from buf to out.
        string *out = new_string ();

        str_reserve (out, buf->len / 3 + 16);   // small buffer provides test coverage of loop below.

        // These four vars are required and maintained by iconv().
        // There's no clean way to do this. the iconv API just plain sucks.
        const char *inbuf = buf->data;
        char   *outbuf = out->data;
        size_t  inbytesleft = buf->len;
        size_t  outbytesleft = out->cap;

        for (;;) {
            size_t  rc = iconv (cd, (char **) &inbuf, &inbytesleft, &outbuf, &outbytesleft);

            // whatever happened, keep the string buffer in sync.
            out->len = outbuf - out->data;
            out->data[out->len] = 0;

            if (rc != (size_t) - 1 && inbytesleft == 0) {   // success.
                // swap out and buf.
                str_swap (buf, out);
                break;
            }

            if (rc == (size_t) - 1 && errno == E2BIG) { // need to increase outbuf
                str_reserve (out, out->len + inbytesleft);
                outbuf = out->data + out->len;
                outbytesleft = out->cap - out->len;
                continue;
            }

            // Otherwise, the conversion failed due to an invalid or incomplete multibyte sequence
            // or because we guessed the encoding wrong. Either way, we bail out.
            printf ("\r\nError: invalid or incomplete sequence in iconv()\r\n");
            break;
        }
        delete_string (out);
        iconv_close (cd);
    }
#endif
}

static void
expand_tabs (string * s)
{
    // Count tabs so we can allocate the destination buffer.
    size_t  t = 0, len = 0;

    for (char *p = s->data; *p; ++p, ++len)
        if (*p == '\t')
            ++t;

    // Over-allocate, at most 8 spaces per tab.
    string *out = new_string ();

    str_reserve (out, s->len + t * 8);

    // expand tabs with spaces to the next 8-column position.
    size_t  col = 0;

    for (const char *p = s->data; *p; ++p)
        if (*p == '\t') {
            for (short n = 8 - (col % 8); n != 0; --n, ++col)
                str_pushc (out, ' ');
        }
        else if (*p == '\n') {
            str_pushc (out, '\n');
            col = 0;
        }
        else {
            str_pushc (out, *p);
            ++col;
        }

    str_swap (s, out);
    delete_string (out);
}


// wrap long lines to the next line, indenting if possible.
static void
wrap_long_lines (string * src)
{
    assert (src);

    // The destination buffer.
    string *out = new_string ();

    const char *p0 = src->data; // start of current line.
    int     col = 0;
    int     indent = 0;         // only used when wrapping.

    for (const char *p = src->data; *p;) {
        if (*p == '\n') {
            ++p;
            // flush the line buffer in range (p0,p) half-open.
            str_pushr (out, p0, p);
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
            str_pushr (out, p0, b);

            // append a newline and indent number of spaces followed
            str_pushc (out, '\n');
            for (int i = 0; i != indent; ++i)
                str_pushc (out, ' ');

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
    str_swap (out, src);
    delete_string (out);
}

// fix non-ASCII chars and wrap long lines.
void
autofix_posts (FILE * fp)
{
    rewind (fp);
    string *buf = new_string ();

    slurp_stream (fp, buf);

    if (buf->len) {
        convert_to_ascii (buf);
        convert_newlines (buf);
        discard_invalid_chars (buf);
        expand_tabs (buf);
        wrap_long_lines (buf);

        // Write the result.
        rewind (fp);
        fputs (buf->data, fp);
        fflush (fp);
        ftruncate (fileno (fp), ftell (fp));
    }
    delete_string (buf);
}

/* vim:set expandtab cindent tabstop=4 softtabstop=4 shiftwidth=4 textwidth=0: */
