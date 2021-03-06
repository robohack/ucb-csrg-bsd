/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Hugh Smith at The University of Guelph.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1990, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)ranlib.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <archive.h>

CHDR chdr;
u_int options;				/* UNUSED -- keep open_archive happy */
char *archive;

main(argc, argv)
	int argc;
	char **argv;
{
	extern int optind;
	int ch, eval, tflag;

	tflag = 0;
	while ((ch = getopt(argc, argv, "t")) != EOF)
		switch(ch) {
		case 't':
			tflag = 1;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (!*argv)
		usage();

	for (eval = 0; archive = *argv++;)
		eval |= tflag ? touch() : build();
	exit(eval);
}

usage()
{
	(void)fprintf(stderr, "usage: ranlib [-t] archive ...\n");
	exit(1);
}
