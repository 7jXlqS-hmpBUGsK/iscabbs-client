typedef struct {
    int killposts:1;			/* Kill posts */
    int killexpress:1;			/* Kill express messages */
    int notifykills:1;			/* Notify that a message was killed */
    int displaywholist:1;		/* Display user list on who list */
    int displaylong:1;			/* Display long (1) or short (0) */
} UserlistAttributes;

typedef struct {
    char name[41];			/* Name of user list */
    UserlistAttributes attributes;	/* Attributes for user list */
    slist *list;			/* List of users */
} Userlist;
