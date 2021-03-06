/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)v7.local.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */

/*
 * Mail -- a mail program
 *
 * Version 7
 *
 * Local routines that are installation dependent.
 */

#include "rcv.h"
#include <fcntl.h>
#include "extern.h"

/*
 * Locate the user's mailbox file (ie, the place where new, unread
 * mail is queued).
 */
void
findmail(user, buf)
	char *user, *buf;
{
	(void)sprintf(buf, "%s/%s", _PATH_MAILDIR, user);
}

/*
 * Get rid of the queued mail.
 */
void
demail()
{

	if (value("keep") != NOSTR || rm(mailname) < 0)
		close(creat(mailname, 0600));
}

/*
 * Discover user login name.
 */
char *
username()
{
	char *np;

	if ((np = getenv("USER")) != NOSTR)
		return np;
	return getname(getuid());
}
