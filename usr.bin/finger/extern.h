/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)extern.h	5.1 (Berkeley) 7/27/91
 */

extern PERSON *htab[HSIZE];		/* The buckets. */
extern PERSON *phead, *ptail;		/* The linked list of all people. */
extern char tbuf[1024];			/* Temp buffer for anybody. */
extern int entries;			/* Number of people. */

void	 enter_lastlog __P((PERSON *));
PERSON	*enter_person __P((struct passwd *));
void	 enter_where __P((struct utmp *, PERSON *));
void	 err __P((const char *, ...));
PERSON	*find_person __P((char *));
void	 lflag_print __P((void));
int	 match __P((struct passwd *, char *));
void	 netfinger __P((char *));
PERSON	*palloc __P((void));
char	*prphone __P((char *));
void	 sflag_print __P((void));
WHERE	*walloc __P((PERSON *));
