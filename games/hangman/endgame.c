/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)endgame.c	8.1 (Berkeley) 5/31/93";
#endif /* not lint */

# include	"hangman.h"

/*
 * endgame:
 *	Do what's necessary at the end of the game
 */
endgame()
{
	register char	ch;

	prman();
	if (Errors >= MAXERRS)
		Errors = MAXERRS + 2;
	prword();
	prdata();
	move(MESGY, MESGX);
	if (Errors > MAXERRS)
		printw("Sorry, the word was \"%s\"\n", Word);
	else
		printw("You got it!\n");

	for (;;) {
		mvaddstr(MESGY + 1, MESGX, "Another word? ");
		leaveok(stdscr, FALSE);
		refresh();
		if ((ch = readch()) == 'n')
			die();
		else if (ch == 'y')
			break;
		mvaddstr(MESGY + 2, MESGX, "Please type 'y' or 'n'");
	}

	leaveok(stdscr, TRUE);
	move(MESGY, MESGX);
	deleteln();
	deleteln();
	deleteln();
}










