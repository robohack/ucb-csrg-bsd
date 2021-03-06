/*-
 * Copyright (c) 1992 Diomidis Spinellis.
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Diomidis Spinellis of Imperial College, University of London.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)extern.h	8.1 (Berkeley) 6/6/93
 */

extern struct s_command *prog;
extern struct s_appends *appends;
extern regmatch_t *match;
extern size_t maxnsub;
extern u_long linenum;
extern int appendnum;
extern int lastline;
extern int aflag, eflag, nflag;
extern char *fname;

void	 cfclose __P((struct s_command *, struct s_command *));
void	 compile __P((void));
void	 cspace __P((SPACE *, char *, size_t, enum e_spflag));
char	*cu_fgets __P((char *, int));
void	 err __P((int, const char *, ...));
int	 mf_fgets __P((SPACE *, enum e_spflag));
void	 process __P((void));
char	*strregerror __P((int, regex_t *));
void	*xmalloc __P((u_int));
void	*xrealloc __P((void *, u_int));
