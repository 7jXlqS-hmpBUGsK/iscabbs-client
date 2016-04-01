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
discard_invalid_chars (char *s)
{
    // Discard all invalid chars.
    char   *d;

    for (d = s; *s; ++s)
        if (valid_post_char (*s))
            *d++ = *s;
    *d = 0;
}

static void
convert_newlines (char *s)
{
    // Replace \r or \r\n with \n
    char   *d;

    for (d = s; *s; ++s)
        if (*s == '\r' && s[1] == '\n') ;
        else if (*s == '\r')
            *d++ = '\n';
        else
            *d++ = *s;
    *d = 0;
}

// Attempt to map the source buffer to ASCII.
// Do not use src after this call, use the return value instead.
// returns the string, possibly re-alloc'd.
static char *
convert_to_ascii (char *src)
{
#ifdef HAVE_ICONV_H
    // We use iconv to convery to ASCII.
    // TODO: handle other encodings without going down the rabbit hole.
    iconv_t cd = iconv_open ("ASCII//TRANSLIT", "UTF-8");

    if (cd == (iconv_t) - 1)
        printf ("\r\nError: iconv_open failed.\r\n");
    else {
        const size_t len = strlen (src);
        size_t  cap = 16 + len / 3; // a small output buffer forces coverage of the loop below.
        char   *data = calloc (cap + 1, 1); // extra byte for \0 terminator.

        // These four vars are required and maintained by iconv().
        // There's no clean way to do this.
        const char *inbuf = src;
        char   *outbuf = data;
        size_t  inbytesleft = len;
        size_t  outbytesleft = cap;

        for (;;) {
            size_t  rc = iconv (cd, (char **) &inbuf, &inbytesleft, &outbuf, &outbytesleft);

            if (rc != (size_t) - 1 && inbytesleft == 0) {   // success.
                *outbuf = 0;
                // data contains the converted string.
                free (src);
                return data;
            }

            if (rc == (size_t) - 1 && errno == E2BIG) { // need to increase outbuf
                const size_t cursz = (outbuf - data);

                cap *= 2;
                data = realloc (data, cap + 1);
                outbuf = data + cursz;
                outbytesleft = data + cap - outbuf;
                continue;
            }

            // Otherwise, the conversion failed due to an invalid or incomplete multibyte sequence
            // or because we guessed the encoding wrong. Either way, we bail out.
            printf ("\r\nError: invalid or incomplete sequence in iconv()\r\n");
            free (data);
            data = NULL;
            break;
        }
        iconv_close (cd);
    }
#endif

    return src;
}

static char *
expand_tabs (char *const s)
{
    // Count tabs so we can allocate the destination buffer.
    size_t  t = 0, len = 0;

    for (char *p = s; *p; ++p, ++len)
        if (*p == '\t')
            ++t;

    // Over-allocate, at most 8 spaces per tab.
    char   *dest = calloc (1 + len + t * 8, sizeof (char));

    // expand tabs with spaces to the next 8-column position.
    size_t  col = 0;
    char   *d = dest;

    for (char *p = s; *p; ++p)
        if (*p == '\t') {
            for (short n = 8 - (col % 8); n != 0; --n, ++col)
                *d++ = ' ';
        }
        else if (*p == '\n') {
            *d++ = *p;
            col = 0;
        }
        else {
            *d++ = *p;
            ++col;
        }

    *d = 0;
    free (s);
    return dest;
}


// wrap long lines to the next line, indenting if possible.
static char *
wrap_long_lines (char *src)
{
    assert (src);

    // The destination buffer.
    size_t  dsz = 64;
    char   *dest = calloc (dsz, sizeof (char));
    char   *d = dest;

#define REQUIRE_DEST_BYTES(N) \
            while ((size_t)((d-dest)+(N)) > (dsz-1)){ \
                size_t curpos = d-dest; \
                dest = realloc (dest, dsz *= 2); \
                d = dest+curpos; \
            }

    // Note we use half-open ranges here.
    // The range from (I,E) includes I but not E, and is exactly (E-I) in length.
#define APPEND_DEST(I,E) \
            REQUIRE_DEST_BYTES(((E)-(I))) \
            memcpy(d,(I),((E)-(I))); \
            d += (E)-(I)

    char   *p0 = src;           // start of current line.
    int     col = 0;
    int     indent = 0;         // only used when wrapping.

    for (char *p = src; *p;) {
        if (*p == '\n') {
            ++p;
            // flush the line buffer in range (p0,p) half-open.
            APPEND_DEST (p0, p);
            // start fresh.
            p0 = p;
            col = indent = 0;
        }
        else if (col == 79) {
            // Break this long line.
            char   *b = p - 1;

            for (; b > p0; --b)
                if (!isalnum (*b)) {
                    ++b;
                    break;
                }
            if (b == p0)        // nowhere clean to break. Just break at p and be done with it.
                b = p;

            // we're about to flush (p0-p) as the current line.
            // Before we do, detect indentation if it's zero. Otherwise we use the previous value.
            if (indent == 0)
                while (p0[indent] == ' ')
                    ++indent;

            // if the indent is severe, then it's probably a paste mistake.
            if (indent >= 72)   // arbitrary limit
                indent = 0;

            // flush (p0,b)
            APPEND_DEST (p0, b);

            // flush a newline, plus indent number of spaces.
            REQUIRE_DEST_BYTES (indent + 1);
            *d++ = '\n';
            memset (d, ' ', indent);
            d += indent;

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
    *d = 0;;
    free (src);
    return dest;
}

// fix non-ASCII chars and wrap long lines.
void
autofix_posts (FILE * fp)
{
    rewind (fp);
    char   *buf = slurp_stream (fp);

    buf = convert_to_ascii (buf);
    convert_newlines (buf);
    discard_invalid_chars (buf);
    buf = expand_tabs (buf);
    buf = wrap_long_lines (buf);

    // Write the result.
    rewind (fp);
    fwrite (buf, 1, strlen (buf), fp);
    fflush (fp);
    ftruncate (fileno (fp), ftell (fp));
    free (buf);
}

/* vim:set expandtab cindent tabstop=4 softtabstop=4 shiftwidth=4 textwidth=0: */
