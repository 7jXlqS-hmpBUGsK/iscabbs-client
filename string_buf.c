#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <assert.h>
#include "string_buf.h"
// Note: the sizes are always the LOGICAL limits, when in fact
// we always allocate an extra byte for a \0 terminator behind the scenes.
struct string {
    char   *data;               // never NULL.
    size_t  len;                // current strlen()
    size_t  cap;                // max strlen() possible.
};

#define ENDP(S) ((S)->data + (S)->len)  // ptr to logical end of string
#define BEGINP(S) ((S)->data)
#define AVAIL(S) ((S)->cap - (S)->len)
#define NUL_TERMINATE(S) (*ENDP(S) = '\0')
#define MIN_CAP 16              // arbitrary minimum. any positive value will do

const char *
str_cdata (const string * s)
{
    assert (s);
    return BEGINP (s);
}

const char *
str_cend (const string * s)
{
    assert (s);
    return ENDP (s);
}

char   *
str_data (string * s)
{
    return (char *) str_cdata (s);
}

char   *
str_begin (string * s)
{
    return str_data (s);
}

const char *
str_cbegin (const string * s)
{
    return str_cdata (s);
}

char   *
str_end (string * s)
{
    return (char *) str_cend (s);
}

size_t
str_length (const string * s)
{
    assert (s);
    return s->len;
}

size_t
str_capacity (const string * s)
{
    assert (s);
    return s->cap;
}

static void
str_invariant_ (string * s)
{
    assert (s);
    assert (s->data);
    assert (s->len <= s->cap);
    assert (s->cap >= MIN_CAP);
    assert (s->data[s->len] == '\0');
}

string *
new_string (size_t initial_capacity)
{
    string *s = (string *) calloc (1, sizeof (string));

    if (initial_capacity < MIN_CAP)
        initial_capacity = MIN_CAP;
    s->data = calloc (1, initial_capacity + 1);
    s->cap = initial_capacity;
    return s;
}

string *
new_strings (const char* s)
{
    string * d = new_string (s ? strlen(s) : 0);
    str_assigns (d,s);
    return d;
}

void
delete_string (string * s)
{
    if (s) {
        free (s->data);
        free (s);
    }
}

void
str_swap (string * p, string * q)
{
    assert (p);
    assert (q);
    string  t = *p;

    *p = *q;
    *q = t;
}

void
str_clear (string * s)
{
    str_resize (s, 0);
}

void
str_reserve (string * s, size_t n /*total */ )
{
    assert (s);
    if (n > s->cap) {
        // select a growth factor. The current wisdom is to double each time.
        do
            s->cap = (s->cap + 1) * 2 - 1;
        while (s->cap < n);
        s->data = realloc (s->data, s->cap + 1);
        NUL_TERMINATE (s);
        str_invariant_ (s);
    }
}

void
str_resize (string * s, size_t n /*total */ )
{
    assert (s);
    if (n > s->cap)
        str_reserve (s, n);

    if (n > s->len)
        // fill with \0.
        memset (ENDP (s), 0, n - s->len);

    s->len = n;
    NUL_TERMINATE (s);
    str_invariant_ (s);
}

void
str_pushr (string * s, const char *i, const char *E)
{
    assert (s);
    assert (i);
    assert (E);
    const size_t sz = E - i;

    str_resize (s, str_length (s) + sz);
    memcpy (ENDP (s) - sz, i, sz);
    str_invariant_ (s);
}

void
str_pushs (string * s, const char *q)
{
    assert (s);
    assert (q);
    str_pushr (s, q, q + strlen (q));
    str_invariant_ (s);
}

void
str_pushc (string * s, int c)
{
    assert (s);
    str_reserve (s, s->len + 1);
    s->data[s->len++] = c;
    NUL_TERMINATE (s);
    str_invariant_ (s);
}

void
str_sprintf (string * out, const char *fmt, ...)
{
    assert (out);
    assert (fmt);
    va_list ap;

    // Take a guess at how much space we need.
    str_clear (out);
    str_reserve (out, strlen (fmt) * 2);

    va_start (ap, fmt);
    int     N = vsnprintf (out->data, out->cap, fmt, ap);

    va_end (ap);
    if (N < 0)
        return;

    if ((size_t) N > out->cap) {
        // try again
        str_reserve (out, out->len + N);
        va_start (ap, fmt);
        N = vsnprintf (out->data, out->cap, fmt, ap);
        va_end (ap);
        if (N < 0)
            return;
    }

    assert (N >= 0);
    if ((size_t) N <= out->cap) {   // Success
        out->len = (size_t) N;
        NUL_TERMINATE (out);
    }
}

void
str_assign (string * dest, const string * src)
{
    assert (dest);
    assert (src);
    str_clear (dest);
    str_pushr (dest, BEGINP (src), ENDP (src));
}

void
str_assignr (string * s, const char *i, const char *E)
{
    assert (s);
    assert (i);
    assert (E);
    str_clear (s);
    str_pushr (s, i, E);
}

void
str_assigns (string * s, const char * src)
{
    if (s && src)
        str_assignr (s, src, src + strlen(src));
}

bool
str_getline (string * out, FILE * fp)
{
    assert (out);
    assert (fp);
    str_clear (out);
    str_reserve (out, 32);      // arbitrary. small buffer covers the loop below. caller can reserve more if they need more.

    // Continue to append until we get a newline or eof.
    // Note: fgets() guarantees a \0-terminator.
    bool    success = false;

    while (fgets (ENDP (out), AVAIL (out) + 1, fp)) {
        success = true;
        out->len = strlen (BEGINP (out));
        if (str_chomp (out))    // successfully got a newline
            break;
        else                    // there was no newline. Our buffer was too small. Grow and read more.
            str_reserve (out, out->cap * 2);
    }
    str_invariant_ (out);
    return success;
}

char
str_back (const string * s)
{
    assert (s);
    assert (s->len);
    return s->data[s->len - 1];
}

void
str_pop_back (string * s)
{
    assert (s);
    assert (s->len);
    --s->len;
    NUL_TERMINATE (s);
}

bool
str_empty (const string * s)
{
    return str_length (s) == 0;
}

bool
str_chomp (string * s)
{
    assert (s);
    const size_t n = str_length (s);

    while (!str_empty (s) && (str_back (s) == '\n' || str_back (s) == '\r'))
        str_pop_back (s);
    return str_length (s) != n; // if the length changed, we chomped
}

bool 
str_eqs (const string* a, const char* b)
{
    if (!b)
        b = "";
    return 0==strcmp(a ? "" : str_cdata(a),b);
}

/* vim:set expandtab cindent tabstop=4 softtabstop=4 shiftwidth=4 textwidth=0: */
