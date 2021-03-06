/*-
 * Copyright (c) 1986 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Computer Consoles Inc.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)Amull3.c	7.1 (Berkeley) 12/6/90
 */

#include "align.h" 
mull3(infop)	process_info *infop;
/*
/*	Multiply longwords, 3 operands.
/*
/*****************************************/
{
	register	long	Register_12;	/* Has to be first reg ! */
	register	long	result, data0, data1;

	data0 = operand(infop,0)->data; 
	data1 = operand(infop,1)->data; 
	Register_12=psl;
	Set_psl(r12);	/* restore the user psl */
	result = data0 * data1;
	asm ("	movpsl	r12");
	New_cc (Register_12);
	write_back (infop,result, operand(infop,2) );
}
