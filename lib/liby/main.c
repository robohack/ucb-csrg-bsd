/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)main.c	8.1 (Berkeley) 6/4/93";
#endif /* not lint */

main()
{
	exit(yyparse());
}
