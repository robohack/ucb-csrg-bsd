/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ozan Yigit at York University.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)pathnames.h	8.1 (Berkeley) 6/6/93
 */

/*
 * Definitions of diversion files.  If the name of the file is changed,
 * adjust UNIQUE to point to the wildcard (*) character in the filename.
 */

#ifdef msdos
#define _PATH_DIVNAME	"\\M4*XXXXXX"		/* msdos diversion files */
#define	UNIQUE		3			/* unique char location */
#endif

#ifdef unix
#define _PATH_DIVNAME	"/tmp/m4.0XXXXXX"	/* unix diversion files */
#define UNIQUE		8			/* unique char location */
#endif

#ifdef vms
#define _PATH_DIVNAME	"sys$login:m4*XXXXXX"	/* vms diversion files */
#define UNIQUE		12			/* unique char location */
#endif
