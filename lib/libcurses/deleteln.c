/*
 * Copyright (c) 1981 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)deleteln.c	5.12 (Berkeley) 10/26/92";
#endif	/* not lint */

#include <curses.h>
#include <string.h>

/*
 * wdeleteln --
 *	Delete a line from the screen.  It leaves (cury, curx) unchanged.
 */
int
wdeleteln(win)
	register WINDOW *win;
{
	register int y, i;
	register __LINE *temp;

#ifdef DEBUG
	__TRACE("deleteln: (%0.2o)\n", win);
#endif
	temp = win->lines[win->cury];
	for (y = win->cury; y < win->maxy - 1; y++) {
		win->lines[y]->flags &= ~__ISPASTEOL;
		win->lines[y + 1]->flags &= ~__ISPASTEOL;
		if (win->orig == NULL)
			win->lines[y] = win->lines[y + 1];
		else
			bcopy(win->lines[y + 1]->line, win->lines[y]->line, 
			      win->maxx * __LDATASIZE);
		touchline(win, y, 0, win->maxx - 1);
	}

	if (win->orig == NULL)
		win->lines[y] = temp;
	else
		temp = win->lines[y];

	for(i = 0; i < win->maxx; i++) {
		temp->line[i].ch = ' ';
		temp->line[i].attr = 0;
	}
	touchline(win, y, 0, win->maxx - 1);
	if (win->orig == NULL)
		__id_subwins(win);
	return (OK);
}
