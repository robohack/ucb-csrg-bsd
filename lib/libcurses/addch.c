# include	"curses.ext"

/*
 *	This routine adds the character to the current position
 *
 * 5/9/83 (Berkeley) @(#)addch.c	1.4
 */
waddch(win, c)
reg WINDOW	*win;
char		c;
{
	reg int		x, y;
	reg WINDOW	*wp;

	x = win->_curx;
	y = win->_cury;
# ifdef FULLDEBUG
	fprintf(outf, "ADDCH('%c') at (%d, %d)\n", c, y, x);
# endif
	if (y >= win->_maxy || x >= win->_maxx || y < 0 || x < 0)
		return ERR;
	switch (c) {
	  case '\t':
	  {
		reg int		newx;

		for (newx = x + (8 - (x & 07)); x < newx; x++)
			if (waddch(win, ' ') == ERR)
				return ERR;
		return OK;
	  }

	  default:
# ifdef FULLDEBUG
		fprintf(outf, "ADDCH: 1: y = %d, x = %d, firstch = %d, lastch = %d\n", y, x, win->_firstch[y], win->_lastch[y]);
# endif
		if (win->_flags & _STANDOUT)
			c |= _STANDOUT;
		set_ch(win, y, x, c);
		for (wp = win->_nextp; wp != win; wp = wp->_nextp)
			set_ch(wp, y, x, c);
		win->_y[y][x++] = c;
		if (x >= win->_maxx) {
			x = 0;
newline:
			if (++y >= win->_maxy)
				if (win->_scroll) {
					wrefresh(win);
					scroll(win);
					--y;
				}
				else
					return ERR;
		}
# ifdef FULLDEBUG
		fprintf(outf, "ADDCH: 2: y = %d, x = %d, firstch = %d, lastch = %d\n", y, x, win->_firstch[y], win->_lastch[y]);
# endif
		break;
	  case '\n':
		wclrtoeol(win);
		if (!NONL)
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
	win->_curx = x;
	win->_cury = y;
	return OK;
}

/*
 * set_ch:
 *	Set the first and last change flags for this window.
 */
static
set_ch(win, y, x, ch)
reg WINDOW	*win;
int		y, x; {

	if (win->_orig != NULL) {
		y += win->_begy - win->_orig->_begy;
		x += win->_begx - win->_orig->_begx;
	}
	if (win->_y[y][x] != ch) {
		if (win->_firstch[y] == _NOCHANGE)
			win->_firstch[y] = win->_lastch[y] = x;
		else if (x < win->_firstch[y])
			win->_firstch[y] = x;
		else if (x > win->_lastch[y])
			win->_lastch[y] = x;
	}
}
