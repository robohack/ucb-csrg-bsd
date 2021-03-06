/*
 * Copyright (c) 1983 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#include "SYS.h"

#if defined(LIBC_SCCS) && !defined(lint)
	ASMSTR "@(#)getegid.s	5.8 (Berkeley) 5/30/90"
#endif /* LIBC_SCCS and not lint */

PSEUDO(getegid,getgid)
	movl	r1,r0
	ret		/* egid = getegid(); */
