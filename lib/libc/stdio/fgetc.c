#ifndef lint
static char sccsid[] = "@(#)fgetc.c	5.1 (Berkeley) 6/5/85";
#endif not lint

#include <stdio.h>

fgetc(fp)
FILE *fp;
{
	return(getc(fp));
}
