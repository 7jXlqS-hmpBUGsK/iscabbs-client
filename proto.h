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
extern void color_options (void);
extern void connectbbs (void);
extern void continued_data_helper (void);
extern void continued_post_helper (void);
extern void copyright (void);
extern void deinitialize (void);
extern void default_colors (int);
extern void editusers (slist *, int (*findfn) (const void *, const void *), const char *);
extern void express_color_config (void);
extern void express_config (void);
extern void express_friend_color_config (void);
extern void express_user_color_config (void);
extern void fatalexit (const char *, const char *);
extern void fatalperror (const char *, const char *);
extern void feed_pager (int, ...);
extern void filter_express (int);
extern void filter_data (int);
extern void filter_post (int);
extern void filter_url (char *);
extern void filter_wholist (int);
extern void findhome (void);
extern void flush_input (unsigned int);
extern void general_color_config (void);
extern void get_five_lines (int);
extern void get_string (int, char *, int);
extern void information (void);
extern void initialize (const char *);
extern void input_color_config (void);
extern void killSSL (void);
extern void license (void);
extern void looper (void);
extern void makemessage (int);
extern void moreprompt_helper (void);
extern void move_if_needed (const char *, const char *);
extern void myexit (void);
extern void mysleep (unsigned int);
extern void newawaymsg (void);
extern void newmacro (int);
extern void notitlebar (void);
extern void not_replying_transform_express (char *);
extern void open_browser (void);
extern void opentmpfile (void);
extern void otherinfo (void);
extern void post_color_config (void);
extern void post_friend_color_config (void);
extern void post_user_color_config (void);
extern void readbbsrc (void);
extern void replycode_transform_express (char *);
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
extern void smarterase (char *);
extern void smartprint (char *, char *);
extern void s_perror (const char *, const char *);
extern void suspend (void);
extern void techinfo (void);
extern void telinit (void);
extern void tempfileerror (void);
extern void titlebar (void);
extern void truncbbsrc (int);
extern void warranty (void);
extern void writebbsrc (void);

extern int binary_search (char *);
extern int binary_sort (void);
extern int cap_printf (const char *, ...);
extern int cap_putchar (int);
extern int cap_puts (char *);
extern int checkfile (FILE *);
extern int colorize (const char *);
extern int delete_queue (queue *);
extern int deletefile (const char *);
extern int ExtractNumber (char *);
extern int fsortcmp (Friend **, Friend **);
extern int fstrcmp (char *, Friend *);
extern int getkey (void);
extern int getwindowsize (void);
extern int inkey (void);
extern int is_automatic_reply (const char *);
extern int is_queued (char *, queue *);
extern int more (int *, int);
extern int net_printf (const char *, ...);
extern int net_putchar (int);
extern int net_puts (char *);
extern int newkey (int);
extern int pop_queue (char *, queue *);
extern int prompt (FILE *, int *, int);
extern int push_queue (char *, queue *);
extern int safe_delete_queue (queue *);
extern int slistAddItem (slist *, void *, int);
extern int slistFind (slist *, void *, int (*findfn) (const void *, const void *));
extern int slistRemoveItem (slist *, int);
extern int smartname (char *, char *);
extern int sortcmp (char **, char **);
extern int s_prompt (const char *, const char *, int);
extern int std_printf (const char *, ...);
extern int std_putchar (int);
extern int std_puts (char *);
extern int telrcv (int);
extern int waitnextevent (void);
extern int yesno (void);
extern int yesnodefault (int);

extern char ansi_transform (char);
extern char ansi_transform_post (char, int);
extern char background_picker (void);
extern char color_picker (void);
extern char express_color_menu (void);
extern char *ExtractName (char *);
extern char general_color_menu (void);
extern char *get_name (int);
extern char post_color_menu (void);
extern char *strctrl (int);
extern char *stripansi (char *);
extern char user_or_friend (void);

extern FILE *findbbsrc (void);
extern FILE *openbbsrc (void);
extern FILE *openbbsfriends (void);
extern FILE *findbbsfriends (void);

extern queue *new_queue (int, int);

extern slist *slistCreate (int, int (*sortfn) (const void *, const void *), ...);
extern slist *slistIntersection (const slist *, const slist *);

extern void bye (int);
extern void naws (int);
extern void reapchild (int);

#endif /* PROTO_H_INCLUDED */
