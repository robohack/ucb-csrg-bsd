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
static char sccsid[] = "@(#)filename.c	8.1 (Berkeley) 5/31/93";
#endif /* not lint */

#include <sys/types.h>

#include <limits.h>
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
 * A central function for any command that has to deal with a filename
 * (to be or not to be remembered).
 */
char *
filename(inputt, errnum)
	FILE *inputt;
	int *errnum;
{
	register int l_cnt = 0;
	char *l_fname;
	int l_esc = 0, l_bang_flag = 0;

	l_fname = calloc(FILENAME_LEN+2, sizeof(char));
	if (l_fname == NULL) {
		*errnum = -1;
		strcpy(help_msg, "out of memory error");
		return (NULL);
	}
	if ((ss = getc(inputt)) != ' ') {
		if (ss == '\n') {
			ungetc(ss, inputt);
			/*
			 * It's not really an error, but to flag remembered
			 * filename is to be used.
			 */
			*errnum = -2;
		} else {
			*errnum = -1;
			strcpy(help_msg,
			    "space required before filename given");
		}
		return (NULL);
	}
	while (ss = getc(inputt))
		if (ss != ' ') {
			ungetc(ss, inputt);
			break;
		}
	for (;;) {
		ss = getc(inputt);
		if ((ss == '\\') && (l_esc == 0)) {
			ss = getchar();
			l_esc = 1;
		} else
			l_esc = 0;
		if ((ss == '\n') || (ss == EOF)) {
			l_fname[l_cnt] = '\0';
			break;
		} else
			if ((ss == '!') && (l_esc == 0) && (l_cnt < 2))
				l_bang_flag = 1;
			else
				if ((ss != ' ') || (l_bang_flag))
					l_fname[l_cnt++] = ss;
				else {
					*errnum = -1;
					return (NULL);
				}

		if (l_cnt >= FILENAME_LEN) {
			strcpy(help_msg, "filename+path length too long");
			*errnum = -1;
			ungetc('\n', inputt);
			return (NULL);
		}
	}

	if (l_fname[0] == '\0') {
		sigspecial++;
		strcpy(l_fname, filename_current);
		sigspecial--;
		if (sigint_flag && (!sigspecial))
			SIGINT_ACTION;
	}
	*errnum = 1;
	l_fname[FILENAME_LEN+1] = l_bang_flag;
	return (l_fname);
}
