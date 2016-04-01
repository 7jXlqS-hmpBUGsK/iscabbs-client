#ifndef STRING_BUF_H
#define STRING_BUF_H
/* This is a simple growable string intended to replace 
 * ad-hoc string manipulation and fixed-size temp buffers.
 * The API is intended to resemble that of C++.
 * A non-NULL buffer is always \0-terminated.
 */
struct string {
    char   *data;
    size_t  len;                // current strlen()
    size_t  cap;                // max strlen() possible.
};
typedef struct string string;
string *new_string ();
void    delete_string (string *);
void    str_pushr (string *, const char *i, const char *E);
void    str_pushs (string *, const char *);
void    str_pushc (string *, int);
void    str_clear (string *);
void    str_reserve (string * s, size_t n /*total */ );
void    str_swap (string *, string *);
string *str_sprintf (const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

#endif
/* vim:set expandtab cindent tabstop=4 softtabstop=4 shiftwidth=4 textwidth=0: */
