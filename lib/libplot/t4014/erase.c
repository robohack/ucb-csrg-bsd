/*-
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.proprietary.c%
 */

#ifndef lint
static char sccsid[] = "@(#)erase.c	8.1 (Berkeley) 6/4/93";
#endif /* not lint */

extern int ohiy;
extern int ohix;
extern int oloy;
extern int oextra;
erase(){
	int i;
		putch(033);
		putch(014);
		ohiy= -1;
		ohix = -1;
		oextra = -1;
		oloy = -1;
		return;
}
