/*-
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)astosc.h	8.1 (Berkeley) 6/6/93
 */

/*
 * This defines the structure used to translate:
 *
 *	ascii name ==> (scancode, shiftstate)
 *
 * (Actually, map3270 does "ascii name ==> index", and
 * termin does "index ==> (scancode, shiftstate)".  Both
 * mappings use this structure.)
 */

#define	INCLUDED_ASTOSC

struct astosc {
    unsigned char
	scancode,		/* Scan code for this function */
	shiftstate;		/* Shift state for this function */
    enum ctlrfcn function;	/* Internal function identifier */
    char *name;			/* Name of this function */
};

int ascii_to_index();		/* Function to feed InitControl() */

extern struct astosc astosc[256];
