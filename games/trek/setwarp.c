/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)setwarp.c	8.1 (Berkeley) 5/31/93";
#endif /* not lint */

# include	"trek.h"
# include	"getpar.h"

/*
**  SET WARP FACTOR
**
**	The warp factor is set for future move commands.  It is
**	checked for consistancy.
*/

setwarp()
{
	double	warpfac;

	warpfac = getfltpar("Warp factor");
	if (warpfac < 0.0)
		return;
	if (warpfac < 1.0)
		return (printf("Minimum warp speed is 1.0\n"));
	if (warpfac > 10.0)
		return (printf("Maximum speed is warp 10.0\n"));
	if (warpfac > 6.0)
		printf("Damage to warp engines may occur above warp 6.0\n");
	Ship.warp = warpfac;
	Ship.warp2 = Ship.warp * warpfac;
	Ship.warp3 = Ship.warp2 * warpfac;
}
