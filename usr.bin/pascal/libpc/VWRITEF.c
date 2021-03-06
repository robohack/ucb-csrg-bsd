/*-
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)VWRITEF.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */

#include "h00vars.h"

#include <stdarg.h>

VWRITEF(curfile, file, format, args)

	register struct iorec	*curfile;
	FILE			*file;
	char 			*format;
	va_list			args;
{

	if (curfile->funit & FREAD) {
		ERROR("%s: Attempt to write, but open for reading\n",
			curfile->pfname);
		return;
	}
	vfprintf(file, format, args);
	if (ferror(curfile->fbuf)) {
		PERROR("Could not write to ", curfile->pfname);
		return;
	}
}
