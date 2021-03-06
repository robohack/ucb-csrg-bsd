/*
 * Copyright (c) 1981, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)idlok.c	8.2 (Berkeley) 5/4/94";
#endif	/* not lint */

#include "curses.h"

/*
 * idlok --
 *	Turn on and off using insert/deleteln sequences for the
 *	given window.
 */
void
idlok(win, bf)
	WINDOW *win;
	int bf;
{
	if (bf)
		win->flags |= __IDLINE;
	else
		win->flags &= ~__IDLINE;
}
