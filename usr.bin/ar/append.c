/*-
 * Copyright (c) 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Hugh Smith at The University of Guelph.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)append.c	8.3 (Berkeley) 4/2/94";
#endif /* not lint */

#include <sys/param.h>
#include <sys/stat.h>

#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include "archive.h"
#include "extern.h"

/*
 * append --
 *	Append files to the archive - modifies original archive or creates
 *	a new archive if named archive does not exist.
 */
int
append(argv)
	char **argv;
{
	int afd, fd, eval;
	char *file;
	CF cf;
	struct stat sb;

	afd = open_archive(O_CREAT|O_RDWR);
	if (lseek(afd, (off_t)0, SEEK_END) == (off_t)-1)
		error(archive);

	/* Read from disk, write to an archive; pad on write. */
	SETCF(0, 0, afd, archive, WPAD);
	for (eval = 0; file = *argv++;) {
		if ((fd = open(file, O_RDONLY)) < 0) {
			warn("%s", file);
			eval = 1;
			continue;
		}
		if (options & AR_V)
			(void)printf("q - %s\n", file);
		cf.rfd = fd;
		cf.rname = file;
		put_arobj(&cf, &sb);
		(void)close(fd);
	}
	close_archive(afd);
	return (eval);	
}
