/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)devname.c	5.14 (Berkeley) 5/6/91";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <fcntl.h>
#include <db.h>
#include <stdio.h>
#include <paths.h>

char *
devname(dev, type)
	dev_t dev;
	mode_t type;
{
	struct {
		mode_t type;
		dev_t dev;
	} bkey;
	static DB *db;
	static int failure;
	DBT data, key;

	if (!db && !failure &&
	    !(db = hash_open(_PATH_DEVDB, O_RDONLY, 0, NULL))) {
		(void)fprintf(stderr,
		    "warning: no device database %s\n", _PATH_DEVDB);
		failure = 1;
	}
	if (failure)
		return("??");

	/*
	 * Keys are a mode_t followed by a dev_t.  The former is the type of
	 * the file (mode & S_IFMT), the latter is the st_rdev field.
	 */
	bkey.dev = dev;
	bkey.type = type;
	key.data = &bkey;
	key.size = sizeof(bkey);
	return((db->get)(db, &key, &data, 0L) ? "??" : (char *)data.data);
}
