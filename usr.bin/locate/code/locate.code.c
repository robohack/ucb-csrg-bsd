/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * James A. Woods.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1989 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)locate.code.c	5.3 (Berkeley) 5/27/92";
#endif /* not lint */

/*
 * PURPOSE:	sorted list compressor (works with a modified 'find'
 *		to encode/decode a filename database)
 *
 * USAGE:	bigram < list > bigrams
 *		process bigrams (see updatedb) > common_bigrams
 *		code common_bigrams < list > squozen_list
 *
 * METHOD:	Uses 'front compression' (see ";login:", Volume 8, Number 1
 *		February/March 1983, p. 8).  Output format is, per line, an
 *		offset differential count byte followed by a partially bigram-
 *		encoded ascii residue.  A bigram is a two-character sequence,
 *		the first 128 most common of which are encoded in one byte.
 *
 * EXAMPLE:	For simple front compression with no bigram encoding,
 *		if the input is...		then the output is...
 *
 *		/usr/src			 0 /usr/src
 *		/usr/src/cmd/aardvark.c		 8 /cmd/aardvark.c
 *		/usr/src/cmd/armadillo.c	14 armadillo.c
 *		/usr/tmp/zoo			 5 tmp/zoo
 *
 *	The codes are:
 *
 *	0-28	likeliest differential counts + offset to make nonnegative
 *	30	switch code for out-of-range count to follow in next word
 *	128-255 bigram codes (128 most common, as determined by 'updatedb')
 *	32-127  single character (printable) ascii residue (ie, literal)
 *
 * SEE ALSO:	updatedb.csh, bigram.c
 *
 * AUTHOR:	James A. Woods, Informatics General Corp.,
 *		NASA Ames Research Center, 10/82
 */

#include <sys/param.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "locate.h"

#define	BGBUFSIZE	(NBG * 2)	/* size of bigram buffer */
#define OERR		err("stdout: %s", strerror(errno))

char buf1[MAXPATHLEN] = " ";	
char buf2[MAXPATHLEN];
char bigrams[BGBUFSIZE + 1] = { 0 };

int	bgindex __P((char *));
void	err __P((const char *, ...));
void	usage __P((void));

int
main(argc, argv)
	int argc;
	char *argv[];
{
	register char *cp, *oldpath, *path;
	int ch, code, count, diffcount, oldcount;
	FILE *fp;

	while ((ch = getopt(argc, argv, "")) != EOF)
		switch(ch) {
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc != 1)
		usage();

	if ((fp = fopen(argv[1], "r")) == NULL)
		err("%s: %s", argv[1], strerror(errno));

	/* First copy bigram array to stdout. */
	(void)fgets(bigrams, BGBUFSIZE + 1, fp);
	if (fwrite(bigrams, 1, BGBUFSIZE, stdout) != BGBUFSIZE)
		err("stdout: %s", strerror(errno));
	(void)fclose(fp);

	oldpath = buf1;
	path = buf2;
	oldcount = 0;
	while (fgets(path, sizeof(buf2), stdin) != NULL) {
		/* Truncate newline. */
		cp = path + strlen(path) - 1;
		if (cp > path && *cp == '\n')
			*cp = '\0';

		/* Squelch characters that would botch the decoding. */
		for (cp = path; *cp != NULL; cp++) {
			if ((u_char)*cp >= PARITY)
				*cp &= PARITY-1;
			else if (*cp <= SWITCH)
				*cp = '?';
		}

		/* Skip longest common prefix. */
		for (cp = path; *cp == *oldpath; cp++, oldpath++)
			if (*oldpath == NULL)
				break;
		count = cp - path;
		diffcount = count - oldcount + OFFSET;
		oldcount = count;
		if (diffcount < 0 || diffcount > 2 * OFFSET) {
			if (putchar(SWITCH) == EOF ||
			    putw(diffcount, stdout) == EOF)
				OERR;
		} else
			if (putchar(diffcount) == EOF)
				OERR;

		while (*cp != NULL) {
			if (*(cp + 1) == NULL) {
				if (putchar(*cp) == EOF)
					OERR;
				break;
			}
			if ((code = bgindex(cp)) < 0) {
				if (putchar(*cp++) == EOF ||
				    putchar(*cp++) == EOF)
					OERR;
			} else {
				/* Found, so mark byte with parity bit. */
				if (putchar((code / 2) | PARITY) == EOF)
					OERR;
				cp += 2;
			}
		}
		if (path == buf1) {		/* swap pointers */
			path = buf2;
			oldpath = buf1;
		} else {
			path = buf1;
			oldpath = buf2;
		}
	}
	/* Non-zero status if there were errors */
	if (fflush(stdout) != 0 || ferror(stdout))
		exit(1);
	exit(0);
}

int
bgindex(bg)			/* Return location of bg in bigrams or -1. */
	char *bg;
{
	register char bg0, bg1, *p;

	bg0 = bg[0];
	bg1 = bg[1];
	for (p = bigrams; *p != NULL; p++)
		if (*p++ == bg0 && *p == bg1)
			break;
	return (*p == NULL ? -1 : --p - bigrams);
}

void
usage()
{
	(void)fprintf(stderr,
	    "usage: locate.code common_bigrams < list > squozen_list\n");
	exit(1);
}

#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

void
#if __STDC__
err(const char *fmt, ...)
#else
err(fmt, va_alist)
	char *fmt;
        va_dcl
#endif
{
	va_list ap;
#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	(void)fprintf(stderr, "locate.code: ");
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fprintf(stderr, "\n");
	exit(1);
	/* NOTREACHED */
}
