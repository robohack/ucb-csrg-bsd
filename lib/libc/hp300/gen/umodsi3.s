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
	.asciz "@(#)umodsi3.s	5.1 (Berkeley) 6/7/90"
#endif /* LIBC_SCCS and not lint */

#include "DEFS.h"

/* unsigned % unsigned */
ENTRY(__umodsi3)
	movel	sp@(4),d1
	divull	sp@(8),d0:d1
	rts
