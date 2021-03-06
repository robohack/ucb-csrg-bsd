/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#if defined(SYSLIBC_SCCS) && !defined(lint)
	.asciz "@(#)sigprocmask.s	8.1 (Berkeley) 6/4/93"
#endif /* SYSLIBC_SCCS and not lint */

#include "SYS.h"

err:
	jmp	cerror

ENTRY(sigprocmask)
	tstl	8(fp)			# check new sigset pointer
	bneq	1f			# if not null, indirect
	movl	$0,8(fp)		# null mask pointer: block empty set
	movl	$1,4(fp)		# SIG_BLOCK
	jbr	2f
1:	movl	*8(fp),8(fp)		# indirect to new mask arg
2:	kcall	$SYS_sigprocmask
	jcs	err
	tstl	12(fp)			# test if old mask requested
	beql	out
	movl	r0,*12(fp)		# store old mask
out:
	clrl	r0
	ret
