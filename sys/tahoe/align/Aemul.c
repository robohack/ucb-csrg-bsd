/*-
 * Copyright (c) 1986 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Computer Consoles Inc.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)Aemul.c	7.1 (Berkeley) 12/6/90
 */

#include "align.h"
emul(infop)	process_info *infop;
/*
/*	Extended precision multiplication.
/*
/***************************************/
{
	register long Register_12;	/* multiplicand */
	register long Register_11;	/* product least */
	register long Register_10;	/* product most */
	register long Register_9;	/* addend */
	register long Register_8;	/* multiplier */
	quadword result;

	Register_8 = operand(infop, 0)->data;
	Register_12 = operand(infop, 1)->data;
	Register_9 = operand(infop, 2)->data;
	Register_10=psl;
	Set_psl(r10);	/* restore the user psl */
	asm ("	emul	r8,r12,r9,r10");
	asm ("	movpsl	r8");
	New_cc (Register_8);
	result.high = Register_10;
	result.low  = Register_11;
	write_quadword (infop, result, operand(infop, 3));
}
