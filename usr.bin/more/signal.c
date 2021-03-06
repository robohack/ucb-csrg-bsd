/*
 * Copyright (c) 1988 Mark Nudleman
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)signal.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */

/*
 * Routines dealing with signals.
 *
 * A signal usually merely causes a bit to be set in the "signals" word.
 * At some convenient time, the mainline code checks to see if any
 * signals need processing by calling psignal().
 * If we happen to be reading from a file [in iread()] at the time
 * the signal is received, we call intread to interrupt the iread.
 */

#include <less.h>
#include <signal.h>

/*
 * "sigs" contains bits indicating signals which need to be processed.
 */
int sigs;

#ifdef SIGTSTP
#define	S_STOP		02
#endif
#if defined(SIGWINCH) || defined(SIGWIND)
#define S_WINCH		04
#endif

extern int sc_width, sc_height;
extern int screen_trashed;
extern int lnloop;
extern int linenums;
extern int scroll;
extern int reading;

#ifdef SIGTSTP
/*
 * "Stop" (^Z) signal handler.
 */
static void
stop()
{
	(void)signal(SIGTSTP, stop);
	sigs |= S_STOP;
	if (reading)
		intread();
}
#endif

#ifdef SIGWINCH
/*
 * "Window" change handler
 */
void
winch()
{
	(void)signal(SIGWINCH, winch);
	sigs |= S_WINCH;
	if (reading)
		intread();
}
#else
#ifdef SIGWIND
/*
 * "Window" change handler
 */
winch()
{
	(void)signal(SIGWIND, winch);
	sigs |= S_WINCH;
	if (reading)
		intread();
}
#endif
#endif

static void
purgeandquit()
{

	purge();	/* purge buffered output */
	quit();
}

/*
 * Set up the signal handlers.
 */
init_signals(on)
	int on;
{
	if (on)
	{
		/*
		 * Set signal handlers.
		 */
		(void)signal(SIGINT, purgeandquit);
#ifdef SIGTSTP
		(void)signal(SIGTSTP, stop);
#endif
#ifdef SIGWINCH
		(void)signal(SIGWINCH, winch);
#else
#ifdef SIGWIND
		(void)signal(SIGWIND, winch);
#endif
#endif
	} else
	{
		/*
		 * Restore signals to defaults.
		 */
		(void)signal(SIGINT, SIG_DFL);
#ifdef SIGTSTP
		(void)signal(SIGTSTP, SIG_DFL);
#endif
#ifdef SIGWINCH
		(void)signal(SIGWINCH, SIG_IGN);
#endif
#ifdef SIGWIND
		(void)signal(SIGWIND, SIG_IGN);
#endif
	}
}

/*
 * Process any signals we have received.
 * A received signal cause a bit to be set in "sigs".
 */
psignals()
{
	register int tsignals;

	if ((tsignals = sigs) == 0)
		return;
	sigs = 0;

#ifdef S_WINCH
	if (tsignals & S_WINCH)
	{
		int old_width, old_height;
		/*
		 * Re-execute get_term() to read the new window size.
		 */
		old_width = sc_width;
		old_height = sc_height;
		get_term();
		if (sc_width != old_width || sc_height != old_height)
		{
			scroll = (sc_height + 1) / 2;
			screen_trashed = 1;
		}
	}
#endif
#ifdef SIGTSTP
	if (tsignals & S_STOP)
	{
		/*
		 * Clean up the terminal.
		 */
#ifdef SIGTTOU
		(void)signal(SIGTTOU, SIG_IGN);
#endif
		lower_left();
		clear_eol();
		deinit();
		(void)flush();
		raw_mode(0);
#ifdef SIGTTOU
		(void)signal(SIGTTOU, SIG_DFL);
#endif
		(void)signal(SIGTSTP, SIG_DFL);
		(void)kill(getpid(), SIGTSTP);
		/*
		 * ... Bye bye. ...
		 * Hopefully we'll be back later and resume here...
		 * Reset the terminal and arrange to repaint the
		 * screen when we get back to the main command loop.
		 */
		(void)signal(SIGTSTP, stop);
		raw_mode(1);
		init();
		screen_trashed = 1;
	}
#endif
}
