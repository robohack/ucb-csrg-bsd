/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Michael Fischbein.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)ls.h	5.2 (Berkeley) 6/24/89
 */

typedef struct _lsstruct {
	char *name;			/* file name */
	struct stat lstat;		/* lstat(2) for file */
} LS;

/* flags */
extern int f_listdot;		/* list files beginning with . */
extern int f_listalldot;	/* list . and .. as well */
extern int f_modtime;		/* use time of last change for time */
extern int f_accesstime;	/* use time of last access */
extern int f_statustime;	/* use time of last mode change */
extern int f_singlecol;		/* use single column output */
extern int f_listdir;		/* list actual directory, not contents */
extern int f_specialdir;	/* force operands to be directories */
extern int f_type;		/* add type character for non-regular files */
extern int f_group;		/* show group ownership of a file */
extern int f_inode;		/* print inode */
extern int f_longform;		/* long listing format */
extern int f_nonprint;		/* show unprintables as ? */
extern int f_reversesort;	/* reverse whatever sort is used */
extern int f_recursive;		/* ls subdirectories also */
extern int f_size;		/* list size in short listing */
extern int f_timesort;		/* sort by time vice name */
extern int f_firsttime;		/* to control recursion */

extern int errno;
