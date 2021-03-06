/*-
 * Copyright (c) 1982, 1986, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)dkbad.h	8.2 (Berkeley) 7/10/94
 */

/*
 * Definitions needed to perform bad sector revectoring ala DEC STD 144.
 *
 * The bad sector information is located in the first 5 even numbered
 * sectors of the last track of the disk pack.  There are five identical
 * copies of the information, described by the dkbad structure.
 *
 * Replacement sectors are allocated starting with the first sector before
 * the bad sector information and working backwards towards the beginning of
 * the disk.  A maximum of 126 bad sectors are supported.  The position of
 * the bad sector in the bad sector table determines which replacement sector
 * it corresponds to.
 *
 * The bad sector information and replacement sectors are conventionally
 * only accessible through the 'c' file system partition of the disk.  If
 * that partition is used for a file system, the user is responsible for
 * making sure that it does not overlap the bad sector information or any
 * replacement sectors.
 */
struct dkbad {
	int32_t   bt_csn;		/* cartridge serial number */
	u_int16_t bt_mbz;		/* unused; should be 0 */
	u_int16_t bt_flag;		/* -1 => alignment cartridge */
	struct bt_bad {
		u_int16_t bt_cyl;	/* cylinder number of bad sector */
		u_int16_t bt_trksec;	/* track and sector number */
	} bt_bad[126];
};

#define	ECC	0
#define	SSE	1
#define	BSE	2
#define	CONT	3
