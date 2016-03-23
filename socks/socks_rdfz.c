/* socks_rdfz */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <syslog.h>
#include "socks.h"


int socks_rdfz(fc, cfAddrPtr, NcfPtr, cfstrings, useSyslog)
char *fc;
struct config **cfAddrPtr;
int *NcfPtr;
char **cfstrings;
int useSyslog;
{
	int fd, i;
	char *stringarea, *startaddr;
	int stringsize;
	int Ncf;
	struct config *cfAddr, *cp;

	if ((fd = open(fc, O_RDONLY)) < 0) {
		if (useSyslog)
			syslog(LOG_HIGH, "Cannot open %s: %m\n", fc);
		else
			perror("socks_rdfz(): open()");
		exit(1);
	}
	if (*cfAddrPtr != NULL)
		free(*cfAddrPtr);
	if (*cfstrings != NULL)
		free(*cfstrings);

	if (read(fd, &Ncf, sizeof(Ncf)) != sizeof(Ncf)) {
		if (useSyslog)
			syslog(LOG_HIGH, "Error: read from %s: %m\n", fc);
		else
			perror("socks_rdfz(): read()");
		exit(1);
	}

	if (read(fd, &stringsize, sizeof(stringsize)) != sizeof(stringsize)) {
		if (useSyslog)
			syslog(LOG_HIGH, "Error: read from %s: %m\n", fc);
		else
			perror("socks_rdfz(): read()");
		exit(1);
	}

	cfAddr = (struct config *) malloc(Ncf * (sizeof(struct config)));
	if (cfAddr == NULL) {
		if (useSyslog)
			syslog(LOG_HIGH, "Out of memory\n");
		else
			perror("socks_rdfz(): malloc()");
		exit(1);
	}
	if (read(fd, cfAddr, Ncf * sizeof(struct config)) != (Ncf * sizeof(struct config))) {
		if (useSyslog)
			syslog(LOG_HIGH, "Error: read from %s: %m\n", fc);
		else
			perror("socks_rdfz(): read()");
		exit(1);
	}
	*cfAddrPtr = cfAddr;
	*NcfPtr = Ncf;
	if (stringsize == 0) {
		close(fd);
		return 0;
	}
	if ((stringarea = (char *)malloc(stringsize)) == NULL) {
		if (useSyslog)
			syslog(LOG_HIGH, "Out of memory\n");
		else
			perror("socks_rdfz(): malloc()");
		exit(1);
	}
	*cfstrings = stringarea;
	if (read(fd, stringarea, stringsize) != stringsize) {
		if (useSyslog)
			syslog(LOG_HIGH, "Error: read from %s: %m\n", fc);
		else
			perror("socks_rdfz(): read()");
		exit(1);
	}

	startaddr = stringarea - 1;

	for (i = 0, cp = cfAddr; i++ < Ncf; cp++) {
		if (cp->userlist != NULL)
			cp->userlist = startaddr + (int)cp->userlist;
		if (cp->serverlist != NULL)
			cp->serverlist = startaddr + (int)cp->serverlist;
		if (cp->sdomain != NULL)
			cp->sdomain = startaddr + (int)cp->sdomain;
		if (cp->ddomain != NULL)
			cp->ddomain = startaddr + (int)cp->ddomain;
		if (cp->cmdp != NULL)
			cp->cmdp = startaddr + (int)cp->cmdp;
	}
	return 0;
}
