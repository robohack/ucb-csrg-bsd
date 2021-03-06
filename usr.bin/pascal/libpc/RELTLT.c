/*-
 * Copyright (c) 1979, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)RELTLT.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */

#include "h00vars.h"

bool
RELTLT(bytecnt, left, right)

	long		bytecnt;
	register long	*left;
	register long	*right;
{
	register int	longcnt;

	longcnt = bytecnt >> 2;
	do	{
		if ((*left & ~*right) != 0)
			return FALSE;
		if ((*right++ & ~*left++) != 0)
			goto leq;
	} while (--longcnt);
	return FALSE;
leq:
	while (--longcnt) {
		if ((*left++ & ~*right++) != 0) 
			return FALSE;
	}
	return TRUE;
}
