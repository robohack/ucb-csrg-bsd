/*-
 * Copyright (c) 1986 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Computer Consoles Inc.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)Aaddd.c	7.1 (Berkeley) 12/6/90
 */

#include "align.h"

addd(infop)	process_info *infop;
/*
/*	Add operand with accumulator to accumulator (double).
/*
/*************************************************************/
{
	register double 	*operand_pnt;
	register double		*acc_pnt;

	operand_pnt = (double *)&operand(infop,0)->data;
	acc_pnt = (double *) &acc_high;
	*acc_pnt = *acc_pnt + *operand_pnt;
}

