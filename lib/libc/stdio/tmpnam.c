/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)tmpnam.c	5.1 (Berkeley) 1/20/91";
#endif /* LIBC_SCCS and not lint */

#include <sys/param.h>
#include <stdio.h>

char *
tmpnam(s)
	char *s;
{
	static char buf[MAXPATHLEN];
	char *mktemp();

	if (s == NULL)
		s = buf;
	(void) sprintf(s, "%s/XXXXXX", P_tmpdir);
	return (mktemp(s));
}
