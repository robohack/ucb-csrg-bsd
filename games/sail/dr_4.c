/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)dr_4.c	8.2 (Berkeley) 4/28/95";
#endif /* not lint */

#include "extern.h"

ungrap(from, to)
register struct ship *from, *to;
{
	register k;
	char friend;

	if ((k = grappled2(from, to)) == 0)
		return;
	friend = capship(from)->nationality == capship(to)->nationality;
	while (--k >= 0) {
		if (friend || die() < 3) {
			cleangrapple(from, to, 0);
			makesignal(from, "ungrappling %s (%c%c)", to);
		}
	}
}

grap(from, to)
register struct ship *from, *to;
{
	if (capship(from)->nationality != capship(to)->nationality && die() > 2)
		return;
	Write(W_GRAP, from, 0, to->file->index, 0, 0, 0);
	Write(W_GRAP, to, 0, from->file->index, 0, 0, 0);
	makesignal(from, "grappled with %s (%c%c)", to);
}
