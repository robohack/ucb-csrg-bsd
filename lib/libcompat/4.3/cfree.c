/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)cfree.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */

void
cfree(p)
	void *p;
{
	free(p);
}
