#ifndef lint
static char sccsid[] = "@(#)clrerr.c	5.1 (Berkeley) 6/5/85";
#endif not lint

#include <stdio.h>
#undef	clearerr

clearerr(iop)
	register FILE *iop;
{
	iop->_flag &= ~(_IOERR|_IOEOF);
}
