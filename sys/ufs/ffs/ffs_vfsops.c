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
 *	@(#)ffs_vfsops.c	7.28 (Berkeley) 11/14/89
 */

#include "param.h"
#include "systm.h"
#include "time.h"
#include "kernel.h"
#include "namei.h"
#include "vnode.h"
#include "mount.h"
#include "buf.h"
#include "ucred.h"
#include "file.h"
#include "disklabel.h"
#include "ioctl.h"
#include "errno.h"
#include "malloc.h"
#include "../ufs/fs.h"
#include "../ufs/ufsmount.h"
#include "../ufs/inode.h"

/*
 * ufs vfs operations.
 */
int ufs_mount();
int ufs_start();
int ufs_unmount();
int ufs_root();
int ufs_statfs();
int ufs_sync();
int ufs_fhtovp();
int ufs_vptofh();
int ufs_init();

struct vfsops ufs_vfsops = {
	ufs_mount,
	ufs_start,
	ufs_unmount,
	ufs_root,
	ufs_statfs,
	ufs_sync,
	ufs_fhtovp,
	ufs_vptofh,
	ufs_init
};

/*
 * ufs mount table.
 */
struct ufsmount mounttab[NMOUNT];

/*
 * Called by vfs_mountroot when ufs is going to be mounted as root.
 *
 * Name is updated by mount(8) after booting.
 */
#define ROOTNAME	"root_device"

ufs_mountroot()
{
	register struct mount *mp;
	extern struct vnode *rootvp;
	struct ufsmount *ump;
	register struct fs *fs;
	u_int size;
	int error;

	mp = (struct mount *)malloc((u_long)sizeof(struct mount),
		M_MOUNT, M_WAITOK);
	mp->m_op = &ufs_vfsops;
	mp->m_flag = 0;
	mp->m_exroot = 0;
	mp->m_mounth = (struct vnode *)0;
	error = mountfs(rootvp, mp);
	if (error) {
		free((caddr_t)mp, M_MOUNT);
		return (error);
	}
	if (error = vfs_lock(mp)) {
		(void)ufs_unmount(mp, 0);
		free((caddr_t)mp, M_MOUNT);
		return (error);
	}
	rootfs = mp;
	mp->m_next = mp;
	mp->m_prev = mp;
	mp->m_vnodecovered = (struct vnode *)0;
	ump = VFSTOUFS(mp);
	fs = ump->um_fs;
	fs->fs_fsmnt[0] = '/';
	bzero(fs->fs_fsmnt + 1, sizeof(fs->fs_fsmnt) - 1);
	(void) copystr(ROOTNAME, ump->um_mntname, MNAMELEN - 1, &size);
	bzero(ump->um_mntname + size, MNAMELEN - size);
	vfs_unlock(mp);
	inittodr(fs->fs_time);
	return (0);
}

/*
 * VFS Operations.
 *
 * mount system call
 */
ufs_mount(mp, path, data, ndp)
	struct mount *mp;
	char *path;
	caddr_t data;
	struct nameidata *ndp;
{
	struct vnode *devvp;
	struct ufs_args args;
	struct ufsmount *ump;
	register struct fs *fs;
	u_int size;
	int error;

	if (error = copyin(data, (caddr_t)&args, sizeof (struct ufs_args)))
		return (error);
	if ((error = getmdev(&devvp, args.fspec, ndp)) != 0)
		return (error);
	if ((mp->m_flag & M_UPDATE) == 0) {
		error = mountfs(devvp, mp);
	} else {
		ump = VFSTOUFS(mp);
		fs = ump->um_fs;
		if (fs->fs_ronly && (mp->m_flag & M_RDONLY) == 0)
			fs->fs_ronly = 0;
		/*
		 * Verify that the specified device is the one that
		 * is really being used for the root file system.
		 */
		if (devvp != ump->um_devvp)
			error = EINVAL;	/* needs translation */
	}
	if (error) {
		vrele(devvp);
		return (error);
	}
	ump = VFSTOUFS(mp);
	fs = ump->um_fs;
	(void) copyinstr(path, fs->fs_fsmnt, sizeof(fs->fs_fsmnt) - 1, &size);
	bzero(fs->fs_fsmnt + size, sizeof(fs->fs_fsmnt) - size);
	(void) copyinstr(args.fspec, ump->um_mntname, MNAMELEN - 1, &size);
	bzero(ump->um_mntname + size, MNAMELEN - size);
	return (0);
}

/*
 * Common code for mount and mountroot
 */
mountfs(devvp, mp)
	struct vnode *devvp;
	struct mount *mp;
{
	register struct ufsmount *ump;
	struct ufsmount *fmp = NULL;
	struct buf *bp = NULL;
	register struct fs *fs;
	dev_t dev = devvp->v_rdev;
	struct partinfo dpart;
	caddr_t base, space;
	int havepart = 0, blks;
	int error, i, size;
	int needclose = 0;
	int ronly = (mp->m_flag & M_RDONLY) != 0;

	for (ump = &mounttab[0]; ump < &mounttab[NMOUNT]; ump++) {
		if (ump->um_fs == NULL) {
			if (fmp == NULL)
				fmp = ump;
		} else if (dev == ump->um_dev) {
			return (EBUSY);		/* needs translation */
		}
	}
	if ((ump = fmp) == NULL)
		return (EMFILE);		/* needs translation */
	ump->um_fs = (struct fs *)1;		/* just to reserve this slot */
	error = VOP_OPEN(devvp, ronly ? FREAD : FREAD|FWRITE, NOCRED);
	if (error) {
		ump->um_fs = NULL;
		return (error);
	}
	needclose = 1;
	if (VOP_IOCTL(devvp, DIOCGPART, (caddr_t)&dpart, FREAD, NOCRED) != 0)
		size = DEV_BSIZE;
	else {
		havepart = 1;
		size = dpart.disklab->d_secsize;
	}
	if (error = bread(devvp, SBLOCK, SBSIZE, NOCRED, &bp)) {
		ump->um_fs = NULL;
		goto out;
	}
	fs = bp->b_un.b_fs;
	if (fs->fs_magic != FS_MAGIC || fs->fs_bsize > MAXBSIZE ||
	    fs->fs_bsize < sizeof(struct fs)) {
		ump->um_fs = NULL;
		error = EINVAL;		/* XXX also needs translation */
		goto out;
	}
	ump->um_fs = (struct fs *)malloc((u_long)fs->fs_sbsize, M_SUPERBLK,
	    M_WAITOK);
	bcopy((caddr_t)bp->b_un.b_addr, (caddr_t)ump->um_fs,
	   (u_int)fs->fs_sbsize);
	brelse(bp);
	bp = NULL;
	fs = ump->um_fs;
	fs->fs_ronly = ronly;
	if (ronly == 0)
		fs->fs_fmod = 1;
	if (havepart) {
		dpart.part->p_fstype = FS_BSDFFS;
		dpart.part->p_fsize = fs->fs_fsize;
		dpart.part->p_frag = fs->fs_frag;
		dpart.part->p_cpg = fs->fs_cpg;
	}
	blks = howmany(fs->fs_cssize, fs->fs_fsize);
	base = space = (caddr_t)malloc((u_long)fs->fs_cssize, M_SUPERBLK,
	    M_WAITOK);
	for (i = 0; i < blks; i += fs->fs_frag) {
		size = fs->fs_bsize;
		if (i + fs->fs_frag > blks)
			size = (blks - i) * fs->fs_fsize;
		error = bread(devvp, fsbtodb(fs, fs->fs_csaddr + i), size,
			NOCRED, &bp);
		if (error) {
			free((caddr_t)base, M_SUPERBLK);
			goto out;
		}
		bcopy((caddr_t)bp->b_un.b_addr, space, (u_int)size);
		fs->fs_csp[fragstoblks(fs, i)] = (struct csum *)space;
		space += size;
		brelse(bp);
		bp = NULL;
	}
	mp->m_data = (qaddr_t)ump;
	mp->m_bsize = fs->fs_bsize;
	mp->m_fsize = fs->fs_fsize;
	mp->m_fsid.val[0] = (long)dev;
	mp->m_fsid.val[1] = MOUNT_UFS;
	ump->um_mountp = mp;
	ump->um_dev = dev;
	ump->um_devvp = devvp;
	ump->um_qinod = NULL;
	devvp->v_mount = mp;

	/* Sanity checks for old file systems.			   XXX */
	fs->fs_npsect = MAX(fs->fs_npsect, fs->fs_nsect);	/* XXX */
	fs->fs_interleave = MAX(fs->fs_interleave, 1);		/* XXX */
	if (fs->fs_postblformat == FS_42POSTBLFMT)		/* XXX */
		fs->fs_nrpos = 8;				/* XXX */
	return (0);
out:
	if (needclose)
		(void) VOP_CLOSE(devvp, ronly ? FREAD : FREAD|FWRITE, NOCRED);
	if (ump->um_fs) {
		free((caddr_t)ump->um_fs, M_SUPERBLK);
		ump->um_fs = NULL;
	}
	if (bp)
		brelse(bp);
	return (error);
}

/*
 * Make a filesystem operational.
 * Nothing to do at the moment.
 */
/* ARGSUSED */
ufs_start(mp, flags)
	struct mount *mp;
	int flags;
{

	return (0);
}

/*
 * unmount system call
 */
ufs_unmount(mp, flags)
	struct mount *mp;
	int flags;
{
	register struct ufsmount *ump;
	register struct fs *fs;
	int error, ronly;

	if (flags & MNT_FORCE)
		return (EINVAL);
	ump = VFSTOUFS(mp);
	if (error = vflush(mp, ITOV(ump->um_qinod), flags))
		return (error);
#ifdef QUOTA
	(void) closedq(ump);
	/*
	 * Here we have to vflush again to get rid of the quota inode.
	 * A drag, but it would be ugly to cheat, & this does not happen often.
	 */
	if (vflush(mp, (struct vnode *)NULL, MNT_NOFORCE))
		panic("ufs_unmount: quota");
#endif
	fs = ump->um_fs;
	ronly = !fs->fs_ronly;
	free((caddr_t)fs->fs_csp[0], M_SUPERBLK);
	free((caddr_t)fs, M_SUPERBLK);
	ump->um_fs = NULL;
	ump->um_dev = NODEV;
	error = VOP_CLOSE(ump->um_devvp, ronly ? FREAD : FREAD|FWRITE, NOCRED);
	ump->um_devvp->v_mount = (struct mount *)0;
	vrele(ump->um_devvp);
	ump->um_devvp = (struct vnode *)0;
	return (error);
}

/*
 * Return root of a filesystem
 */
ufs_root(mp, vpp)
	struct mount *mp;
	struct vnode **vpp;
{
	register struct inode *ip;
	struct inode *nip;
	struct vnode tvp;
	int error;

	tvp.v_mount = mp;
	ip = VTOI(&tvp);
	ip->i_vnode = &tvp;
	ip->i_dev = VFSTOUFS(mp)->um_dev;
	error = iget(ip, (ino_t)ROOTINO, &nip);
	if (error)
		return (error);
	*vpp = ITOV(nip);
	return (0);
}

/*
 * Get file system statistics.
 */
ufs_statfs(mp, sbp)
	struct mount *mp;
	register struct statfs *sbp;
{
	register struct ufsmount *ump;
	register struct fs *fs;

	ump = VFSTOUFS(mp);
	fs = ump->um_fs;
	if (fs->fs_magic != FS_MAGIC)
		panic("ufs_statfs");
	sbp->f_type = MOUNT_UFS;
	sbp->f_fsize = fs->fs_fsize;
	sbp->f_bsize = fs->fs_bsize;
	sbp->f_blocks = fs->fs_dsize;
	sbp->f_bfree = fs->fs_cstotal.cs_nbfree * fs->fs_frag +
		fs->fs_cstotal.cs_nffree;
	sbp->f_bavail = (fs->fs_dsize * (100 - fs->fs_minfree) / 100) -
		(fs->fs_dsize - sbp->f_bfree);
	sbp->f_files =  fs->fs_ncg * fs->fs_ipg - ROOTINO;
	sbp->f_ffree = fs->fs_cstotal.cs_nifree;
	bcopy((caddr_t)fs->fs_fsmnt, (caddr_t)&sbp->f_mntonname[0], MNAMELEN);
	bcopy((caddr_t)ump->um_mntname, (caddr_t)&sbp->f_mntfromname[0],
		MNAMELEN);
	return (0);
}

int	syncprt = 0;

/*
 * Go through the disk queues to initiate sandbagged IO;
 * go through the inodes to write those that have been modified;
 * initiate the writing of the super block if it has been modified.
 */
ufs_sync(mp, waitfor)
	struct mount *mp;
	int waitfor;
{
	register struct vnode *vp;
	register struct inode *ip;
	register struct ufsmount *ump = VFSTOUFS(mp);
	register struct fs *fs;
	int error = 0;
	static int updlock = 0;

	if (syncprt)
		bufstats();
	if (updlock)
		return (EBUSY);
	fs = ump->um_fs;
	if (fs == (struct fs *)1)
		return (0);
	updlock++;
	/*
	 * Write back modified superblock.
	 * Consistency check that the superblock
	 * is still in the buffer cache.
	 */
	if (fs->fs_fmod != 0) {
		if (fs->fs_ronly != 0) {		/* XXX */
			printf("fs = %s\n", fs->fs_fsmnt);
			panic("update: rofs mod");
		}
		fs->fs_fmod = 0;
		fs->fs_time = time.tv_sec;
		error = sbupdate(ump, waitfor);
	}
	/*
	 * Write back each (modified) inode.
	 */
	for (vp = mp->m_mounth; vp; vp = vp->v_mountf) {
		ip = VTOI(vp);
		if ((ip->i_flag & ILOCKED) != 0 || ITOV(ip)->v_count == 0 ||
		    (ip->i_flag & (IMOD|IACC|IUPD|ICHG)) == 0)
			continue;
		ILOCK(ip);
		VREF(vp);
		error = iupdat(ip, &time, &time, waitfor == MNT_WAIT);
		iput(ip);
	}
	updlock = 0;
	/*
	 * Force stale buffer cache information to be flushed.
	 */
	bflush(ump->um_devvp->v_mount);
	return (error);
}

/*
 * Write a superblock and associated information back to disk.
 */
sbupdate(mp, waitfor)
	struct ufsmount *mp;
	int waitfor;
{
	register struct fs *fs = mp->um_fs;
	register struct buf *bp;
	int blks;
	caddr_t space;
	int i, size, error = 0;

	bp = getblk(mp->um_devvp, SBLOCK, (int)fs->fs_sbsize);
	bcopy((caddr_t)fs, bp->b_un.b_addr, (u_int)fs->fs_sbsize);
	/* Restore compatibility to old file systems.		   XXX */
	if (fs->fs_postblformat == FS_42POSTBLFMT)		/* XXX */
		bp->b_un.b_fs->fs_nrpos = -1;			/* XXX */
	if (waitfor == MNT_WAIT)
		error = bwrite(bp);
	else
		bawrite(bp);
	blks = howmany(fs->fs_cssize, fs->fs_fsize);
	space = (caddr_t)fs->fs_csp[0];
	for (i = 0; i < blks; i += fs->fs_frag) {
		size = fs->fs_bsize;
		if (i + fs->fs_frag > blks)
			size = (blks - i) * fs->fs_fsize;
		bp = getblk(mp->um_devvp, fsbtodb(fs, fs->fs_csaddr + i), size);
		bcopy(space, bp->b_un.b_addr, (u_int)size);
		space += size;
		if (waitfor == MNT_WAIT)
			error = bwrite(bp);
		else
			bawrite(bp);
	}
	return (error);
}

/*
 * Print out statistics on the current allocation of the buffer pool.
 * Can be enabled to print out on every ``sync'' by setting "syncprt"
 * above.
 */
bufstats()
{
	int s, i, j, count;
	register struct buf *bp, *dp;
	int counts[MAXBSIZE/CLBYTES+1];
	static char *bname[BQUEUES] = { "LOCKED", "LRU", "AGE", "EMPTY" };

	for (bp = bfreelist, i = 0; bp < &bfreelist[BQUEUES]; bp++, i++) {
		count = 0;
		for (j = 0; j <= MAXBSIZE/CLBYTES; j++)
			counts[j] = 0;
		s = splbio();
		for (dp = bp->av_forw; dp != bp; dp = dp->av_forw) {
			counts[dp->b_bufsize/CLBYTES]++;
			count++;
		}
		splx(s);
		printf("%s: total-%d", bname[i], count);
		for (j = 0; j <= MAXBSIZE/CLBYTES; j++)
			if (counts[j] != 0)
				printf(", %d-%d", j * CLBYTES, counts[j]);
		printf("\n");
	}
}

/*
 * File handle to vnode
 *
 * Have to be really careful about stale file handles:
 * - check that the inode number is in range
 * - call iget() to get the locked inode
 * - check for an unallocated inode (i_mode == 0)
 * - check that the generation number matches
 */
ufs_fhtovp(mp, fhp, vpp)
	register struct mount *mp;
	struct fid *fhp;
	struct vnode **vpp;
{
	register struct ufid *ufhp;
	register struct fs *fs;
	register struct inode *ip;
	struct inode *nip;
	struct vnode tvp;
	int error;

	ufhp = (struct ufid *)fhp;
	fs = VFSTOUFS(mp)->um_fs;
	if (ufhp->ufid_ino < ROOTINO ||
	    ufhp->ufid_ino >= fs->fs_ncg * fs->fs_ipg) {
		*vpp = (struct vnode *)0;
		return (EINVAL);
	}
	tvp.v_mount = mp;
	ip = VTOI(&tvp);
	ip->i_vnode = &tvp;
	ip->i_dev = VFSTOUFS(mp)->um_dev;
	if (error = iget(ip, ufhp->ufid_ino, &nip)) {
		*vpp = (struct vnode *)0;
		return (error);
	}
	ip = nip;
	if (ip->i_mode == 0) {
		iput(ip);
		*vpp = (struct vnode *)0;
		return (EINVAL);
	}
	if (ip->i_gen != ufhp->ufid_gen) {
		iput(ip);
		*vpp = (struct vnode *)0;
		return (EINVAL);
	}
	*vpp = ITOV(ip);
	return (0);
}

/*
 * Vnode pointer to File handle
 */
/* ARGSUSED */
ufs_vptofh(vp, fhp)
	struct vnode *vp;
	struct fid *fhp;
{
	register struct inode *ip = VTOI(vp);
	register struct ufid *ufhp;

	ufhp = (struct ufid *)fhp;
	ufhp->ufid_len = sizeof(struct ufid);
	ufhp->ufid_ino = ip->i_number;
	ufhp->ufid_gen = ip->i_gen;
	return (0);
}

/*
 * Common code for mount and quota.
 * Check that the user's argument is a reasonable
 * thing on which to mount, and return the device number if so.
 */
getmdev(devvpp, fname, ndp)
	struct vnode **devvpp;
	caddr_t fname;
	register struct nameidata *ndp;
{
	register struct vnode *vp;
	int error;

	ndp->ni_nameiop = LOOKUP | LOCKLEAF | FOLLOW;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = fname;
	if (error = namei(ndp)) {
		if (error == ENOENT)
			return (ENODEV);	/* needs translation */
		return (error);
	}
	vp = ndp->ni_vp;
	if (vp->v_type != VBLK) {
		vput(vp);
		return (ENOTBLK);
	}
	if (major(vp->v_rdev) >= nblkdev) {
		vput(vp);
		return (ENXIO);
	}
	iunlock(VTOI(vp));
	*devvpp = vp;
	return (0);
}
