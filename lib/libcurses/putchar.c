# include	"curses.ext"

/*
 * @(#)putchar.c	1.1 (Berkeley) 5/1/85
 */
char
_putchar(c)
reg char	c; {

	putchar(c);
#ifdef DEBUG
	fprintf(outf, "_PUTCHAR(%s)\n", unctrl(c));
#endif
}
