/*
 *  This is the home of function prototypes for all the global functions.
 */

#ifndef PROTO_H_INCLUDED
#define PROTO_H_INCLUDED

extern void ansi_transform_express (char *);
extern void ansi_transform_posthdr (char *, int);
extern void arguments (int, char **);
extern void configbbsrc (void);
extern void color_config (void);
extern void connectbbs (void);
extern void continued_post_helper (void);
extern void deinitialize (void);
extern void default_colors (int);
extern void fatalexit (const char *, const char *);
extern void fatalperror (const char *, const char *);
extern void feed_pager (int, ...);
extern void filter_express (int);
extern void filter_data (int);
extern void filter_post (int);
extern void filter_wholist (int);
extern void findhome (void);
extern void flush_input (unsigned int);
extern void get_five_lines (int);
extern void get_string (int, char *, int);
extern void information (void);
extern void initialize (const char *);
extern void killSSL (void);
extern void looper (void);
extern void makemessage (int);
extern void moreprompt_helper (void);
extern void myexit (void);
extern void mysleep (unsigned int);
extern void open_browser (void);
extern void opentmpfile (void);
extern void readbbsrc (void);
extern void replymessage (void);
extern void reprint_line (void);
extern void resetterm (void);
extern void run (char *, char *);
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
extern void slistDestroy (slist *);
extern void slistDestroyItems (slist *);
extern void slistSort (slist *);
extern void s_perror (const char *, const char *);
extern void suspend (void);
extern void techinfo (void);
extern void telinit (void);
extern void tempfileerror (void);
extern void truncbbsrc (int);
extern void writebbsrc (void);

extern int binary_search (char *);
extern int binary_sort (void);
extern int cap_putchar (int);
extern void cap_puts (char *);
extern int colorize (const char *);
extern int deletefile (const char *);
extern int ExtractNumber (const char *);
extern int getkey (void);
extern int getwindowsize (void);
extern int inkey (void);
extern int is_queued (char *, queue *);
extern int more (int *, int);
extern void net_putchar (int);
extern int pop_queue (char *, queue *);
extern int push_queue (char *, queue *);
extern int slistAddItem (slist *, void *, int);
extern int slistFind (slist *, void *, int (*findfn) (const void *, const void *));
extern int slistRemoveItem (slist *, int);
extern int s_prompt (const char *, const char *, int);
extern void std_printf (const char *, ...) __attribute__ ((format (printf, 1, 2)));
extern void std_putchar (int);
extern int telrcv (int);
extern int waitnextevent (void);
extern int yesno (void);
extern int yesnodefault (int);

extern char ansi_transform (char);
extern char ansi_transform_post (char, int);
extern char *ExtractName (char *);
extern char general_color_menu (void);
extern char *get_name (int);

extern FILE *findbbsrc (void);
extern FILE *openbbsrc (void);
extern FILE *openbbsfriends (void);
extern FILE *findbbsfriends (void);

extern queue *new_queue (int, int);

extern slist *slistCreate (int (*sortfn) (const void *, const void *));
extern int fstrcmp (const char *a, const Friend * b);
extern int fsortcmp (const Friend * const *a, const Friend * const *b);
extern int sortcmp (const char *const *a, const char *const *b);

extern char *jhpdecode (char *dest, const char *src, size_t len);
extern char *jhpencode (char *dest, const char *src, size_t len);

extern bool INPUT_LEFT (void);
extern int ptyget (void);
extern bool NET_INPUT_LEFT (void);
extern int netget (void);
extern int netflush (void);
#endif /* PROTO_H_INCLUDED */
