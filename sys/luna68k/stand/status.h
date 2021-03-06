/*
 * Copyright (c) 1992 OMRON Corporation.
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * OMRON Corporation.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)status.h	8.1 (Berkeley) 6/10/93
 */

/*
 * status.h -- status code table for internal commands
 * by A.Fujita, FEB-02-1992
 */


#define ST_NORMAL	0
#define ST_EXIT		-1
#define ST_NOTFOUND	-2

#define ST_ERROR	1
