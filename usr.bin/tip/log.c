/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)log.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */

#include "tip.h"

#ifdef ACULOG
static	FILE *flog = NULL;

/*
 * Log file maintenance routines
 */

logent(group, num, acu, message)
	char *group, *num, *acu, *message;
{
	char *user, *timestamp;
	struct passwd *pwd;
	long t;

	if (flog == NULL)
		return;
	if (flock(fileno(flog), LOCK_EX) < 0) {
		perror("tip: flock");
		return;
	}
	if ((user = getlogin()) == NOSTR)
		if ((pwd = getpwuid(getuid())) == NOPWD)
			user = "???";
		else
			user = pwd->pw_name;
	t = time(0);
	timestamp = ctime(&t);
	timestamp[24] = '\0';
	fprintf(flog, "%s (%s) <%s, %s, %s> %s\n",
		user, timestamp, group,
#ifdef PRISTINE
		"",
#else
		num,
#endif
		acu, message);
	(void) fflush(flog);
	(void) flock(fileno(flog), LOCK_UN);
}

loginit()
{
	flog = fopen(value(LOG), "a");
	if (flog == NULL)
		fprintf(stderr, "can't open log file %s.\r\n", value(LOG));
}
#endif
