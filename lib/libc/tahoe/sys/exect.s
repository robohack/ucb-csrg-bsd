/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef SYSLIBC_SCCS
_sccsid:.asciz	"@(#)exect.s	5.1 (Berkeley) 7/2/86"
#endif SYSLIBC_SCCS

#include "SYS.h"
#include <machine/psl.h>

ENTRY(exect)
	bispsw	$PSL_T
	kcall	$SYS_execve
	jmp	cerror		# exect(file, argv, env)
