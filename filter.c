/* IO ERROR's BBS client 2.3, Michael Hampton.  Modified from the original
 * ISCA BBS client 1.5 and patches.  Copyright 1999 Michael Hampton.
 * Internet: error@citadel.org  WWW: http://ioerror.bbsclient.net/
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 675 Mass
 * Ave, Cambridge, MA 02139, USA.
 */

#include "defs.h"
#include "ext.h"
#include "telnet.h"


static char thisline[320];	/* Copy of the current line */


void filter_wholist(register int c)
{
    register int i;		/* generic counter */
    static char new;
    static int col;
    static unsigned char who[21];	/* Buffer for current name in who list */
    static unsigned char *whop = (char *) NULL;
    static long timestamp = 0;	/* Friend list timestamp */
    static long timer = 0;	/* Friend list timestamp */
    static long now = 0;	/* Current time */
    static long extime = 0;	/* Extended time decoder */
    char junk[80], work[100];
    char *pc;
    friend *pf;

    if (!whop) {		/* First time through */
	whop = who;
	*who = 0;
	new = col = 0;
    }
    if (c) {			/* received a character */
	new = 1;
	*whop++ = (char) c;
	*whop = 0;
    } else {			/* received a null */
	/*
	 * DOC sends two NULs after S_WHO when we are supposed to display the
	 * saved who list.  We will eat the first character and only exit when
	 * the second NUL comes in.  Since it sends a single NUL to signal
	 * termination of the who list, this lets us distinguish.
	 * Follow carefully, lest you get lost in here.
	 */
	if (who == whop) {	/* Time to end it all */
	    if (new == 1) {
		if (!savewhop)	/* FIXME: I think this is buggy */
		    std_printf("No friends online (new)");
		whop = (char *) NULL;
		new = 0;
		wholist = 0;
	    } else if (new == 0) {
		now = time(NULL) - timestamp;
		if (now - 66 == timer)
		    timer = -1;
		else
		    timer = now;
		if (savewhop) {	/* we've stored an old wholist */
		    std_printf(timer == -1 ?
					"Die die die fornicate (666)\r\n\n" :
				    "Your friends online (%d:%02d old)\r\n\n",
			       (int) (now / 60),
			       (int) (now % 60));
		    for (; col++ < savewhop;) {
			if ((i = slistFind(friendList, savewho[col - 1], (int (*)())strcmp)) != -1)
			    pf = friendList->items[i];
			else
			    pf = NULL;
			/* FIXME: Finish writing this! */
			strcpy(junk, (char *) savewho[col - 1] + 1);
			sprintf(work, flags.useansi ? "@Y%c%-18s%c @R   %2d:%02d@G  @C%s\r\n"
				: "%c%-18s%c    %2d:%02d  %s\r\n",
			*junk & 0x7f, junk + 1, *junk & 0x80 ? '*' : ' ',
			     (*savewho[col - 1] + (int) (now / 60)) / 60,
			     (*savewho[col - 1] + (int) (now / 60)) % 60,
				saveinfo[col - 1]);
			colorize(work);
		    }
		    col--;	/* FIXME: filter.c has col = 0 ??? */
		} else {
		    std_printf(timer == -1 ?
				    "Die die die (666)" :
				    "No friends online (%d:%02d old)",
			       (int) (now / 60), (int) (now % 60));
		}
		wholist = 0;
		whop = (char *) NULL;
	    }
	} else {		/* Received a friend */
	    whop = who;
	    if (wholist++ == 1) {	/* List copy is OK */
		savewhop = 0;
		slistDestroyItems(whoList);
		slistDestroy(whoList);
		if (!(whoList = slistCreate(0, (int (*)())sortcmp)))
		    fatalexit("Can't re-create saved who list!\r\n", "Fatal error");
		for (i = 0; i < friendList->nitems; i++) {
		    pf = friendList->items[i];
		    if (!(pc = (char *) calloc(1, strlen(pf->name) + 1)))
			fatalexit("Out of memory for list copy!\r\n", "Fatal error");
		    strcpy(pc, pf->name);
		    if (!(slistAddItem(whoList, pc, 0)))
			fatalexit("Out of memory adding item in list copy!\r\n", "Fatal error");
		}
	    }
	    /* Handle extended time information */
	    if (*who == 0xfe) {
		/* Decode BCD */
		for (pc = who + 1; *pc; pc++) {
		    extime = 10 * extime + *pc - 1;
		}
	    } else {
		/* output name and info if user is on our 'friend' list */
		strcpy(junk, (char *) who + 1);
		*junk &= 0x7f;
		if (!(pc = (char *) calloc(1, strlen(junk) + 1)))
		    fatalexit("Out of memory adding to saved who list!\r\n", "Fatal error");
		strcpy(pc, junk);
		if (slistFind(whoList, pc, (int (*)())strcmp) == -1)
		    if (!(slistAddItem(whoList, pc, 0)))
			fatalexit("Can't add item to saved who list!\r\n", "Fatal error");
		timestamp = time(NULL);
		if ((c = slistFind(friendList, junk, (int (*)())fstrcmp)) != -1) {
		    if (!col++)
			std_printf("Your friends online (new)\r\n\n");
		    --*who;
		    if (col <= 60) {	/* FIXME: make saved list a real list? */
			pf = friendList->items[c];
			if (extime == 0)
			    extime = (long) (*who);
			if (extime >= 1440)
			    sprintf(work, flags.useansi ?
				    "@Y%-19s%c @R%2dd%02d:%02d@G  @C%s\r\n" :
				    "%-19s%c %2dd%02d:%02d  %s\r\n",
				    junk, who[1] & 0x80 ? '*' : ' ',
				    extime / 1440,
				    (extime % 1440) / 60, (extime % 1440) % 60,
				    pf->info);
			else
			    sprintf(work, flags.useansi ?
				    "@Y%-19s%c @R   %2d:%02d@G  @C%s\r\n" :
				    "%-19s%c    %2d:%02d  %s\r\n",
				    junk, who[1] & 0x80 ? '*' : ' ',
				    extime / 60, extime % 60, pf->info);
			colorize(work);
			*savewho[savewhop] = *who;
			strcpy((char *) savewho[savewhop] + 1, (char *) who + 1);
			strcpy((char *) saveinfo[savewhop], pf->info);
			savewhop++;
		    }
		}
		extime = 0;
	    }
	}
    }
}


