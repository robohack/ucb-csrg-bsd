#ifndef lint
static char sccsid[] = "@(#)fputc.c	5.1 (Berkeley) 6/5/85";
#endif not lint

#include <stdio.h>

fputc(c, fp)
register FILE *fp;
{
	return(putc(c, fp));
}
