#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)frexp.c	5.3 (Berkeley) 2/23/91";
#endif LIBC_SCCS and not lint

#include <math.h>

/*
 *	the call
 *		x = frexp(arg,&exp);
 *	must return a double fp quantity x which is <1.0
 *	and the corresponding binary exponent "exp".
 *	such that
 *		arg = x*2^exp
 *	if the argument is 0.0, return 0.0 mantissa and 0 exponent.
 */

double
frexp(x,i)
double x;
int *i;
{
	int neg;
	int j;
	j = 0;
	neg = 0;
	if(x<0){
		x = -x;
		neg = 1;
		}
	if(x>=1.0)
		while(x>=1.0){
			j = j+1;
			x = x/2;
			}
	else if(x<0.5 && x != 0.0)
		while(x<0.5){
			j = j-1;
			x = 2*x;
			}
	*i = j;
	if(neg) x = -x;
	return(x);
	}
