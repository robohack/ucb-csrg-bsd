/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
	.asciz "@(#)udiv.s	5.3 (Berkeley) 4/8/91"
#endif /* LIBC_SCCS and not lint */

/*
 * Unsigned division, PCC flavor.
 * udiv() takes an ordinary dividend/divisor pair;
 * audiv() takes a pointer to a dividend and an ordinary divisor.
 */

#include "DEFS.h"

#define	DIVIDEND	4(ap)
#define	DIVISOR		8(ap)

ASENTRY(udiv,0)
	movl	DIVISOR,r2
	jlss	Leasy		# big divisor: settle by comparison
	movl	DIVIDEND,r0
	jlss	Lhard		# big dividend: extended division
	divl2	r2,r0		# small divisor and dividend: signed division
	ret
Lhard:
	clrl	r1
	ediv	r2,r0,r0,r1
	ret
Leasy:
	cmpl	DIVIDEND,r2
	jgequ	Lone		# if dividend is as big or bigger, return 1
	clrl	r0		# else return 0
	ret
Lone:
	movl	$1,r0
	ret

ASENTRY(audiv,0)
	movl	DIVISOR,r2
	jlss	Laeasy		# big divisor: settle by comparison
	movl	DIVIDEND,r3
	movl	(r3),r0
	jlss	Lahard		# big dividend: extended division
	divl3	r2,r0,(r3)	# small divisor and dividend: signed division
	ret
Lahard:
	clrl	r1
	ediv	r2,r0,(r3),r1
	ret
Laeasy:
	cmpl	(r3),r2
	jgequ	Laone		# if dividend is as big or bigger, store 1
	clrl	(r3)		# else store 0
	ret
Laone:
	movl	$1,(r3)
	ret
