/*-
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)pathnames.h	8.1 (Berkeley) 6/9/93
 */

#include <paths.h>

#define	_PATH_MAKEKEY		"/usr/libexec/makekey"
#define	_PATH_BINMAIL		"/usr/libexec/mail.local"
#define	_PATH_EXRECOVER		"/usr/libexec/ex3.7recover"
#define	_PATH_EXPRESERVE	"/usr/libexec/ex3.7preserve"
#define	_PATH_PRESERVE		"/var/preserve"
#ifndef VMUNIX
#ifndef	vms
#define	EXSTRINGS	"/usr/libexec/ex3.7strings"
#endif
#endif