void filter_express(register int c)
{
    register int i;		/* generic counter */
    static char *sp, *bp;	/* comparison pointer */
    static struct {
	int crlf:1;		/* Needs initial CR/LF */
	int prochdr:1;		/* Needs killfile processing */
	int ignore:1;		/* Ignore the remainder of the X */
    } needs;
    char *thisline = xmsgbufp;	/* Pointer to the current line */

    if (c == -1) {		/* signal from IAC to begin/end X */
	if (xmsgnow) {		/* Need to re-init for new X */
	    xmsgbufp = xmsgbuf;
	    *xmsgbuf = 0;
	    needs.ignore = 0;
	    needs.crlf = 0;
	    needs.prochdr = 1;
	    return;
	} else if (!needs.ignore) {	/* Finished this X, dump it out */
		/* Process for automatic reply */
		i = ExtractNumber(xmsgbuf);
		/* Don't queue if it's an outgoing X */
		/* Only send 'x' if it's a new incoming X */
		if ((away || xland) && !is_queued(bp, xlandQueue) &&
		    i > highxmsg && !is_automatic_reply(xmsgbuf) && bp) {
			push_queue(bp, xlandQueue);
			needx = 1;
		} else if (is_automatic_reply(xmsgbuf) && i > highxmsg) {
			not_replying_transform_express(xmsgbuf);
		}
		/* Update highxmsg with greatest number seen */
		if (i > highxmsg)
		    highxmsg = i;
		replycode_transform_express(xmsgbuf);
		ansi_transform_express(xmsgbuf);
		std_printf("%s%s", (needs.crlf) ? "\r\n" : "", xmsgbuf);
		return;
	}
    }
    /* If the message is killed, don't bother doing anything. */
    if (needs.ignore) return;

    if (xmsgbuf == xmsgbufp) {	/* Check for initial CR/LF pair */
	if (c == '\r' || c == '\n') {
	    needs.crlf = 1;
	    return;
	}
    }
    /* Insert character into the buffer */
    *xmsgbufp++ = c;
    *xmsgbufp = 0;

    /* Extract URLs if any */
    if (!needs.prochdr && c == '\r') {
	    filter_url(thisline);
	    thisline = xmsgbufp;
    }

    /* If reached a \r it's time to do header processing */
    if (needs.prochdr && c == '\r') {
	needs.prochdr = 0;
	thisline = xmsgbufp;

	/* Process for kill file */
	sp = mystrstr(xmsgbuf, " from ");
	if (!mystrstr(xmsgbuf, "*** Message ") &&
	    !mystrstr(xmsgbuf, "%%% Question "))
	    sp = NULL;
	/* get name for ^N function */
	bp = ExtractName(xmsgbuf);
	/* FIXME: move this down to where the msg is printed! so that enemies are
	   not added to the ^N scroll */
	if (slistFind(enemyList, bp, (int (*)())strcmp) != -1) {
	    if (!flags.squelchexpress)
		std_printf("\r\n[X message by %s killed]\r\n", bp);
	    needs.ignore = 1;
	    return;
	}
    }
    return;
}

