#ifndef lint
static char sccsid[] = "@(#)circle.c	1.1 (Berkeley) 1/2/85";
#endif

circle(x,y,r){
	arc(x,y,x+r,y,x+r,y);
}
