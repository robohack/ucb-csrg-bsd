/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1988 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)main.c	1.14 (Berkeley) 6/28/90";
#endif /* not lint */

#include <sys/types.h>

#include "ring.h"

#include "externs.h"
#include "defines.h"

/*
 * Initialize variables.
 */

void
tninit()
{
    init_terminal();

    init_network();
    
    init_telnet();

    init_sys();

    init_3270();
}


/*
 * main.  Parse arguments, invoke the protocol or command parser.
 */


int
main(argc, argv)
	int argc;
	char *argv[];
{
    char *user = 0;

    tninit();		/* Clear out things */
#ifdef	CRAY
    _setlist_init();	/* Work around compiler bug */
#endif

    TerminalSaveState();

    prompt = argv[0];
    while ((argc > 1) && (argv[1][0] == '-')) {
	if (!strcmp(argv[1], "-d")) {
	    debug = 1;
	} else if (!strcmp(argv[1], "-n")) {
	    if ((argc > 1) && (argv[2][0] != '-')) {	/* get file name */
		SetNetTrace(argv[2]);
		argv++;
		argc--;
	    }
	} else if (!strcmp(argv[1], "-l")) {
	    if ((argc > 1) && (argv[2][0] != '-')) {	/* get user name */
		user = argv[2];
		argv++;
		argc--;
	    }
	} else if (!strncmp(argv[1], "-e", 2)) {
		set_escape_char(&argv[1][2]);
	} else {
#if	defined(TN3270) && defined(unix)
	    if (!strcmp(argv[1], "-t")) {
		if ((argc > 1) && (argv[2][0] != '-')) { /* get file name */
		    transcom = tline;
		    (void) strcpy(transcom, argv[2]);
		    argv++;
		    argc--;
		}
	    } else if (!strcmp(argv[1], "-noasynch")) {
		noasynchtty = 1;
		noasynchnet = 1;
	    } else if (!strcmp(argv[1], "-noasynchtty")) {
		noasynchtty = 1;
	    } else if (!strcmp(argv[1], "-noasynchnet")) {
		noasynchnet = 1;
	    } else
#endif	/* defined(TN3270) && defined(unix) */
	    if (argv[1][1] != '\0') {
		fprintf(stderr, "Unknown option *%s*.\n", argv[1]);
	    }
	}
	argc--;
	argv++;
    }
    if (argc != 1) {
	if (setjmp(toplevel) != 0)
	    Exit(0);
	if (user) {
	    argc += 2;
	    argv -= 2;
	    argv[0] = argv[2];
	    argv[1] = "-l";
	    argv[2] = user;
	}
	if (tn(argc, argv) == 1) {
	    return 0;
	} else {
	    return 1;
	}
    }
    (void) setjmp(toplevel);
    for (;;) {
#if	!defined(TN3270)
	command(1, 0, 0);
#else	/* !defined(TN3270) */
	if (!shell_active) {
	    command(1, 0, 0);
	} else {
#if	defined(TN3270)
	    shell_continue();
#endif	/* defined(TN3270) */
	}
#endif	/* !defined(TN3270) */
    }
}
