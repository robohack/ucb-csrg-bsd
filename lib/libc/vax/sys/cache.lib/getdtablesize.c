/*
 * Copyright (c) 1983 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
	.asciz "@(#)getdtablesize.c	5.1 (Berkeley) 4/12/91"
#endif /* LIBC_SCCS and not lint */

#include "SYS.h"

	.data
dtablesize:
	.long	0
	.text

ENTRY(getdtablesize)
	movl	dtablesize,r0	# check cache
	beql	doit
	ret
doit:
	chmk	$SYS_getdtablesize
	jcs	err
	movl	r0,dtablesize	# set cache
	ret			# dtablesize = dtablesize();
err:
	jmp cerror;
