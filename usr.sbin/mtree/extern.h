/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)extern.h	8.1 (Berkeley) 6/6/93
 */

int	 compare __P((char *, NODE *, FTSENT *));
int	 crc __P((int, u_long *, u_long *));
void	 cwalk __P((void));
void	 err __P((const char *, ...));
char	*inotype __P((u_int));
u_int	 parsekey __P((char *, int *));
char	*rlink __P((char *));
NODE	*spec __P((void));
int	 verify __P((void));
