#ifndef lint
static char sccsid[] = "@(#)putchar.c	5.1 (Berkeley) 6/5/85";
#endif not lint

/*
 * A subroutine version of the macro putchar
 */
#include <stdio.h>

#undef putchar

putchar(c)
register c;
{
	putc(c, stdout);
}
