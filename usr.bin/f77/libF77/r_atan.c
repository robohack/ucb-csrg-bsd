/*-
 * Copyright (c) 1980 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.proprietary.c%
 */

#ifndef lint
static char sccsid[] = "@(#)r_atan.c	5.3 (Berkeley) 4/12/91";
#endif /* not lint */

float r_atan(x)
float *x;
{
double atan();
return( atan(*x) );
}
