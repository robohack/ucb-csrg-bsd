#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)sprintf.c	5.3 (Berkeley) 10/25/87";
#endif LIBC_SCCS and not lint

#include	<stdio.h>

sprintf(str, fmt, args)
char *str, *fmt;
{
	FILE _strbuf;
	int len;

	_strbuf._flag = _IOWRT+_IOSTRG;
	_strbuf._ptr = str;
	_strbuf._cnt = 32767;
	len = _doprnt(fmt, &args, &_strbuf);
	*_strbuf._ptr = 0;
	return(len);
}
