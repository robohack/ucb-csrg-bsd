/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)domain.h	7.6 (Berkeley) 11/16/92
 */

/*
 * Structure per communications domain.
 */

/*
 * Forward structure declarations for function prototypes [sic].
 */
struct	mbuf;

struct	domain {
	int	dom_family;		/* AF_xxx */
	char	*dom_name;
	int	(*dom_init)		/* initialize domain data structures */
		__P((void));
	int	(*dom_externalize)	/* externalize access rights */
		__P((struct mbuf *));
	int	(*dom_dispose)		/* dispose of internalized rights */
		__P((struct mbuf *));
	struct	protosw *dom_protosw, *dom_protoswNPROTOSW;
	struct	domain *dom_next;
	int	(*dom_rtattach)		/* initialize routing table */
		__P((void **, int));
	int	dom_rtoffset;		/* an arg to rtattach, in bits */
	int	dom_maxrtkey;		/* for routing layer */
};

#ifdef KERNEL
struct	domain *domains;
#endif
