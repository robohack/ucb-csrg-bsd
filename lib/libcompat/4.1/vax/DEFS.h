/*-
 * Copyright (c) 1983 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.proprietary.c%
 *
 *	@(#)DEFS.h	5.2 (Berkeley) 4/12/91
 */

#ifdef PROF
#define	ENTRY(x)	.globl _/**/x; .align 2; _/**/x: .word 0; \
			.data; 1:; .long 0; .text; moval 1b,r0; jsb mcount
#define	ASENTRY(x)	.globl x; .align 2; x: .word 0; \
			.data; 1:; .long 0; .text; moval 1b,r0; jsb mcount
#else
#define	ENTRY(x)	.globl _/**/x; .align 2; _/**/x: .word 0
#define	ASENTRY(x)	.globl x; .align 2; x: .word 0
#endif
