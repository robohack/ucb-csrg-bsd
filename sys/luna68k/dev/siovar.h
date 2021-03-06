/*
 * Copyright (c) 1992 OMRON Corporation.
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * OMRON Corporation.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)siovar.h	8.1 (Berkeley) 6/10/93
 */

/*
 * siovar.h --
 */


struct	sio_portc {
	int	pc_major;
	int	pc_unit;
	int	pc_port;
	struct siodevice *pc_addr;
	int	(*pc_intr)();
};

struct	sio_softc {
	struct sio_portc *sc_pc;
};
