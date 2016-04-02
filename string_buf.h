#ifndef STRING_BUF_H
#define STRING_BUF_H
/* This is a simple growable string intended to replace 
 * ad-hoc string manipulation and fixed-size temp buffers.
 * The API is intended to resemble that of C++.
 * A non-NULL buffer is always \0-terminated.
 */
struct string;
typedef struct string string;
string *new_string (size_t cap);
void    delete_string (string *);
const char*    str_cdata (const string *);
char*    str_data (string *);
void    str_pushr (string *, const char *i, const char *E);
void    str_pushs (string *, const char *);
void    str_pushc (string *, int);
void    str_clear (string *);
void    str_reserve (string * s, size_t n /*total */ );
void    str_resize (string * s, size_t n /*total */ );
void    str_swap (string *, string *);
void    str_assign (string * overwrite, const string * src);
void    str_assignr (string * overwrite, const char* i, const char * E);
char*    str_begin (string *);
const char*    str_cbegin (const string *);
const char*    str_cend (const string *);
char*    str_end (string *);
size_t    str_length (const string *);
size_t    str_capacity (const string *);
void str_sprintf (string* overwrite, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
void str_getline (string* overwrite, FILE*);
bool str_chomp (string*);
bool str_empty (const string*);

#endif
/* vim:set expandtab cindent tabstop=4 softtabstop=4 shiftwidth=4 textwidth=0: */
