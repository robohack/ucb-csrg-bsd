/*-
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)runtime.h	8.1 (Berkeley) 6/6/93
 */

/*
 * Definitions for the runtime environment.
 *
 * In general, runtime organization is language, machine, and
 * even compiler dependent.
 */

BOOLEAN isactive();		/* tell if a symbol is currently active */
ADDRESS entry_addr();		/* entry address of current proc/func */
ADDRESS entry_point();		/* point where environment is setup */
ADDRESS return_addr();		/* return address of current proc/func */
ADDRESS caller_addr();		/* entry of caller of current proc/func */
int where();			/* print out currently active procedures */
int dump();			/* dump the world */
int callproc();			/* call a procedure */
int procreturn();		/* return from a "call"-ed procedure */
ADDRESS address();		/* address of a variable */
ADDRESS firstline();		/* address of first line in a procedure */
int findbeginning();		/* find address of beginning of a procedure */
int runtofirst();		/* step to first line in current procedure */
ADDRESS lastaddr();		/* address of last line in program */
ADDRESS fparamaddr();		/* entry address of a function parameter */
