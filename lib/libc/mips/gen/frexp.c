/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)frexp.c	5.1 (Berkeley) 6/27/92";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <machine/endian.h>
#include <math.h>

double
frexp(value, eptr)
	double value;
	int *eptr;
{
	union {
                double v;
                struct {
#if BYTE_ORDER == LITTLE_ENDIAN
			u_int u_mant2 : 32;
			u_int u_mant1 : 20;
			u_int   u_exp : 11;
                        u_int  u_sign :  1;
#else
                        u_int  u_sign :  1;
			u_int   u_exp : 11;
			u_int u_mant1 : 20;
			u_int u_mant2 : 32;
#endif
                } s;
        } u;

	if (value) {
		u.v = value;
		*eptr = u.s.u_exp - 1022;
		u.s.u_exp = 1022;
		return(u.v);
	} else {
		*eptr = 0;
		return((double)0);
	}
}
