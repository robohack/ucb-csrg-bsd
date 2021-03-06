/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)umount.c	8.1 (Berkeley) 6/11/93
 */

#include <sys/param.h>
#include <sys/mount.h>

int eval;

main(argc, argv)
	int argc;
	char **argv;
{
	if (*++argv && **argv == '-') {
		err("no options available", 0);
		_exit(1);
	}
	for (eval = 0; *argv; ++argv)
		if (unmount(*argv, MNT_NOFORCE)) {
			err(*argv, 1);
			eval = 1;
		}
	_exit(eval);
}

#define	PROGNAME	"umount: "
#include "errfunction"
