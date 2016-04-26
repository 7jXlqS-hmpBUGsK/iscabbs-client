#ifndef STRING_BUF_H
#define STRING_BUF_H

/* This is a growable string intended to replace ad-hoc char* manipulation and
 * provide an alternative to fixed-size temp buffers.
 *
 * The API is closely resembles that of C++.  In particular, the buffer is
 * never NULL, the string is always \0-terminated, and ranges are half-open
 * (the beginning is inclusive, the end is outside the range.) You can access
 * the raw buffer directly via str_data() and drop into C.
 *
 * If you plan to modify the char buffer directly, then you simply need to call
 * str_resize() before and/or after your modifications.
 *
 *   str_resize (buf, TOTAL_LEN_POSSIBLE); // Only necessary if length may increase.  
 *   char * d = str_data (buf);    // Get direct access to buffer.
 *   modify_buffer (d);
 *   str_resize (buf, new_length); // Only necessary if length changed.
 *
 * Note the difference between str_reserve() and str_resize().  The purpose of
 * str_reserve() is to allocate now, avoiding future re-allocations.  The
 * purpose of str_resize() is to alter the length of the logical C-string.
 */

struct string;
typedef struct string string;

string *new_string (size_t initial_capacity);   // allocate a string
string *new_strings (const char*);   // allocate a copy of s
void    delete_string (string *);   // free resources. param may be null.
size_t  str_length (const string *);    // logical length not including \0-terminator
size_t  str_capacity (const string *);  // max length possible without re-allocation
bool    str_empty (const string *); // true if length()==0
char   *str_data (string *);    // start of buffer
char   *str_begin (string *);   // same as data()
char   *str_end (string *);     // one past-end (points to '\0')
const char *str_cdata (const string *); // const version of data()
const char *str_cbegin (const string *);    // const version of begin()
const char *str_cend (const string *);  // const version of end()
void    str_pushr (string *, const char *i, const char *E); // append range from i to E
void    str_pushs (string *, const char *); // append C string
void    str_pushc (string *, int);  // append char
void    str_clear (string *);   // set length to zero, no re-allocation performed.
void    str_reserve (string * s, size_t total_capacity);    // reallocate only to grow. no-op otherwise.
void    str_resize (string * s, size_t total_size); // truncate or grow length, appending '\0' chars as necessary.
void    str_swap (string *, string *);  // swap contents (exchange internal pointers)
void    str_assignr (string * overwrite, const char *i, const char *E); // copy the given range
void    str_assign (string * overwrite, const string * src);    // copy src
void    str_assigns (string * overwrite, const char * src);    // copy src
void    str_sprintf (string * overwrite, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
bool    str_getline (string * overwrite, FILE *);   // chomp newlines. return false on EOF or error.
bool    str_chomp (string *);   // trim all trailing \r and \n chars

bool str_eqs (const string*, const char*);

#endif
/* vim:set expandtab cindent tabstop=4 softtabstop=4 shiftwidth=4 textwidth=0: */
