/*
 * $Id: wr_dumpset.c,v 5.2.1.2 90/12/21 16:46:50 jsp Alpha $
 *
 * Copyright (c) 1989 Jan-Simon Pendry
 * Copyright (c) 1989 Imperial College of Science, Technology & Medicine
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Jan-Simon Pendry at Imperial College, London.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)wr_dumpset.c	5.1 (Berkeley) 3/17/91
 */

#include "../fsinfo/fsinfo.h"

static int write_dumpset_info(ef, q)
FILE *ef;
qelem *q;
{
	int errors = 0;
	disk_fs *dp;

	ITER(dp, disk_fs, q) {
		if (dp->d_dumpset) {
			fprintf(ef, "%s\t%s:%-30s\t# %s\n",
				dp->d_dumpset,
				dp->d_host->h_lochost ?
				dp->d_host->h_lochost :
				dp->d_host->h_hostname,
				dp->d_mountpt,
				dp->d_dev);
		}
	}
	return errors;
}

int write_dumpset(q)
qelem *q;
{
	int errors = 0;

	if (dumpset_pref) {
		FILE *ef = pref_open(dumpset_pref, "dumpsets", info_hdr, "exabyte dumpset");
		if (ef) {
			host *hp;
			ITER(hp, host, q) {
				if (hp->h_disk_fs) {
					errors += write_dumpset_info(ef, hp->h_disk_fs);
				}
			}
			errors += pref_close(ef);
		} else {
			errors++;
		}
	}

	return errors;
}
