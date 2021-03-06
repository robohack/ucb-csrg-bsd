/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
 *
 * %sccs.include.redist.c%
 */

#if defined(SYSLIBC_SCCS) && !defined(lint)
	.asciz "@(#)exect.s	8.1 (Berkeley) 6/4/93"
#endif /* SYSLIBC_SCCS and not lint */

#include "SYS.h"
#include <machine/psl.h>

ENTRY(exect)
	lea	SYS_execve,%eax
	pushf
	popl	%edx
	orl	$ PSL_T,%edx
	pushl	%edx
	popf
	LCALL(7,0)
	jmp	cerror		/* exect(file, argv, env); */
