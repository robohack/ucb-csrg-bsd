/*
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
	.asciz "@(#)ntohl.s	8.1 (Berkeley) 6/4/93"
#endif /* LIBC_SCCS and not lint */

/* hostorder = ntohl(netorder) */

#include "DEFS.h"

ENTRY(ntohl, 0)
	movl	4(fp),r0
	ret