void filter_post(register int c)
{
    static int ansistate = 0;	/* ANSI state count */
    static int numposts = 0;	/* count of the # of posts received so far */
    static char posthdr[140];	/* store the post header here */
    static char *posthdrp;	/* pointer into posthdr */
    static struct {
	int crlf:1;		/* need to add a CR/LF */
	int prochdr:1;		/* process post header */
	int ignore:1;		/* kill this post */
	int second_n:1;		/* send a second n */
    } needs;
    static char *bp;		/* misc. pointer */
    static char junk[160];
    static int isFriend;	/* Current post is by a friend */

    if (c == -1) {		/* control: begin/end of post */
	if (postnow) {		/* beginning of post */
	    posthdrp = posthdr;
	    *posthdr = 0;
	    needs.crlf = 0;
	    needs.prochdr = 1;
	    isFriend = 0;
	    needs.ignore = 0;
	    needs.second_n = 0;
	} else {		/* end of post */
	    if (needs.second_n) {
		net_putchar('n');
		byte++;
	    }
	    filter_url(" ");
	    numposts++;
	}
	return;
    }
    /* If post is killed, do no further processing. */
    if (needs.ignore)		/* ignore our stupid enemies :) */
	return;

    if (posthdr == posthdrp) {	/* Check for initial CR/LF pair */
	if (c == '\r' || c == '\n') {
	    needs.crlf = 1;
	    return;
	}
    }
    /* At this point we should either insert the character into the post
     * buffer, or echo it to the screen.
     */
    if (!needs.prochdr) {
	    /* Store this line for processing */
	    if (c == '\n') {
		filter_url(thisline);
		thisline[0] = 0;
	    } else {
		/* This is a whole lot faster than calling strcat() */
	        char *p = thisline;
	
		for (; *p; p++)		/* Find end of string */
		    ;
	
		*p++ = c;		/* Copy character to end of string */
		*p = 0;
	    }

	/* Process ANSI codes in the middle of a post */
	if (ansistate) {
	    ansistate--;
	    if (ansistate == 1) {
		if (flags.offbold && c == 109) {	/* Turn boldface off */
		    printf("m\033[0");
		    ansistate--;
		} else {
		    lastcolor = ansi_transform_post((char)c, isFriend);
		    c = lastcolor;
		}
	    }
	} else if (c == '\033') {	/* Escape character */
	    ansistate = 4;
	    if (!flags.useansi) {
		flags.useansi = 1;
		if (!flags.usebold)
		    flags.offbold = 1;
	    }
	}
	/* Change color for end of more prompt */
	if (flags.useansi && flags.moreflag && c == ' ')
	    std_printf("\033[3%cm", lastcolor = color.text);	/* assignment */

	/* Output character */
	std_putchar(c);
    } else {
	*posthdrp++ = c;	/* store character in buffer */
	*posthdrp = 0;

	/* If reached a \r it's time to do header processing */
	if (c == '\r') {
	    needs.prochdr = 0;

	    /* Process for enemy list kill file */
	    strcpy(junk, posthdr);
	    /* FIXME: name of enemy should not be added to ^N list */
	    bp = ExtractName(junk);
/*      strcpy(junk, posthdr);  * Why did I do this? */
	    isFriend = (slistFind(friendList, bp, (int (*)())fstrcmp) != -1) ? 1 : 0;
	    ansi_transform_posthdr(posthdr, isFriend);
	    strcpy(saveheader, posthdr);
	    strcat(saveheader, "\r\n");
	    if (slistFind(enemyList, bp, (int (*)())strcmp) != -1) {
		needs.ignore = 1;
		postflag = postnow = -1;
		net_printf("%c%c%c%c", IAC, POST_K, numposts & 0xFF, 17);
		netflush();
		if (!flags.squelchpost) {
		    std_printf("%s[Post by %s killed]\r\n",
			       *posthdr == '\n' ? "\r\n" : "", bp);
		} else {
		    eatline = (needs.crlf ? 1 : 2);
		    net_putchar('n');
		    netflush();
		    byte++;
		}
		return;
	    }
	    std_printf("%s%s\r", (needs.crlf) ? "\r\n" : "", posthdr);
	}
    }
}


