/*-
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.proprietary.c%
 */

#ifndef lint
static char sccsid[] = "@(#)circle.c	8.1 (Berkeley) 6/4/93";
#endif /* not lint */

circle(x,y,r){
	arc(x,y,x+r,y,x+r,y);
}
