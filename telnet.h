/*
 * Generic telnet protocol #defines, along with some special protocol that is
 * spoken between the ISCA BBS and a connected client.  You don't want to
 * change this, it depends on this being the same on both ends, and you can't
 * change what the BBS end does. 
 */

#define	IAC	255                 /* interpret as command: */
#define	DONT	254             /* you are not to use option */
#define	DO	253                 /* please, you use option */
#define	WONT	252             /* I won't use option */
#define	WILL	251             /* I will use option */
#define	SB	250                 /* interpret as subnegotiation */
#define	SE	240                 /* end sub negotiation */

/* BBS client additions */
#define CLIENT	0xa0            /* Is a client */
#define BLOCK	0xa1            /* Start block mode transfer */
#define G_STR	0xa2            /* Get string (arg = length) */
#define G_NAME	0xa3            /* Get name (arg = type) */
#define G_FIVE	0xa4            /* Get 5 lines of text (Xmsg, info) */
#define G_POST	0xa5            /* Get post (arg: 0 = normal, 1 = upload) */
#define S_WHO	0xa6            /* Send who list */
#define XMSG_S	0xa7            /* Start X message transfer */
#define XMSG_E	0xa8            /* End X message transfer */
#define POST_S	0xa9            /* Start post transfer */
#define POST_E	0xaa            /* End post transfer */
#define MORE_M  0xab            /* Mark for MORE prompt */
#define START	0xac            /* Synchronize count, pass version number */
#define START2	0xad            /* Version of client (1.3) which does config */
#define CONFIG	0xae            /* Tell client to do configuration */
#define START3	0xaf            /* "Final" stable version of client (1.5) */
#define CLIENT2	0xb0            /* Client code for version 1.5 */
#define POST_K	0xb1            /* Kill post */

#define TELOPT_ECHO	1           /* echo */
#define	TELOPT_SGA	3           /* suppress go ahead */
#define	TELOPT_NAWS	31          /* window size */
#define TELOPT_ENVIRON	36      /* Environment variables */

/* Various states for telnet state machine */
#define	TS_DATA		0
#define	TS_IAC		1
#define	TS_CR		2
#define TS_GET		3
#define TS_VOID		4
