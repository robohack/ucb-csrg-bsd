# Copyright (c) 1985, 1993
#	The Regents of the University of California.  All rights reserved.
#
# %sccs.include.redist.sh%
#
#	@(#)infnan.s	8.1 (Berkeley) 6/4/93
#
	.data
	.align	2
_sccsid:
.asciz	"@(#)infnan.s	8.1	(ucb.elefunt)	6/4/93"

/*
 * double infnan(arg)
 * int arg;
 * where arg :=    EDOM	if result is  NaN
 *	     :=  ERANGE	if result is +INF
 *	     := -ERANGE if result is -INF
 *
 * The Reserved Operand Fault is generated inside of this routine.
 */	
	.globl	_infnan
	.set	EDOM,33
	.set	ERANGE,34
	.text
	.align 2
_infnan:
	.word	0x0000			# save nothing
	cmpl	4(fp),$ERANGE
	bneq	1f
	movl	$ERANGE,_errno
	brb	2f
1:	movl	$EDOM,_errno
2:	cmpf2	$0x80000000,$0x80000000	# generates the reserved operand fault
	ret
