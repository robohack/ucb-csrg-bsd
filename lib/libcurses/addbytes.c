/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)addbytes.c	5.16 (Berkeley) 1/11/93";
#endif	/* not lint */

#include <curses.h>
#include <termios.h>

#define	SYNCH_IN	{y = win->cury; x = win->curx;}
#define	SYNCH_OUT	{win->cury = y; win->curx = x;}

int
waddbytes(win, bytes, count)
	WINDOW *win;
	char *bytes;
	int count;
{
	__waddbytes(win, bytes, count, 0);
}

/*
 * waddbytes --
 *	Add the character to the current position in the given window.
 */
int
__waddbytes(win, bytes, count, so)
	register WINDOW *win;
	register char *bytes;
	register int count;
	int so;
{
	static char blanks[] = "        ";
	register int c, newx, x, y;
	char stand;
	__LINE *lp;

	SYNCH_IN;

#ifdef DEBUG
	__TRACE("ADDBYTES('%c') at (%d, %d)\n", c, y, x);
#endif
	while (count--) {
		c = *bytes++;
		switch (c) {
		case '\t':
			SYNCH_OUT;
			if (waddbytes(win, blanks, 8 - (x % 8)) == ERR)
				return (ERR);
			SYNCH_IN;
			break;

		default:
#ifdef DEBUG
	__TRACE("ADDBYTES(%0.2o, %d, %d)\n", win, y, x);
#endif
			
			lp = win->lines[y];
			if (lp->flags & __ISPASTEOL) {
				lp->flags &= ~__ISPASTEOL;
newline:			if (y == win->maxy - 1) {
					if (win->flags & __SCROLLOK) {
						SYNCH_OUT;
						scroll(win);
						SYNCH_IN;
						lp = win->lines[y];
					        x = 0;
					} 
				} else {
					y++;
					lp = win->lines[y];
					x = 0;
				}
			}
				
			stand = '\0';
			if (win->flags & __WSTANDOUT || so)
				stand |= __STANDOUT;
#ifdef DEBUG
	__TRACE("ADDBYTES: 1: y = %d, x = %d, firstch = %d, lastch = %d\n",
	    y, x, *win->lines[y]->firstchp, *win->lines[y]->lastchp);
#endif
			if (lp->line[x].ch != c || 
			    !(lp->line[x].attr & stand)) {
				newx = x + win->ch_off;
				if (!(lp->flags & __ISDIRTY)) {
					lp->flags |= __ISDIRTY;
					*lp->firstchp = *lp->lastchp = newx;
				}
				else if (newx < *lp->firstchp)
					*lp->firstchp = newx;
				else if (newx > *lp->lastchp)
					*lp->lastchp = newx;
#ifdef DEBUG
	__TRACE("ADDBYTES: change gives f/l: %d/%d [%d/%d]\n",
	    *lp->firstchp, *lp->lastchp,
	    *lp->firstchp - win->ch_off,
	    *lp->lastchp - win->ch_off);
#endif
			}
			lp->line[x].ch = c;
			if (stand)
				lp->line[x].attr |= __STANDOUT;
			else
				lp->line[x].attr &= ~__STANDOUT;
			if (x == win->maxx - 1)
				lp->flags |= __ISPASTEOL;
			else
				x++;
#ifdef DEBUG
	__TRACE("ADDBYTES: 2: y = %d, x = %d, firstch = %d, lastch = %d\n",
	    y, x, *win->lines[y]->firstchp, *win->lines[y]->lastchp);
#endif
			break;
		case '\n':
			SYNCH_OUT;
			wclrtoeol(win);
			SYNCH_IN;
			if (origtermio.c_oflag & ONLCR)
				x = 0;
			goto newline;
		case '\r':
			x = 0;
			break;
		case '\b':
			if (--x < 0)
				x = 0;
			break;
		}
	}
	SYNCH_OUT;
	return (OK);
}
