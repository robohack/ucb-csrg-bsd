/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1992 OMRON Corporation.
 * Copyright (c) 1990, 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
 *
 * %sccs.include.redist.c%
 *
 * from: Utah $Hdr: cons.h 1.1 90/07/09$
 *
 * from: hp300/hp300/cons.h	7.1 (Berkeley) 6/4/92
 *
 *	@(#)cons.h	8.1 (Berkeley) 6/10/93
 */

struct consdev {
	int	(*cn_probe)();	/* probe hardware and fill in consdev info */
	int	(*cn_init)();	/* turn on as console */
	int	(*cn_getc)();	/* kernel getchar interface */
	int	(*cn_putc)();	/* kernel putchar interface */
	struct	tty *cn_tp;	/* tty structure for console device */
	dev_t	cn_dev;		/* major/minor of device */
	short	cn_pri;		/* pecking order; the higher the better */
};

/* values for cn_pri - reflect our policy for console selection */
#define	CN_DEAD		0	/* device doesn't exist */
#define CN_NORMAL	1	/* device exists but is nothing special */
#define CN_INTERNAL	2	/* "internal" bit-mapped display */
#define CN_REMOTE	3	/* serial interface with remote bit set */

/* XXX */
#define	CONSMAJOR	0

#ifdef KERNEL
extern	struct consdev constab[];
extern	struct consdev *cn_tab;
extern	struct tty *cn_tty;
#endif
