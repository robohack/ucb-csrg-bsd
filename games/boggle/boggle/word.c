/*-
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Barry Brachman.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)word.c	8.1 (Berkeley) 6/11/93";
#endif /* not lint */

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bog.h"
#include "extern.h"

static char *dictspace, *dictend;
static char *sp;

static int first = 1, lastch = 0;

/*
 * Return the next word in the compressed dictionary in 'buffer' or
 * NULL on end-of-file
 */
char *
nextword(fp)
	FILE *fp;
{
	extern int wordlen;
	register int ch, pcount;
	register char *p;
	static char buf[MAXWORDLEN + 1];

	if (fp == NULL) {
		if (sp == dictend)
			return (NULL);
	
		p = buf + (int) *sp++;

		/*
		 * The dictionary ends with a null byte
		 */
		while (*sp >= 'a')
			if ((*p++ = *sp++) == 'q')
				*p++ = 'u';
	} else {
		if (first) {
			if ((pcount = getc(fp)) == EOF)
				return (NULL);
			first = 0;
		} else if ((pcount = lastch) == EOF)
			return (NULL);

		p = buf + pcount;
 
		while ((ch = getc(fp)) != EOF && ch >= 'a')
			if ((*p++ = ch) == 'q')
				*p++ = 'u';
		lastch = ch;
	}
	wordlen = (int) (p - buf);
	*p = '\0';
	return (buf);
}
 
/*
 * Reset the state of nextword() and do the fseek()
 */
long
dictseek(fp, offset, ptrname)
	FILE *fp;
	long offset;
	int ptrname;
{
	if (fp == NULL) {
		if ((sp = dictspace + offset) >= dictend)
			return (-1);
		return (0);
	}

	first = 1;
	return (fseek(fp, offset, ptrname));
}

FILE *
opendict(dict)
	char *dict;
{
	FILE *fp;

	if ((fp = fopen(dict, "r")) == NULL)
		return (NULL);
	return (fp);
}

/*
 * Load the given dictionary and initialize the pointers
 */
int
loaddict(fp)
	FILE *fp;
{
	struct stat statb;
	long n;
	int st;
	char *p;

	if (fstat(fileno(fp), &statb) < 0) {
		(void)fclose(fp);
		return (-1);
	}

	/*
	 * An extra character (a sentinel) is allocated and set to null
	 * to improve the expansion loop in nextword().
	 */
	if ((dictspace = malloc(statb.st_size + 1)) == NULL) {
		(void)fclose(fp);
		return (-1);
	}
	n = (long)statb.st_size;
	sp = dictspace;
	dictend = dictspace + n;

	p = dictspace;
	st = -1;
	while (n > 0 && (st = fread(p, 1, BUFSIZ, fp)) > 0) {
		p += st;
		n -= st;
	}
	if (st < 0) {
		(void)fclose(fp);
		(void)fprintf(stderr, "Error reading dictionary\n");
		return (-1);
	}
	*p = '\0';
	return (0);
}

/*
 * Dependent on the exact format of the index file:
 * Starting offset field begins in column 1 and length field in column 9
 * Taking the easy way out, the input buffer is made "large" and a check
 * is made for lines that are too long
 */
int
loadindex(indexfile)
	char *indexfile;
{
	register int i, j;
	char buf[BUFSIZ];
	FILE *fp;
	extern struct dictindex dictindex[];
 
	if ((fp = fopen(indexfile, "r")) == NULL) {
		(void) fprintf(stderr, "Can't open '%s'\n", indexfile);
		return (-1);
	}
	i = 0;
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (strchr(buf, '\n') == NULL) {
			(void)fprintf(stderr,
				"A line in the index file is too long\n");
			return(-1);
		}
		j = *buf - 'a';
		if (i != j) {
		    (void) fprintf(stderr, "Bad index order\n");
		    return(-1);
		}
		dictindex[j].start = atol(buf + 1);
		dictindex[j].length = atol(buf + 9) - dictindex[j].start;
		i++;
	}
	if (i != 26) {
		(void) fprintf(stderr, "Bad index length\n");
		return(-1);
	}
	(void) fclose(fp);
	return(0);
} 
