/*-
 * Copyright (c) 1985, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.proprietary.c%
 */

#ifndef lint
static char sccsid[] = "@(#)line.c	8.1 (Berkeley) 6/4/93";
#endif /* not lint */

line(x0,y0,x1,y1){
	move(x0,y0);
	cont(x1,y1);
}
