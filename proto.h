/*
 *  This is the home of function prototypes for all the global functions.
 */

#ifndef PROTO_H_INCLUDED
#define PROTO_H_INCLUDED

#ifdef STDC_HEADERS
#define P(x) x
#else
#error Sorry, an ANSI C compiler is required now.
#endif

extern void
ansi_transform_express P ((char *)),
    ansi_transform_posthdr P ((char *, int)),
    arguments P ((int, char **)),
    configbbsrc P ((void)),
    color_config P ((void)),
    color_options P ((void)),
    connectbbs P ((void)),
    continued_data_helper P ((void)),
    continued_post_helper P ((void)),
    copyright P ((void)),
    deinitialize P ((void)),
    default_colors P ((int)),
    editusers P ((slist *, int (*findfn) (const void *, const void *), const char *)),
    express_color_config P ((void)),
    express_config P ((void)),
    express_friend_color_config P ((void)),
    express_user_color_config P ((void)),
    fatalexit P ((const char *, const char *)),
    fatalperror P ((const char *, const char *)),
    feed_pager P ((int, ...)),
    filter_express P ((int)),
    filter_data P ((int)),
    filter_post P ((int)),
    filter_url P ((char *)),
    filter_wholist P ((int)),
    findhome P ((void)),
    flush_input P ((unsigned int)),
    general_color_config P ((void)),
    get_five_lines P ((int)),
    get_string P ((int, char *, int)),
    information P ((void)),
    initialize P ((const char *)),
    input_color_config P ((void)),
    killSSL P ((void)),
    license P ((void)),
    looper P ((void)),
    makemessage P ((int)),
    moreprompt_helper P ((void)),
    move_if_needed P ((const char *, const char *)),
    myexit P ((void)),
    mysleep P ((unsigned int)),
    newawaymsg P ((void)),
    newmacro P ((int)),
    notitlebar P ((void)),
    not_replying_transform_express P ((char *)),
    open_browser P ((void)),
    opentmpfile P ((void)),
    otherinfo P ((void)),
    post_color_config P ((void)),
    post_friend_color_config P ((void)),
    post_user_color_config P ((void)),
    readbbsrc P ((void)),
    replycode_transform_express P ((char *)),
    replymessage P ((void)),
    reprint_line P ((void)),
    resetterm P ((void)),
    run P ((char *, char *)),
    savebbsrc P ((void)),
    send_an_x P ((void)),
    sendblock P ((void)),
    sendnaws P ((void)),
    setterm P ((void)),
    s_error P ((const char *, const char *)),
    setup P ((int)),
    siginit P ((void)),
    sigoff P ((void)),
    s_info P ((const char *, const char *)),
    slistDestroy P ((slist *)),
    slistDestroyItems P ((slist *)),
    slistSort P ((slist *)),
    smarterase P ((char *)),
    smartprint P ((char *, char *)),
    s_perror P ((const char *, const char *)),
    suspend P ((void)),
    techinfo P ((void)),
    telinit P ((void)),
    tempfileerror P ((void)),
    titlebar P ((void)), truncbbsrc P ((int)), warranty P ((void)), writebbsrc P ((void));

extern int
binary_search P ((char *)),
    binary_sort P ((void)),
    cap_printf P ((const char *, ...)),
    cap_putchar P ((int)),
    cap_puts P ((char *)),
    checkfile P ((FILE *)),
    colorize P ((const char *)),
    delete_queue P ((queue *)),
    deletefile P ((const char *)),
    ExtractNumber P ((char *)),
    fsortcmp P ((friend **, friend **)),
    fstrcmp P ((char *, friend *)),
    getkey P ((void)),
    getwindowsize P ((void)),
    inkey P ((void)),
    is_automatic_reply P ((const char *)),
    is_queued P ((char *, queue *)),
    more P ((int *, int)),
    net_printf P ((const char *, ...)),
    net_putchar P ((int)),
    net_puts P ((char *)),
    newkey P ((int)),
    pop_queue P ((char *, queue *)),
    prompt P ((FILE *, int *, int)),
    push_queue P ((char *, queue *)),
    safe_delete_queue P ((queue *)),
    slistAddItem P ((slist *, void *, int)),
    slistFind P ((slist *, void *, int (*findfn) (const void *, const void *))),
    slistRemoveItem P ((slist *, int)),
    smartname P ((char *, char *)),
    sortcmp P ((char **, char **)),
    s_prompt P ((const char *, const char *, int)),
    std_printf P ((const char *, ...)),
    std_putchar P ((int)),
    std_puts P ((char *)),
    telrcv P ((int)), waitnextevent P ((void)), yesno P ((void)), yesnodefault P ((int));

extern char
ansi_transform P ((char)),
    ansi_transform_post P ((char, int)),
    background_picker P ((void)),
    color_picker P ((void)),
    express_color_menu P ((void)),
    *ExtractName P ((char *)),
    general_color_menu P ((void)),
    *get_name P ((int)),
    *mystrchr P ((const char *, int)),
    *mystrdup P ((const char *)),
    *mystrstr P ((const char *, const char *)),
    post_color_menu P ((void)), *strctrl P ((int)), *stripansi P ((char *)), user_or_friend P ((void));

extern  FILE
    * findbbsrc P ((void)), *openbbsrc P ((void)), *openbbsfriends P ((void)), *findbbsfriends P ((void));

extern queue *new_queue P ((int, int));

extern  slist
    * slistCreate P ((int, int (*sortfn) (const void *, const void *), ...)),
    *slistIntersection P ((const slist *, const slist *));

extern RETSIGTYPE bye P ((int)), naws P ((int)), reapchild P ((int));

#endif /* PROTO_H_INCLUDED */
