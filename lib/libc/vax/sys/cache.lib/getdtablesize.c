/*-
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
	.asciz "@(#)getdtablesize.c	8.1 (Berkeley) 6/4/93"
#endif /* LIBC_SCCS and not lint */

#include "SYS.h"

	.data
dtablesize:
	.long	0
	.text

ENTRY(getdtablesize)
	movl	dtablesize,r0	# check cache
	beql	doit
	ret
doit:
	chmk	$SYS_getdtablesize
	jcs	err
	movl	r0,dtablesize	# set cache
	ret			# dtablesize = dtablesize();
err:
	jmp cerror;
