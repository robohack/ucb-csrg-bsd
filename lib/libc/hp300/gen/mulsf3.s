/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
	.asciz "@(#)mulsf3.s	5.1 (Berkeley) 6/7/90"
#endif /* LIBC_SCCS and not lint */

#include "DEFS.h"

/* single * single */
ENTRY(__mulsf3)
	fmoves	sp@(4),fp0
	fmuls	sp@(8),fp0
	fmoves	fp0,d0
	rts
