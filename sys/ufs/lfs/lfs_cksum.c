/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)lfs_cksum.c	7.2 (Berkeley) 12/6/91
 */

#include <sys/param.h>

#include <ufs/lfs/lfs.h>
#include <ufs/lfs/lfs_extern.h>

/*
 * Simple, general purpose, fast checksum.  Data must be short-aligned.
 * Returns a u_long in case we ever want to do something more rigorous.
 *
 * XXX
 * Use the TCP/IP checksum instead.
 */
u_long
cksum(str, len)
	register void *str;
	register size_t len;
{
	register u_long sum;
	
	len &= ~(sizeof(u_short) - 1);
	for (sum = 0; len; len -= sizeof(u_short))
		sum ^= *((u_short *)str)++;
	return (sum);
}
