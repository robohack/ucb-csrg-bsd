/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)af.h	8.1 (Berkeley) 6/5/93
 */

/*
 * Routing table management daemon.
 */

/*
 * Per address family routines.
 */
struct afswitch {
	int	(*af_hash)();		/* returns keys based on address */
	int	(*af_netmatch)();	/* verifies net # matching */
	int	(*af_output)();		/* interprets address for sending */
	int	(*af_portmatch)();	/* packet from some other router? */
	int	(*af_portcheck)();	/* packet from privileged peer? */
	int	(*af_checkhost)();	/* tells if address is valid */
	int	(*af_rtflags)();	/* get flags for route (host or net) */
	int	(*af_sendroute)();	/* check bounds of subnet broadcast */
	int	(*af_canon)();		/* canonicalize address for compares */
	char	*(*af_format)();	/* convert address to string */
};

/*
 * Structure returned by af_hash routines.
 */
struct afhash {
	u_int	afh_hosthash;		/* host based hash */
	u_int	afh_nethash;		/* network based hash */
};

extern struct	afswitch afswitch[];	/* table proper */
extern int	af_max;			/* number of entries in table */
