/*
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)ftruncate.c	8.1 (Berkeley) 6/17/93";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/syscall.h>

/*
 * This function provides 64-bit offset padding that
 * is not supplied by GCC 1.X but is supplied by GCC 2.X.
 */
int
ftruncate(fd, length)
	int	fd;
	off_t	length;
{

	return(__syscall((quad_t)SYS_ftruncate, fd, 0, length));
}
