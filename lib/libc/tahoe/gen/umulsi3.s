/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
	.asciz "@(#)umulsi3.s	8.1 (Berkeley) 6/4/93"
#endif /* LIBC_SCCS and not lint */

#include "DEFS.h"

ENTRY(__umulsi3, 0)
	emul	8(fp),4(fp),$0,r0
	movl	r1,r0
	ret
