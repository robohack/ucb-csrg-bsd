/*
 * Copyright (c) 1982, 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)samachdep.h	7.1 (Berkeley) 11/15/92
 */

#ifdef DS5000
#define	NASC		1

/*
 * Define different classes of device drivers for booting.
 */
#define TC_SCSI		0

struct intr_tab {
	void	(*func)();	/* pointer to interrupt routine */
	int	unit;		/* logical unit number */
};

extern	struct intr_tab intr_tab[];
#endif

#ifdef DS3100
#define	NSII		1
#endif

#define NRZ		7
#define NTZ		7
