/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)ex_extern.c	1.1.1.1 (Berkeley) 5/31/85";
#endif not lint

/*
 * Provide defs of the global variables.
 * This crock is brought to you by the turkeys
 * who broke Unix on the Bell Labs 3B machine,
 * all in the name of "but that's what the C
 * book says!"
 */

# define extern 	/* nothing */
# include "ex.h"
# include "ex_argv.h"
# include "ex_re.h"
# include "ex_temp.h"
# include "ex_tty.h"
# include "ex_tune.h"
# include "ex_vars.h"
# include "ex_vis.h"

short tfile = -1;
