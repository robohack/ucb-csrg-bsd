/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)if_vba.h	1.1 (Berkeley) 3/9/89
 */

struct ifvba {
	struct	mbuf *iff_mbuf;	/* associated mbuf to free */
	caddr_t	iff_buffer;	/* contiguous memory for data, kernel address */
	u_long	iff_physaddr;	/* contiguous memory for data, phys address */
};

#define VIFF_16BIT 1		/* only allow two byte transfers */

#ifdef KERNEL
struct mbuf *if_vbaget();
#endif
