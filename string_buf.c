#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "string_buf.h"
// Note: the sizes are always the PUBLIC LOGICAL limits, when in fact
// we always allocate an extra byte for a \0 terminator behind the scenes.

#define ENDP(S) ((S)->data + (S)->len)  // ptr to logical end of string
#define NUL_TERMINATE(S) ((S)->data[(S)->len] = '\0')

static void
str_invariant_ (string * s)
{
    if (s) {
        if (s->data == NULL) {
            assert (s->len == 0);
            assert (s->cap == 0);
        }
        else {
            assert (s->len <= s->cap);
            assert (s->cap != 0);
            assert (s->data[s->len] == '\0');
        }
    }
}

string *
new_string ()
{
    return (string *) calloc (1, sizeof (string));
}

void
delete_string (string * s)
{
    if (s) {
        free (s->data);
        s->data = NULL;
        s->len = s->cap = 0;
    }
    free (s);
}

void
str_swap (string * p, string * q)
{
    string  t = *p;

    *p = *q;
    *q = t;
}

void
str_clear (string * s)
{
    if (s) {
        s->len = 0;
        if (s->data)
            NUL_TERMINATE (s);
    }
    str_invariant_ (s);
}

void
str_reserve (string * s, size_t n /*total */ )
{
    if (s)
        if (s && n > s->cap) {
            // select a growth factor. The current wisdom is to double each time.
            do
                s->cap = (s->cap + 1) * 2 - 1;
            while (s->cap < n);
            s->data = realloc (s->data, s->cap + 1);
            NUL_TERMINATE (s);
        }
    str_invariant_ (s);
}

void
str_pushr (string * s, const char *i, const char *E)
{
    if (s && i && E) {
        size_t  sz = E - i;

        str_reserve (s, s->len + sz);
        memcpy (ENDP (s), i, sz);
        s->len += sz;
        NUL_TERMINATE (s);
    }
    str_invariant_ (s);
}

void
str_pushs (string * s, const char *q)
{
    if (s && q)
        str_pushr (s, q, q + strlen (q));
    str_invariant_ (s);
}

void
str_pushc (string * s, int c)
{
    if (s) {
        str_reserve (s, s->len + 1);
        s->data[s->len++] = c;
        NUL_TERMINATE (s);
    }
    str_invariant_ (s);
}

/* vim:set expandtab cindent tabstop=4 softtabstop=4 shiftwidth=4 textwidth=0: */
