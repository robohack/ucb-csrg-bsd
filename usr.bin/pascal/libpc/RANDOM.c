/*-
 * Copyright (c) 1979, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)RANDOM.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */

#include "h00vars.h"

extern long RAND();

double
RANDOM()
{
	double d;
	long l;

	/*
	 * calculate (1103515245 * seed) mod 2^31-1
	 */
	d = 1103515245.0 * _seed / 2147483647.0;
	l = d;
	d = d - l;
	_seed = d * 2147483647.0;
	/*
	 * want a value in the range 0..1
	 */
	return(d);
}
