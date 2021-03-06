/*-
 * Copyright (c) 1986 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Computer Consoles Inc.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)Aaoblss.c	7.1 (Berkeley) 12/6/90
 */

#include "align.h" 
aoblss(infop)	process_info *infop;
/*
/*	Add one, branch if less than.
/*	Can't use real HW opcode, don't want to branch out of here !
/*
/*******************************************/
{
	register long limit, index, new_address, complement;

	limit = operand(infop,0)->data;
	index = operand(infop,1)->data;
	complement =  limit + ~index;
	if ( complement < 0){ carry_0; negative_1;}else{carry_1; negative_0;}
	if ( complement == 0) zero_1; else zero_0;
	overflow_0;
	write_back (infop,index+1, operand(infop,1));
	new_address = operand(infop,2)->address;
	if (!negative && !zero) pc = new_address;
}
