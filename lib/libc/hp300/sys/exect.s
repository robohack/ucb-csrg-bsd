/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
	.asciz "@(#)exect.s	8.1 (Berkeley) 6/7/93"
#endif /* LIBC_SCCS and not lint */

#include "SYS.h"
#include <machine/psl.h>

ENTRY(exect)
	movl	#SYS_execve,d0
	trap	#0
	jmp	cerror		/* exect(file, argv, env) */
