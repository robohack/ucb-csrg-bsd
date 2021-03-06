/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)ucred.h	8.4 (Berkeley) 1/9/95
 */

#ifndef _SYS_UCRED_H_
#define	_SYS_UCRED_H_

/*
 * Credentials.
 */
struct ucred {
	u_short	cr_ref;			/* reference count */
	uid_t	cr_uid;			/* effective user id */
	short	cr_ngroups;		/* number of groups */
	gid_t	cr_groups[NGROUPS];	/* groups */
};
#define cr_gid cr_groups[0]
#define NOCRED ((struct ucred *)0)	/* no credential available */
#define FSCRED ((struct ucred *)-1)	/* filesystem credential */

#ifdef KERNEL
#define	crhold(cr)	(cr)->cr_ref++

struct ucred	*crcopy __P((struct ucred *cr));
struct ucred	*crdup __P((struct ucred *cr));
void		crfree __P((struct ucred *cr));
struct ucred	*crget __P((void));
int		suser __P((struct ucred *cred, u_short *acflag));
#endif /* KERNEL */

#endif /* !_SYS_UCRED_H_ */
