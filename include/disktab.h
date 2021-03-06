/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)disktab.h	8.1 (Berkeley) 6/2/93
 */

#ifndef	_DISKTAB_H_
#define	_DISKTAB_H_

/*
 * Disk description table, see disktab(5)
 */
#define	DISKTAB		"/etc/disktab"

struct	disktab {
	char	*d_name;		/* drive name */
	char	*d_type;		/* drive type */
	int	d_secsize;		/* sector size in bytes */
	int	d_ntracks;		/* # tracks/cylinder */
	int	d_nsectors;		/* # sectors/track */
	int	d_ncylinders;		/* # cylinders */
	int	d_rpm;			/* revolutions/minute */
	int	d_badsectforw;		/* supports DEC bad144 std */
	int	d_sectoffset;		/* use sect rather than cyl offsets */
	struct	partition {
		int	p_size;		/* #sectors in partition */
		short	p_bsize;	/* block size in bytes */
		short	p_fsize;	/* frag size in bytes */
	} d_partitions[8];
};

#endif /* !_DISKTAB_H_ */
