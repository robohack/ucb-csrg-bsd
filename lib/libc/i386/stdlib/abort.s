/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
	.asciz "@(#)abort.s	5.1 (Berkeley) 4/23/90"
#endif /* LIBC_SCCS and not lint */

#include "DEFS.h"

ENTRY(abort)
	hlt
	/* NOTREACHED */
