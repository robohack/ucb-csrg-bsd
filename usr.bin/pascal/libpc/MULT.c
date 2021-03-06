/*-
 * Copyright (c) 1979, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)MULT.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */

long *
MULT(result0, left, right, siz)

	long		*result0;
	register long	*left;
	register long	*right;
	long		siz;
{
	register long	*result = result0;
	register int	size = siz;

	do {
		*result++ = *left++ & *right++;
	} while (--size);
	return result0;
}
