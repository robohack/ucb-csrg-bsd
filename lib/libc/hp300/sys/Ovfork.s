/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
	.asciz "@(#)Ovfork.s	8.1 (Berkeley) 6/7/93"
#endif /* LIBC_SCCS and not lint */

/*
 * pid = vfork();
 *
 * d1 == 0 in parent process, d1 == 1 in child process.
 * d0 == pid of child in parent, d0 == pid of parent in child.
 *
 * trickery here, due to keith sklower, uses ret to clear the stack,
 * and then returns with a jump indirect, since only one person can return
 * with a ret off this stack... we do the ret before we vfork!
 */

#include "SYS.h"

	vfork = 66
ENTRY(vfork)
	movl	sp@+,a0
	movl	#vfork,d0
	trap	#0
	jcc	vforkok
	jmp	verror
vforkok:
	tstl	d1		/* child process ? */
	jne	child		/* yes */
	jcc 	parent		/* if c-bit not set, fork ok */
	.globl	_errno
verror:
	movl	d0,_errno
	moveq	#-1,d0
	jmp	a0@
child:
	clrl	d0
parent:
	jmp	a0@
