/*
 * Copyright (c) 1990 Jan-Simon Pendry
 * Copyright (c) 1990 Imperial College of Science, Technology & Medicine
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Jan-Simon Pendry at Imperial College, London.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)mount_aix.c	8.1 (Berkeley) 6/6/93
 *
 * $Id: mount_aix.c,v 5.2.2.1 1992/02/09 15:10:08 jsp beta $
 *
 */


/*
 * AIX 3 Mount helper
 */

#include "misc-aix3.h"

static int aix3_mkvp(p, gfstype, flags, object, stub, host, info, info_size, args)
char *p;
int gfstype;
int flags;
char *object;
char *stub;
char *host;
char *info;
int info_size;
char *args;
{
	struct vmount *vp = (struct vmount *) p;
	bzero((voidp) vp, sizeof(*vp));
	/*
	 * Fill in standard fields
	 */
	vp->vmt_revision = VMT_REVISION;
	vp->vmt_flags = flags;
	vp->vmt_gfstype = gfstype;

#define	VMT_ROUNDUP(len) (4 * ((len + 3) / 4))
#define VMT_ASSIGN(vp, idx, data, size) \
	vp->vmt_data[idx].vmt_off = p - (char *) vp; \
	vp->vmt_data[idx].vmt_size = size; \
	bcopy(data, p, size); \
	p += VMT_ROUNDUP(size);

	/*
	 * Fill in all variable length data
	 */
	p += sizeof(*vp);

	VMT_ASSIGN(vp, VMT_OBJECT, object, strlen(object) + 1);
	VMT_ASSIGN(vp, VMT_STUB, stub, strlen(stub) + 1);
	VMT_ASSIGN(vp, VMT_HOST, host, strlen(host) + 1);
	VMT_ASSIGN(vp, VMT_HOSTNAME, host, strlen(host) + 1);
	VMT_ASSIGN(vp, VMT_INFO, info, info_size);
	VMT_ASSIGN(vp, VMT_ARGS, args, strlen(args) + 1);

#undef VMT_ASSIGN
#undef VMT_ROUNDUP

	/*
	 * Return length
	 */
	return vp->vmt_length = p - (char *) vp;
}

/*
 * Map from conventional mount arguments
 * to AIX 3-style arguments.
 */
aix3_mount(fsname, dir, flags, type, data, args)
char *fsname;
char *dir;
int flags;
int type;
void *data;
char *args;
{
	char buf[4096];
	int size;

#ifdef DEBUG
	dlog("aix3_mount: fsname %s, dir %s, type %d", fsname, dir, type);
#endif /* DEBUG */

/* aix3_mkvp(p, gfstype, flags, object, stub, host, info, info_size, args) */

	switch (type) {

	case MOUNT_TYPE_NFS: {
		char *host = strdup(fsname);
		char *rfs = strchr(host, ':');
		int free_rfs = 0;
		if (rfs) {
			*rfs++ = '\0';
		} else {
			rfs = host;
			free_rfs = 1;
			host = strdup(hostname);
		}

		size = aix3_mkvp(buf, type, flags, rfs, dir, host, data, sizeof(struct nfs_args), args);
		if (free_rfs)
			free((voidp) rfs);
		free(host);

		} break;

	case MOUNT_TYPE_UFS:
		/* Need to open block device and extract log device info from sblk. */
		return EINVAL;

	default:
		return EINVAL;
	}
#ifdef DEBUG
	/*dlog("aix3_mkvp: flags %#x, size %d, args %s", flags, size, args);*/
#endif /* DEBUG */

	return vmount(buf, size);
}
