#ifndef lint
static char sccsid[] = "@(#)getchar.c	5.1 (Berkeley) 6/5/85";
#endif not lint

/*
 * A subroutine version of the macro getchar.
 */
#include <stdio.h>

#undef getchar

getchar()
{
	return(getc(stdin));
}
