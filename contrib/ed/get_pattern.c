/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Rodney Ruddock of the University of Guelph.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)get_pattern.c	8.1 (Berkeley) 5/31/93";
#endif /* not lint */

#include <sys/types.h>

#include <regex.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DBI
#include <db.h>
#endif

#include "ed.h"
#include "extern.h"

/*
 * This is for getting RE and replacement patterns for any command
 * that uses RE's and replacements.
 */
char *
get_pattern(delim, inputt, errnum, flag)
	int delim, *errnum, flag;
	FILE *inputt;
{
	static int l_max = 510;
	int l_cnt = 1;
	char *l_pat, *l_pat_tmp;

	/* get a "reasonable amount of space for the RE */
	l_pat = calloc(l_max + 2, sizeof(char));
	if (l_pat == NULL) {
		*errnum = -3;
		strcpy(help_msg, "out of memory error");
		return (NULL);
	}
	l_pat[0] = delim;

	if ((delim == ' ') || (delim == '\n')) {
		if (delim == '\n')
			ungetc(delim, inputt);
		strcpy(help_msg, "illegal delimiter");
		*errnum = -2;
		return (l_pat);
	}
	for (;;) {
		ss = getc(inputt);
		if (ss == '\\') {
			ss = getc(inputt);
			if ((ss == delim) || ((flag == 1) && (ss == '\n')))
				l_pat[l_cnt] = ss;
			else {
				l_pat[l_cnt] = '\\';
				l_pat[++l_cnt] = ss;
			}
			goto leap;
		} else
			if ((ss == '\n') || (ss == EOF)) {
				ungetc(ss, inputt);
				strcpy(help_msg, "no closing delimiter found");
				*errnum = -1;
				/* This is done for s's backward compat. */
				l_pat[l_cnt] = '\0';
				return (l_pat);
			}
		if (ss == delim)
			break;

		l_pat[l_cnt] = ss;

leap:		if (l_cnt > l_max) {
			/* The RE is really long; get more space for it. */
			l_max = l_max + 256;
			l_pat_tmp = l_pat;
			l_pat = calloc(l_max + 2, sizeof(char));
			if (l_pat == NULL) {
				*errnum = -3;
				strcpy(help_msg, "out of memory error");
				return (NULL);
			}
			memmove(l_pat, l_pat_tmp, l_cnt);
			free(l_pat_tmp);
		}
		l_cnt++;
	}
	l_pat[l_cnt] = '\0';
	*errnum = 0;
	/*
	 * Send back the pattern.  l_pat[0] has the delimiter in it so the RE
	 * really starts at l_pat[1]. It's done this way for the special forms
	 * of 's' (substitute).
	 */
	return (l_pat);
}
