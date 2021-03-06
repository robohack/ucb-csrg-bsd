/*-
 * Copyright (c) 1979, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)TEOF.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */

#include "h00vars.h"

bool
TEOF(filep)

	register struct iorec	*filep;
{
	if (filep->fblk >= MAXFILES || _actfile[filep->fblk] != filep ||
	    (filep->funit & FDEF)) {
		ERROR("Reference to an inactive file\n", 0);
		return;
	}
	if (filep->funit & (EOFF|FWRITE))
		return TRUE;
	IOSYNC(filep);
	if (filep->funit & EOFF)
		return TRUE;
	return FALSE;
}
