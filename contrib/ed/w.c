/*-
 * Copyright (c) 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Rodney Ruddock of the University of Guelph.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)w.c	5.7 (Berkeley) 4/30/93";
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
 * Write the contents of the buffer out to the real file (spec'd or
 * remembered). If 'w' then overwrite, if 'W' append to the file. 'W'
 * is probably _the_ command that most editors don't have, and it's
 * so(!) useful. The 'wq' works as 'w' but 'q' immediately follows.
 * Shame on POSIX for not including 'W' and 'wq', they're not that
 * hard to implement; yaaa! BSD for keeping it! :-)
 */
void
w(inputt, errnum)
	FILE *inputt;
	int *errnum;
{
	FILE *l_fp;
	int l_ttl = 0, l_q_flag = 0, l_sl, l_bang_flag=0;
	char *filename_read=NULL, *l_temp;

	if (Start_default && End_default) {
		Start = top;
		End = bottom;
	} else
		if (Start_default)
			Start = End;
	if (Start == NULL) {
		strcpy(help_msg, "buffer empty");
		*errnum = -1;
		return;
	}
	Start_default = End_default = 0;

	l_sl = ss;
	ss = getc(inputt);

	if (ss == 'q')		/* "wq" and "Wq" command */
		l_q_flag = 1;
	else
		ungetc(ss, inputt);

	l_temp = filename(inputt, errnum);
	if (*errnum == 1)
		filename_read = l_temp;
	else
		if (*errnum == -2) {
			while (((ss = getc(inputt)) != '\n') || (ss == EOF));
			filename_read = filename_current;
		} else
			if (*errnum < 0)
				return;
	*errnum = 0;

	if (filename_current == NULL) {
		if (filename_read == NULL) {
			strcpy(help_msg, "no filename given");
			*errnum = -1;
			ungetc('\n', inputt);
			return;
		} else
			filename_current = filename_read;
	}
	sigspecial++;
	if (l_temp && l_temp[FILENAME_LEN+1]) { /* bang flag */
		FILE *popen();

		if (l_temp[0] == '\0') {
			strcpy(help_msg, "no command given");
			*errnum = -1;
			sigspecial--;
			return;
		}
		if ((l_fp = popen(l_temp, "w")) == NULL) {
			strcpy(help_msg, "error executing command");
			*errnum = -1;
			if (l_fp != NULL)
				pclose(l_fp);
			sigspecial--;
			return;
		}
		l_bang_flag = 1;
	}
	else if (l_sl == 'W')
		l_fp = fopen(filename_read, "a");
	else
		l_fp = fopen(filename_read, "w");

	if (l_fp == NULL) {
		strcpy(help_msg, "cannot write to file");
		*errnum = -1;
		ungetc('\n', inputt);
		sigspecial--;
		return;
	}
	sigspecial--;
	if (sigint_flag && (!sigspecial))
		goto point;

	/* Write it out and get a report on the number of bytes written. */
	l_ttl = edwrite(l_fp, Start, End);
	if (explain_flag > 0)		/* For -s option. */
		printf("%d\n", l_ttl);

point:	if (l_bang_flag)
		pclose(l_fp);
	else
		fclose(l_fp);
	if (filename_read != filename_current)
		free(filename_read);
	if ((Start == top) && (End == bottom))
		change_flag = 0L;
	*errnum = 1;
	if (l_q_flag) {			/* For "wq" and "Wq". */
		ungetc('\n', inputt);
		ss = (int) 'q';
		q(inputt, errnum);
	}
}

/*
 * Actually writes out the contents of the buffer to the specified
 * STDIO file pointer for the range of lines specified.
 */
int
edwrite(fp, begi, fini)
	FILE *fp;
	LINE *begi, *fini;
{
	register int l_ttl = 0;

	for (;;) {
		get_line(begi->handle, begi->len);
		if (sigint_flag && (!sigspecial))
			break;

		sigspecial++;
		/* Fwrite is about 20+% faster than fprintf -- no surprise. */
		fwrite(text, sizeof(char), begi->len, fp);
		fputc('\n', fp);
		l_ttl = l_ttl + (begi->len) + 1;
		if (begi == fini)
			break;
		else
			begi = begi->below;
		sigspecial--;
		if (sigint_flag && (!sigspecial))
			break;
	}
	return (l_ttl);
}
