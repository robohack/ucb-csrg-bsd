/*-
 * Copyright (c) 1985, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.proprietary.c%
 */

#ifndef lint
static char sccsid[] = "@(#)erase.c	8.1 (Berkeley) 6/4/93";
#endif /* not lint */

#include "imPcodes.h"
#include "imp.h"
erase(){
	int i;
		putch(imP_ENDPAGE);
		imPx = imPy = 0;
		return;
}
