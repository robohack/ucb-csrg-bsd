/*
 * Copyright (c) 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Computer Consoles Inc.
 *
 * %sccs.include.redist.c%
 */

#if defined(SYSLIBC_SCCS) && !defined(lint)
	.asciz "@(#)cvtdl.s	8.1 (Berkeley) 6/4/93"
#endif /* SYSLIBC_SCCS and not lint */

#include <tahoemath/fp.h>
#include "DEFS.h"
 
XENTRY(cvtdu, R2|R3|R4|R5)
	jbr	1f

XENTRY(cvtdl, R2|R3|R4|R5)
 #
 #Some initializations:
 #
1:
	movl	4(fp),r0	# fetch operand.
	movl	8(fp),r1	
	clrl	r3		# r3 - negative flag.
 #
 #get exponent
 #
	andl3	$EXPMASK,r0,r2	# r2 will hold the exponent.
	jeql	is_reserved	# check for reserved operand. 
	cmpl	$ONE_EXP,r2	# if exponent is less then 1,return zero.
	jgtr	retzero
	andl2	$0!EXPSIGN,r2	# turn off biased exponent sign
	shrl	$EXPSHIFT,r2,r2
 #
 #get fraction
 #
	bbc	$31,r0,positive	# if negative remember it.
	incl	r3
positive:
				# clear the non fraction parts.
	andl2	$(0!(EXPMASK | SIGNBIT)),r0
				# add the hidden bit.
	orl2	$(0!CLEARHID),r0
	subl2	$24,r2		# compute the shift.
	jgtr	shift_left
	mnegl	r2,r2
	shrl	r2,r0,r0	# shift right.
	jmp	shifted
shift_left:
	cmpl	r2,$7
	jgtr	overflow
go_on:	shlq	r2,r0,r0	# shift right.
shifted:
	bbc	$0,r3,done	# check for negative
	mnegl	r0,r0
done:	
	ret

retzero:
	clrl	r0
	ret
overflow:
	callf	$4,fpover
	jmp	go_on

is_reserved:
	bbc	$31,r0,retzero

	callf	$4,fpresop
	ret
