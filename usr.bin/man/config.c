/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
static char sccsid[] = "@(#)config.c	5.2 (Berkeley) 5/27/90";
#endif /* not lint */

#include <sys/param.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include "pathnames.h"

#define	MAXLINE		1024

extern char *progname;

static FILE *cfp;
static char *buf;

/*
 * getpath --
 *	read in the configuration file, calling a function with the line
 *	from each matching section.
 */
char *
getpath(sects)
	char **sects;
{
	register char **av, *p;
	size_t len;
	char **ar, line[MAXLINE], **getorder();

	ar = getorder();
	openconfig();
	while (fgets(line, sizeof(line), cfp)) {
		if (!index(line, '\n')) {
			(void)fprintf(stderr, "%s: config line too long.\n",
			    progname);
			exit(1);
		}
		p = strtok(line, " \t\n");
		if (!p || *p == '#')
			continue;
		for (av = sects; *av; ++av)
			if (!strcmp(p, *av))
				break;
		if (!*av)
			continue;
		while (p = strtok((char *)NULL, " \t\n")) {
			len = strlen(p);
			if (p[len - 1] == '/')
				for (av = ar; *av; ++av)
					cadd(p, len, *av);
			else
				cadd(p, len, (char *)NULL);
		}
	}
	return(buf);
}

static
cadd(add1, len1, add2)
char *add1, *add2;
register size_t len1;
{
	static size_t buflen;
	static char *bp, *endp;
	register size_t len2;

	len2 = add2 ? strlen(add2) : 0;
	if (!bp || bp + len1 + len2 + 2 >= endp) {
		if (!(buf = realloc(buf, buflen += 1024)))
			enomem();
		if (!bp)
			bp = buf;
		endp = buf + buflen;
	}
	bcopy(add1, bp, len1);
	bp += len1;
	if (len2) {
		bcopy(add2, bp, len2);
		bp += len2;
	}
	*bp++ = ':';
	*bp = '\0';
}

static
openconfig()
{
	if (cfp) {
		rewind(cfp);
		return;
	}
	if (!(cfp = fopen(_PATH_MANCONF, "r"))) {
		(void)fprintf(stderr, "%s: no configuration file %s.\n",
		    progname, _PATH_MANCONF);
		exit(1);
	}
}

char **
getdb()
{
	register char *p;
	int cnt, num;
	char **ar, line[MAXLINE];

	ar = NULL;
	num = 0;
	cnt = -1;
	openconfig();
	while (fgets(line, sizeof(line), cfp)) {
		if (!index(line, '\n')) {
			(void)fprintf(stderr, "%s: config line too long.\n",
			    progname);
			exit(1);
		}
		p = strtok(line, " \t\n");
#define	WHATDB	"_whatdb"
		if (!p || *p == '#' || strcmp(p, WHATDB))
			continue;
		while (p = strtok((char *)NULL, " \t\n")) {
			if (cnt == num - 1 &&
			    !(ar = realloc(ar, (num += 30) * sizeof(char **))))
				enomem();
			if (!(ar[++cnt] = strdup(p)))
				enomem();
		}
	}
	if (ar) {
		if (cnt == num - 1 &&
		    !(ar = realloc(ar, ++num * sizeof(char **))))
			enomem();
		ar[++cnt] = NULL;
	}
	return(ar);
}

static char **
getorder()
{
	register char *p;
	int cnt, num;
	char **ar, line[MAXLINE];

	ar = NULL;
	num = 0;
	cnt = -1;
	openconfig();
	while (fgets(line, sizeof(line), cfp)) {
		if (!index(line, '\n')) {
			(void)fprintf(stderr, "%s: config line too long.\n",
			    progname);
			exit(1);
		}
		p = strtok(line, " \t\n");
#define	SUBDIR	"_subdir"
		if (!p || *p == '#' || strcmp(p, SUBDIR))
			continue;
		while (p = strtok((char *)NULL, " \t\n")) {
			if (cnt == num - 1 &&
			    !(ar = realloc(ar, (num += 30) * sizeof(char **))))
				enomem();
			if (!(ar[++cnt] = strdup(p)))
				enomem();
		}
	}
	if (ar) {
		if (cnt == num - 1 &&
		    !(ar = realloc(ar, ++num * sizeof(char **))))
			enomem();
		ar[++cnt] = NULL;
	}
	return(ar);
}

getsection(sect)
	char *sect;
{
	register char *p;
	char line[MAXLINE];

	openconfig();
	while (fgets(line, sizeof(line), cfp)) {
		if (!index(line, '\n')) {
			(void)fprintf(stderr, "%s: config line too long.\n",
			    progname);
			exit(1);
		}
		p = strtok(line, " \t\n");
		if (!p || *p == '#')
			continue;
		if (!strcmp(p, sect))
			return(1);
	}
	return(0);
}

enomem()
{
	(void)fprintf(stderr, "%s: %s\n", progname, strerror(ENOMEM));
	exit(1);
}