void filter_url(char *line)
{
	static int multiline = 0;
	static char url[1024];
	char *p, *q;
	int c, off = 0, len = 0;
	
	if (!urlQueue)	/* Can't store URLs for some reason */
		return;

	if (!multiline)
	    strcpy(url, line);
	else
	    strcat(url, line);

	for (c = strlen(url) - 1; c >= 0; c--)
	    if (url[c] == ' ' || url[c] == '\t' || url[c] == '\r')
		url[c] = 0;
	    else
		break;
	strcat(url, " ");

	if (!(p = strstr(url, "http://"))) {
	    if (!(p = strstr(url, "ftp://"))) {
		url[0] = 0;
		multiline = 0;
		return;
	    }
	}
	off = p - url;

	for (q = p; *q; q++) {
	    if (mystrchr(":/abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789$-_@.&+,=;?%~{|}", *q))
		continue;
	    *q = 0;
	    len = q - p;
	    
	    /* Oops, looks like a multi-line URL */
	    if ((!multiline && p == line && q > p + 77) || (multiline && strlen(line) > 77))
	    if (strlen(line) > 77)
		break;

	    if (!is_queued(p, urlQueue)) {
		char junk[1024];
		while (!push_queue(p, urlQueue)) {
		    pop_queue(junk, urlQueue);
		}
	    }
/*	    printf("\r\nSnarfed URL: <%s>\r\n", urls[nurls - 1]); */
	    multiline = 0;
	    return;
	}

	multiline = 1;
/*	printf("Multiline URL, got <%s> so far.\r\n", url); */
	return;
}


