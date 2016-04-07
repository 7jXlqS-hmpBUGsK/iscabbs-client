/*
 *  This is the home of function prototypes for all the global functions.
 */

#ifndef PROTO_H_INCLUDED
#define PROTO_H_INCLUDED

extern void ansi_transform_express (char *);
extern void ansi_transform_posthdr (char *, bool);
extern void arguments (int, char **);
extern void configbbsrc (void);
extern void color_config (void);
extern void connectbbs (void);
extern void continued_post_helper (void);
extern void deinitialize (void);
extern void default_colors (bool);
extern void fatalexit (const char *, const char *) __attribute__ ((noreturn));
extern void fatalperror (const char *, const char *) __attribute__ ((noreturn));
extern void filter_express (int);
extern void filter_data (int);
extern void filter_post (int);
extern void filter_wholist (int);
extern void findhome (void);
extern void flush_input (unsigned int);
extern void get_five_lines (int);
extern void get_string (int, char *, int);
extern void information (void);
extern void initialize (void);
extern void killSSL (void);
extern void looper (void);
extern void makemessage (int);
extern void moreprompt_helper (void);
extern void myexit (void) __attribute__ ((noreturn));
extern void mysleep (unsigned int);
extern void open_browser (void);
extern void opentmpfile (void);
extern void readbbsrc (void);
extern void replymessage (void);
extern void reprint_line (void);
extern void resetterm (void);
extern void run (const char *, const char *);
extern void savebbsrc (void);
extern void send_an_x (void);
extern void sendblock (void);
extern void sendnaws (void);
extern void setterm (void);
extern void s_error (const char *, const char *);
extern void setup (int);
extern void siginit (void);
extern void sigoff (void);
extern void s_info (const char *, const char *);
extern void s_perror (const char *, const char *);
extern void suspend (void);
extern void telinit (void);
extern void tempfileerror (void);
extern void truncfp (FILE *, long);
extern void writebbsrc (FILE *);

extern void cap_putchar (int);
extern void cap_puts (const char *);
extern void colorize (const char *);
extern int ExtractNumber (const char *);
extern int getkey (void);
extern int getwindowsize (void);
extern int inkey (void);
extern int more (int *, int);
extern void net_putchar (int);
extern void net_putstring (const char *p);
extern void net_putchar_unsyncd (int);
extern void net_putbytes_unsyncd (const char *p, size_t n);
extern int s_prompt (const char *, const char *, int);
extern void std_printf (const char *, ...) __attribute__ ((format (printf, 1, 2)));
extern void std_putchar (int);
extern int telrcv (int);
extern int waitnextevent (void);
extern bool yesno (void);
extern bool yesnodefault (bool);

extern char ansi_transform (char);
extern char ansi_transform_post (char, bool);
extern char *ExtractName (const char *);
extern char general_color_menu (void);
extern char *get_name (int);

extern FILE *findbbsrc (void);
extern FILE *openbbsrc (void);
extern FILE *openbbsfriends (void);
extern FILE *findbbsfriends (void);

struct queue;
typedef struct queue queue;
extern queue *new_queue (size_t max_count);
extern bool is_queued (const char *, const queue *);
extern bool pop_queue (char *, queue *);
extern bool push_queue (const char *, queue *);
extern size_t queue_size (const queue *);
extern const char *queue_at (const queue * q, size_t i);
extern void delete_queue (queue * q);

struct UserEntry {
    char    name[21];           /* User name */
    time_t  login_tm;           /* Only valid when in whoList */
    char    info[54];           /* friend description only valid for friends. */
    bool    xmsg_disabled;      /* Only valid when in wholList */
};
typedef struct UserEntry UserEntry;

struct UList {
    size_t  sz;                 /* number of items in list */
    UserEntry **arr;            /* dynamic array containing item pointers */
};
typedef struct UList UList;
extern UserEntry *new_UserEntry (const char *name);
extern UserEntry *ulist_insert (UList *, const char *name);
extern UserEntry *ulist_find (UList *, const char *name);
extern bool ulist_erase (UList *, const char *name);
extern void ulist_clear (UList *);
extern void ulist_sort_by_name (UList *);
extern void ulist_sort_by_time (UList *);

extern void jhpdecode (char *dest, const char *src, size_t len);
extern void jhpencode (char *dest, const char *src, size_t len);

extern bool INPUT_LEFT (void);
extern int ptyget (void);
extern bool NET_INPUT_LEFT (void);
extern int netget (void);
extern int netflush (void);
extern void slurp_stream (FILE * in, string * out);
extern bool valid_post_char (int c);
void    autofix_posts (FILE * fp);
void    putnchars (int ch, size_t N);
void    rtrim (char *);
#endif /* PROTO_H_INCLUDED */
