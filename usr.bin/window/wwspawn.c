/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Edward Wang at The University of California, Berkeley.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)wwspawn.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */

#include "ww.h"
#include <sys/signal.h>

/*
 * There is a dead lock with vfork and closing of pseudo-ports.
 * So we have to be sneaky about error reporting.
 */
wwspawn(wp, file, argv)
register struct ww *wp;
char *file;
char **argv;
{
	int pid;
	int ret;
	char erred = 0;
	int s;

	s = sigblock(sigmask(SIGCHLD));
	switch (pid = vfork()) {
	case -1:
		wwerrno = WWE_SYS;
		ret = -1;
		break;
	case 0:
		if (wwenviron(wp) >= 0)
			execvp(file, argv);
		erred = 1;
		_exit(1);
	default:
		if (erred) {
			wwerrno = WWE_SYS;
			ret = -1;
		} else {
			wp->ww_pid = pid;
			wp->ww_state = WWS_HASPROC;
			ret = pid;
		}
	}
	(void) sigsetmask(s);
	if (wp->ww_socket >= 0) {
		(void) close(wp->ww_socket);
		wp->ww_socket = -1;
	}
	return ret;
}