void filter_data(register int c)
{
    static int ansistate = 0;	/* Counter for ANSI transformations */

    /* Copy the current line (or what we have so far) */
    if (c == '\n') {
	filter_url(thisline);
	thisline[0] = 0;
    } else {
	/* This is faster than calling strcat() */
        char *p = thisline;

	for (; *p; p++) ;	/* Find end of string */

	*p++ = c;		/* Copy character to end of string */
	*p = 0;
    }

    /* Auto-answer ANSI question */
    if (flags.ansiprompt && strstr(thisline, "Are you on an ANSI")) {
	    net_putchar('y');
	    byte++;
	    *thisline = 0;	/* Kill it; we don't need it */
    }

    /* Automatic X reply */
    /*
    if (SendingX == SX_SENT_NAME) {
	SendingX = SX_SEND_NEXT;
#ifdef DEBUG
		std_printf("filter_data 1 SendingX is %d, xland is %d\r\n", SendingX, xland);
#endif
    }
    */
  /*  if (SendingX == SX_SEND_NEXT && xlandQueue->nobjs && (away || xland))
	send_an_x();
	*/
    if (SendingX == SX_SEND_NEXT && !*thisline && c == '\r') {
	SendingX = SX_NOT;
#ifdef DEBUG
		std_printf("filter_data 2 SendingX is %d, xland is %d\r\n", SendingX, xland);
#endif
	if (xlandQueue->nobjs && (away || xland))
		send_an_x();
    }

    /* Change color for end of more prompt */
    if (flags.useansi && flags.moreflag && c == ' ')
	std_printf("\033[3%cm", lastcolor = color.text);	/* assignment */

    /* Parse ANSI sequences */
    if (ansistate) {
	ansistate--;
	if (ansistate == 1) {
	    if (flags.offbold && c == 109) {	/* Turn boldface off */
		std_printf("m\033[0");
		ansistate--;
	    } else {
		lastcolor = ansi_transform((char)c);
		c = lastcolor;
	    }
	}
    } else if (c == '\033') {	/* Escape character */
	std_printf("\033[4%cm", color.background);
	ansistate = 4;
	if (!flags.useansi) {
	    flags.useansi = 1;
	    if (!flags.usebold)
		flags.offbold = 1;
	}
    }
    if (eatline > 0) {
	if (c == '\n') {
	    eatline--;
	}
    } else {
	eatline = 0;		/* eatline should never be less than 0 */
	std_putchar(c);
    }
    return;
}


void reprint_line(void)
{
	char line[320];
	char *p;
	
	strncpy(line, thisline, 320);
	std_putchar('\r');
	for (p = line; *p; p++)
		filter_data(*p);
	strncpy(thisline, line, 320);
}


void moreprompt_helper(void)
{
    if (!flags.useansi)
	return;

    if (postnow)
	continued_post_helper();
    else
	continued_data_helper();
}


void continued_post_helper(void)
{
    static char junk[] = "\033[32m";
    char *s;

    for (s = junk; *s; s++)
	filter_post(*s);
}


void continued_data_helper(void)
{
    static char junk[] = "\033[32m";
    char *s;

    for (s = junk; *s; s++)
	filter_data(*s);
}


/* Check for an automatic reply message. Return 1 if this is such a message. */
int is_automatic_reply(const char *message)
{
	char *p;

	/* Find first line */
	p = mystrstr(message, ">");

	/* Wasn't a first line? - move past '>' */
	if (!p) return 0;
	p++;

	/* Check for valid automatic reply messages */
	if (!strncmp(p, "+!R", 3) ||
	    !strncmp(p, "This message was automatically generated", 40) ||
	    !strncmp(p, "*** ISCA Windows Client", 23))
		return 1;

	/* Looks normal to me... */
	return 0;
}


void not_replying_transform_express(char *s)
{
	char junk[580];
	char *sp;

	/* Verify this is an X message and set up pointers */
	sp = mystrstr(s, " at ");
	if (!sp)
		return;

	*(sp++) = 0;

	sprintf(junk, "%s (not replying) %s", s, sp);
	strcpy(s, junk);
}


void replycode_transform_express(char *s)
{
	char junk[580];
	char *sp;

	/* Verify this is an auto reply and set up pointers */
	sp = mystrstr(s, ">");
	if (!sp || strncmp(sp, ">+!R ", 5))
		return;

	*(++sp) = 0;
	sp += 4;

	sprintf(junk, "%s%s", s, sp);
	strcpy(s, junk);
}
