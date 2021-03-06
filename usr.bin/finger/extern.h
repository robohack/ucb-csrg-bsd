/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)extern.h	8.2 (Berkeley) 4/28/95
 */

extern char tbuf[1024];			/* Temp buffer for anybody. */
extern int entries;			/* Number of people. */
extern DB *db;				/* Database. */

void	 enter_lastlog __P((PERSON *));
PERSON	*enter_person __P((struct passwd *));
void	 enter_where __P((struct utmp *, PERSON *));
PERSON	*find_person __P((char *));
void	 lflag_print __P((void));
int	 match __P((struct passwd *, char *));
void	 netfinger __P((char *));
PERSON	*palloc __P((void));
char	*prphone __P((char *));
void	 sflag_print __P((void));
