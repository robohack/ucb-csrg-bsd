/*-
 * Copyright (c) 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * This software was developed by the Computer Systems Engineering group
 * at Lawrence Berkeley Laboratory under DARPA contract BG 91-66 and
 * contributed to Berkeley.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)moddi3.c	5.4 (Berkeley) 6/19/92";
#endif /* LIBC_SCCS and not lint */

#include "quad.h"

/*
 * Return remainder after dividing two signed quads.
 *
 * XXX
 * If -1/2 should produce -1 on this machine, this code is wrong.
 */
quad
__moddi3(quad a, quad b)
{
	u_quad ua, ub, ur;
	int neg;

	if (a < 0)
		ua = -(u_quad)a, neg = 1;
	else
		ua = a, neg = 0;
	if (b < 0)
		ub = -(u_quad)b, neg ^= 1;
	else
		ub = b;
	(void)__qdivrem(ua, ub, &ur);
	return (neg ? -ur : ur);
}
