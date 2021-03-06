/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)trap.h	8.1 (Berkeley) 6/11/93
 */

/*
 * Trap type values
 * also known in trap.c for name strings
 */

#define	T_RESADFLT	0	/* reserved addressing */
#define	T_PRIVINFLT	1	/* privileged instruction */
#define	T_RESOPFLT	2	/* reserved operand */
#define	T_BPTFLT	3	/* breakpoint instruction */
#define	T_SYSCALL	5	/* system call (kcall) */
#define	T_ARITHTRAP	6	/* arithmetic trap */
#define	T_ASTFLT	7	/* system forced exception */
#define	T_SEGFLT	8	/* segmentation (limit) fault */
#define	T_PROTFLT	9	/* protection fault */
#define	T_TRCTRAP	10	/* trace trap */
#define	T_PAGEFLT	12	/* page fault */
#define	T_TABLEFLT	13	/* page table fault */
#define	T_ALIGNFLT	14	/* alignment fault */
#define	T_KSPNOTVAL	15	/* kernel stack pointer not valid */
#define	T_BUSERR	16	/* bus error */
#define	T_KDBTRAP	17	/* kernel debugger trap */

#define	T_DIVIDE	18	/* integer divide fault */
#define	T_NMI		19	/* non-maskable trap */
#define	T_OFLOW		20	/* overflow trap */
#define	T_BOUND		21	/* bound instruction fault */
#define	T_DNA		22	/* device not available fault */
#define	T_DOUBLEFLT	23	/* double fault */
#define	T_FPOPFLT	24	/* fp coprocessor operand fetch fault */
#define	T_TSSFLT	25	/* invalid tss fault */
#define	T_SEGNPFLT	26	/* segment not present fault */
#define	T_STKFLT	27	/* stack fault */
#define	T_RESERVED	28	/* reserved fault base */

/* definitions for <sys/signal.h> */
#define	    ILL_RESAD_FAULT	T_RESADFLT
#define	    ILL_PRIVIN_FAULT	T_PRIVINFLT
#define	    ILL_RESOP_FAULT	T_RESOPFLT
#define	    ILL_ALIGN_FAULT	T_ALIGNFLT
#define	    ILL_FPOP_FAULT	T_FPOPFLT	/* coprocessor operand fault */

/* codes for SIGFPE/ARITHTRAP */
#define	    FPE_INTOVF_TRAP	0x1	/* integer overflow */
#define	    FPE_INTDIV_TRAP	0x2	/* integer divide by zero */
#define	    FPE_FLTDIV_TRAP	0x3	/* floating/decimal divide by zero */
#define	    FPE_FLTOVF_TRAP	0x4	/* floating overflow */
#define	    FPE_FLTUND_TRAP	0x5	/* floating underflow */
#define	    FPE_FPU_NP_TRAP	0x6	/* floating point unit not present */
#define	    FPE_SUBRNG_TRAP	0x7	/* subrange out of bounds */

/* codes for SIGBUS */
#define	    BUS_PAGE_FAULT	T_PAGEFLT	/* page fault protection base */
#define	    BUS_SEGNP_FAULT	T_SEGNPFLT	/* segment not present */
#define	    BUS_STK_FAULT	T_STKFLT	/* stack segment */
#define	    BUS_SEGM_FAULT	T_RESERVED	/* segment protection base */

/* Trap's coming from user mode */
#define	T_USER	0x100
