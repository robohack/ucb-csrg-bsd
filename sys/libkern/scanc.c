/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)scanc.c	8.1 (Berkeley) 6/10/93
 */

#include <libkern/libkern.h>

int
scanc(size, cp, table, mask0)
	u_int size;
	register u_char *cp, table[];
	int mask0;
{
	register u_char *end;
	register u_char mask;

	mask = mask0;
	for (end = &cp[size]; cp < end && (table[*cp] & mask) == 0; ++cp);
	return (end - cp);
}
