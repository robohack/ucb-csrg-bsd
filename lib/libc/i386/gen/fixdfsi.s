/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
	.asciz "@(#)fixdfsi.s	8.1 (Berkeley) 6/4/93"
#endif /* LIBC_SCCS and not lint */

	.globl ___fixdfsi
___fixdfsi:
	fldl	4(%esp)
	fistpl	4(%esp)
	movl	4(%esp),%eax
	ret
