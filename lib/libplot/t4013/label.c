#ifndef lint
static char sccsid[] = "@(#)label.c	1.1 (Berkeley) 1/2/85";
#endif

label(s)
char *s;
{
	register i,c;
	putch(037);	/* alpha mode */
	for(i=0; c=s[i]; i++)
		putch(c);
}
