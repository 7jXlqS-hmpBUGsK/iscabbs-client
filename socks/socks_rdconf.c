/* socks_rdconf */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <syslog.h>
#include "socks.h"


int socks_rdconf(filename, cfAddrPtr, NcfPtr, useSyslog)
char *filename;
struct config **cfAddrPtr;
int *NcfPtr;
int useSyslog;
{
	FILE		*fd;
	static char	buf[1024];
	char		*bp, *ch;
	int		linenum = 0;
	char	*argv[10];
	int	argc;
	int	next_arg;
	long	p;
	int Ncf = 0, maxcf = 0;
	struct config *cfAddr, *cp;
	int	has_error = 0;
	int	i;
	int	k;


	if ((fd = fopen(filename, "r")) == NULL) {
		if (useSyslog)
			syslog(LOG_HIGH, "Cannot open %s\n", filename);
		else
			fprintf(stderr, "Cannot open %s\n", filename);
		exit(1);
	}

	for (i = 0, cp = *cfAddrPtr; i++ < *NcfPtr; cp++) {
		if (cp->userlist != NULL)
			free(cp->userlist);
		if (cp->serverlist != NULL)
			free(cp->serverlist);
		if (cp->cmdp != NULL)
			free(cp->cmdp);
		if (cp->ddomain != NULL)
			free(cp->ddomain);
	}
	if (*cfAddrPtr)
		free(*cfAddrPtr);

	maxcf = CONF_INCR;
	cfAddr = (struct config *) malloc( maxcf * sizeof(struct config));
	if (cfAddr == NULL) {
		if (useSyslog)
			syslog(LOG_HIGH, "Out of memory\n");
		else
			fprintf(stderr, "Out of memory\n");
		exit(1);
	}
	Ncf = 0;
	cp = cfAddr;

	while (fgets(buf, sizeof(buf) - 1, fd) != NULL) {
		linenum++;
		bzero(cp, sizeof(struct config));
		/*
		**  Comments start with a '#' anywhere on the line
		*/
		if ((bp = index(buf, '\n')) != NULL)
			*bp = '\0';
		for (bp = buf; *bp != '\0'; bp++) {
			if (*bp == ':') {
				*bp++ = '\0';
				cp->cmdp = strdup(bp);
				if (cp->cmdp == NULL) {
					if (useSyslog)
						syslog(LOG_HIGH, "Out of memory\n");
					else
						fprintf(stderr, "Out of memory\n");
					exit(1);
				}
				break;
			} else if (*bp == '#') {
				*bp = '\0';
				break;
			} else if (*bp == '\t')
				*bp = ' ';
		}
		if (strlen(buf) == 0) continue;
		socks_mkargs(buf, &argc, argv, 10);
		if (argc == 0) {
			continue;
		}
/* #ifdef hpux */
		if (STREQ(*argv, "domain") || STREQ(*argv, "nameserver") ||
			STREQ(*argv, "bind") || STREQ(*argv, "findserver"))
				continue;
/* #endif */ /* #ifdef hpux */
		if ((argc < 3) || (argc > 7)) {
			if (useSyslog)
				syslog(LOG_HIGH, "Invalid entry at line %d in file %s", linenum, filename);
			else
				fprintf(stderr, "Invalid entry at line %d in file %s\n", linenum, filename);
			has_error = 1;
			continue;
		}
		
		/* parse the whole entry now, once. */
		next_arg = 1;

		if (STREQ(argv[0], "sockd")) {
			cp->action = SOCKS_SOCKD;
			k = strlen("@=");
			if (strncmp(argv[1], "@=", k) == 0) {
				if (*(argv[1] + k))
					cp->serverlist = strdup(argv[1] + k);
					if (cp->serverlist == NULL) {
						if (useSyslog)
							syslog(LOG_HIGH, "Out of memory\n");
						else
							fprintf(stderr, "Out of memory\n");
						exit(1);
					}
				next_arg++;
			}
		} else if (strncmp(argv[0], "sockd@", (k=strlen("sockd@"))) == 0) {
			cp->action = SOCKS_SOCKD;
			cp->serverlist = strdup(argv[0] + k);
			if (cp->serverlist == NULL) {
				if (useSyslog)
					syslog(LOG_HIGH, "Out of memory\n");
				else
					fprintf(stderr, "Out of memory\n");
				exit(1);
			}
		} else if (STREQ(argv[0], "direct")) {
			cp->action = SOCKS_DIRECT;
		} else if (STREQ(argv[0], "deny")) {
			cp->action = SOCKS_DENY;
		} else {
			if (useSyslog)
				syslog(LOG_HIGH, "Invalid sockd/direct/deny field at line %d in file %s", linenum, filename);
			else
				fprintf(stderr, "Invalid sockd/direct/deny field at line %d in file %s\n", linenum, filename);
			has_error = 1;
			continue;
		}

		k = strlen("*=");
		if (strncmp(argv[next_arg], "*=", k) == 0) {
			if (*(argv[next_arg] + k)) {
				cp->userlist = strdup(argv[next_arg] + k);
				if (cp->userlist == NULL) {
					if (useSyslog)
						syslog(LOG_HIGH, "Out of memory\n");
					else
						fprintf(stderr, "Out of memory\n");
					exit(1);
				}
			}
			next_arg++;
		}
		if(argc <= next_arg+1) {
			if (useSyslog)
				syslog(LOG_HIGH, "Invalid entry at line %d in file %s", linenum, filename);
			else
				fprintf(stderr, "Invalid entry at line %d in file %s\n", linenum, filename);
			has_error = 1;
			continue;
		}
		if (socks_GetAddr(argv[next_arg++], &cp->daddr, &cp->ddomain) == -1) {
			if (useSyslog)
				syslog(LOG_HIGH, "Out of memory\n");
			else
				fprintf(stderr, "Out of memory\n");
			has_error = 1;
			continue;
		}
		if (socks_GetQuad(argv[next_arg++], &cp->dmask) == -1) {
			if (useSyslog)
				syslog(LOG_HIGH, "Illegal destination mask at line %d in file %s", linenum, filename);
			else
				fprintf(stderr, "Illegal destination mask at line %d in file %s\n", linenum, filename);
			has_error = 1;
			continue;
		}
		if (argc > next_arg + 1) {
			ch = argv[next_arg];
			if (STREQ(ch, "eq"))
				cp->tst = e_eq;
			else if (STREQ(ch, "neq"))
				cp->tst = e_neq;
			else if (STREQ(ch, "lt"))
				cp->tst = e_lt;
			else if (STREQ(ch, "gt"))
				cp->tst = e_gt;
			else if (STREQ(ch, "le"))
				cp->tst = e_le;
			else if (STREQ(ch, "ge"))
				cp->tst = e_ge;
			else {
				if (useSyslog)
					syslog(LOG_HIGH, "Invalid comparison at line %d in file %s", linenum, filename);
				else
					fprintf(stderr, "Invalid comparison at line %d in file %s\n", linenum, filename);
				has_error = 1;
				continue;
			}
				
			if (((p = socks_GetPort(argv[next_arg+1])) < 0) ||
				(p >= (1L << 16))) {
				if (useSyslog)
					syslog(LOG_HIGH, "Invalid port number at line %d in file %s", linenum, filename);
				else
					fprintf(stderr, "Invalid port number at line %d in file %s\n", linenum, filename);
				has_error = 1;
				continue;
			} else {
				cp->dport = p;
			}
		} else {
			cp->tst = e_nil;
		}

		if (++Ncf >= maxcf) {
			maxcf += CONF_INCR;
			cfAddr = (struct config *) realloc(cfAddr, maxcf * sizeof(struct config));
			if (cfAddr == NULL) {
				if (useSyslog)
					syslog(LOG_HIGH, "Out of memory\n");
				else
					fprintf(stderr, "Out of memory\n");
				exit(1);
			}
			cp = cfAddr + Ncf;
		} else  {
			cp++;
		}

	}
	fclose(fd);
	if (Ncf == 0) {
		if (useSyslog)
			syslog(LOG_HIGH, "No valid entires in file %s", filename);
		else
			fprintf(stderr, "No valid entires in file %s\n", filename);
		exit(1);
	}
	if (has_error)
		exit(1);
	if (Ncf < maxcf)
		cfAddr = (struct config *) realloc(cfAddr, Ncf * sizeof(struct config));
	*NcfPtr = Ncf;
	*cfAddrPtr = cfAddr;
	return 0;
}
