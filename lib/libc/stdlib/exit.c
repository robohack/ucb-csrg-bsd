#ifndef lint
static char sccsid[] = "@(#)exit.c	5.1 (Berkeley) 6/5/85";
#endif not lint

exit(code)
	int code;
{

	_cleanup();
	_exit(code);
}
