/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)authenc.c	5.1 (Berkeley) 3/1/91";
#endif /* not lint */

#if	defined(ENCRYPT) || defined(AUTHENTICATE)
#include "telnetd.h"
#include <libtelnet/misc.h>

	int
net_write(str, len)
	unsigned char *str;
	int len;
{
	if (nfrontp + len < netobuf + BUFSIZ) {
		bcopy((void *)str, (void *)nfrontp, len);
		nfrontp += len;
		return(len);
	}
	return(0);
}

	void
net_encrypt()
{
#if	defined(ENCRYPT)
	char *s = (nclearto > nbackp) ? nclearto : nbackp;
	if (s < nfrontp && encrypt_output) {
		(*encrypt_output)((unsigned char *)s, nfrontp - s);
	}
	nclearto = nfrontp;
#endif
}

	int
telnet_spin()
{
	ttloop();
	return(0);
}

	char *
telnet_getenv(val)
	char *val;
{
	extern char *getenv();
	return(getenv(val));
}

	char *
telnet_gets(prompt, result, length, echo)
	char *prompt;
	char *result;
	int length;
	int echo;
{
	return((char *)0);
}
#endif
