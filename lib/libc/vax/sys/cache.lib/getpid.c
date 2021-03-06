/*-
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
	.asciz "@(#)getpid.c	8.1 (Berkeley) 6/4/93"
#endif /* LIBC_SCCS and not lint */

#include "SYS.h"

	.data
	.globl	mypid, myppid
mypid:
	.long	0
	.text

ENTRY(getpid)
	movl	mypid,r0	# check cache
	beql	doit
	ret
doit:
	chmk	$SYS_getpid
	jcs	err
	movl	r0,mypid	# set cache
	movl	r1,myppid	# set cache
	ret			# pid = getpid();
err:
	jmp cerror;
