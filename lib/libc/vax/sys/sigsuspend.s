/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#if defined(SYSLIBC_SCCS) && !defined(lint)
	.asciz "@(#)sigsuspend.s	8.1 (Berkeley) 6/4/93"
#endif /* SYSLIBC_SCCS and not lint */

#include "SYS.h"

err:
	jmp	cerror

ENTRY(sigsuspend)
	movl	*4(ap),4(ap)		# indirect to mask arg
	chmk	$SYS_sigsuspend
	jcs	err
	clrl	r0			# shouldn't happen
	ret
