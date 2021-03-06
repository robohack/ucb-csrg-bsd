/*
 * Copyright (c) 1986, 1989, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)signal.h	8.2 (Berkeley) 5/3/95
 */

/*
 * Machine-dependent signal definitions
 */

typedef int sig_atomic_t;

#if !defined(_POSIX_SOURCE) && !defined(_ANSI_SOURCE)
#include <machine/trap.h>	/* codes for SIGILL, SIGFPE */

/*
 * Information pushed on stack when a signal is delivered.
 * This is used by the kernel to restore state following
 * execution of the signal handler.  It is also made available
 * to the handler to allow it to restore state properly if
 * a non-standard exit is performed.
 */
struct	sigcontext {
	int	sc_onstack;	/* sigstack state to restore */
	int	sc_mask;	/* signal mask to restore */
	int	sc_sp;		/* sp to restore */
	int	sc_fp;		/* fp to restore */
	int	sc_ap;		/* ap to restore */
	int	sc_pc;		/* pc to restore */
	int	sc_ps;		/* psl to restore */
};
#endif
