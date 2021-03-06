/*-
 * Copyright (c) 1979, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)PFLUSH.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */

#include "h00vars.h"

/*
 * insure that a usable image is in the buffer window
 */
PFLUSH()
{
	register struct iorec	*next;

	for (next = _fchain.fchain; next != FILNIL; next = next->fchain) {
		if ((next->funit & (FDEF | FREAD)) != 0)
			continue;
		if (next->fbuf != 0)
			fflush(next->fbuf);
	}
}
