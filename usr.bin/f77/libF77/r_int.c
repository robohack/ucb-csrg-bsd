/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)r_int.c	5.1	6/7/85
 */

double r_int(x)
float *x;
{
double floor();

return( (*x >= 0) ? floor(*x) : -floor(- *x) );
}
