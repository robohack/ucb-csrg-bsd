/*
 * Copyright (c) 1989, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)mount.h	8.21 (Berkeley) 5/20/95
 */

#ifndef KERNEL
#include <sys/ucred.h>
#endif
#include <sys/queue.h>
#include <sys/lock.h>
#include <net/radix.h>
#include <sys/socket.h>		/* XXX for AF_MAX */

typedef struct { int32_t val[2]; } fsid_t;	/* file system id type */

/*
 * File identifier.
 * These are unique per filesystem on a single machine.
 */
#define	MAXFIDSZ	16

struct fid {
	u_short		fid_len;		/* length of data in bytes */
	u_short		fid_reserved;		/* force longword alignment */
	char		fid_data[MAXFIDSZ];	/* data (variable length) */
};

/*
 * file system statistics
 */

#define MFSNAMELEN	16	/* length of fs type name, including null */
#define	MNAMELEN	90	/* length of buffer for returned name */

struct statfs {
	short	f_type;			/* filesystem type number */
	short	f_flags;		/* copy of mount flags */
	long	f_bsize;		/* fundamental file system block size */
	long	f_iosize;		/* optimal transfer block size */
	long	f_blocks;		/* total data blocks in file system */
	long	f_bfree;		/* free blocks in fs */
	long	f_bavail;		/* free blocks avail to non-superuser */
	long	f_files;		/* total file nodes in file system */
	long	f_ffree;		/* free file nodes in fs */
	fsid_t	f_fsid;			/* file system id */
	uid_t	f_owner;		/* user that mounted the filesystem */
	long	f_spare[4];		/* spare for later */
	char	f_fstypename[MFSNAMELEN]; /* fs type name */
	char	f_mntonname[MNAMELEN];	/* directory on which mounted */
	char	f_mntfromname[MNAMELEN];/* mounted filesystem */
};

/*
 * Structure per mounted file system.  Each mounted file system has an
 * array of operations and an instance record.  The file systems are
 * put on a doubly linked list.
 */
LIST_HEAD(vnodelst, vnode);

struct mount {
	CIRCLEQ_ENTRY(mount) mnt_list;		/* mount list */
	struct vfsops	*mnt_op;		/* operations on fs */
	struct vfsconf	*mnt_vfc;		/* configuration info */
	struct vnode	*mnt_vnodecovered;	/* vnode we mounted on */
	struct vnodelst	mnt_vnodelist;		/* list of vnodes this mount */
	struct lock	mnt_lock;		/* mount structure lock */
	int		mnt_flag;		/* flags */
	int		mnt_maxsymlinklen;	/* max size of short symlink */
	struct statfs	mnt_stat;		/* cache of filesystem stats */
	qaddr_t		mnt_data;		/* private data */
};

/*
 * Mount flags.
 *
 * Unmount uses MNT_FORCE flag.
 */
#define	MNT_RDONLY	0x00000001	/* read only filesystem */
#define	MNT_SYNCHRONOUS	0x00000002	/* file system written synchronously */
#define	MNT_NOEXEC	0x00000004	/* can't exec from filesystem */
#define	MNT_NOSUID	0x00000008	/* don't honor setuid bits on fs */
#define	MNT_NODEV	0x00000010	/* don't interpret special files */
#define	MNT_UNION	0x00000020	/* union with underlying filesystem */
#define	MNT_ASYNC	0x00000040	/* file system written asynchronously */

/*
 * exported mount flags.
 */
#define	MNT_EXRDONLY	0x00000080	/* exported read only */
#define	MNT_EXPORTED	0x00000100	/* file system is exported */
#define	MNT_DEFEXPORTED	0x00000200	/* exported to the world */
#define	MNT_EXPORTANON	0x00000400	/* use anon uid mapping for everyone */
#define	MNT_EXKERB	0x00000800	/* exported with Kerberos uid mapping */

/*
 * Flags set by internal operations.
 */
#define	MNT_LOCAL	0x00001000	/* filesystem is stored locally */
#define	MNT_QUOTA	0x00002000	/* quotas are enabled on filesystem */
#define	MNT_ROOTFS	0x00004000	/* identifies the root filesystem */

/*
 * Mask of flags that are visible to statfs()
 */
#define	MNT_VISFLAGMASK	0x0000ffff

/*
 * External filesystem control flags.
 */
#define	MNT_UPDATE	0x00010000	/* not a real mount, just an update */
#define	MNT_DELEXPORT	0x00020000	/* delete export host lists */
#define	MNT_RELOAD	0x00040000	/* reload filesystem data */
#define	MNT_FORCE	0x00080000	/* force unmount or readonly change */
/*
 * Internal filesystem control flags.
 *
 * MNT_UNMOUNT locks the mount entry so that name lookup cannot proceed
 * past the mount point.  This keeps the subtree stable during mounts
 * and unmounts.
 */
#define MNT_UNMOUNT	0x01000000	/* unmount in progress */
#define	MNT_MWAIT	0x02000000	/* waiting for unmount to finish */
#define MNT_WANTRDWR	0x04000000	/* upgrade to read/write requested */

/*
 * Sysctl CTL_VFS definitions.
 *
 * Second level identifier specifies which filesystem. Second level
 * identifier VFS_GENERIC returns information about all filesystems.
 */
#define	VFS_GENERIC		0	/* generic filesystem information */
/*
 * Third level identifiers for VFS_GENERIC are given below; third
 * level identifiers for specific filesystems are given in their
 * mount specific header files.
 */
#define VFS_MAXTYPENUM	1	/* int: highest defined filesystem type */
#define VFS_CONF	2	/* struct: vfsconf for filesystem given
				   as next argument */

/*
 * Flags for various system call interfaces.
 *
 * waitfor flags to vfs_sync() and getfsstat()
 */
#define MNT_WAIT	1
#define MNT_NOWAIT	2

/*
 * Generic file handle
 */
struct fhandle {
	fsid_t	fh_fsid;	/* File system id of mount point */
	struct	fid fh_fid;	/* File sys specific id */
};
typedef struct fhandle	fhandle_t;

/*
 * Export arguments for local filesystem mount calls.
 */
struct export_args {
	int	ex_flags;		/* export related flags */
	uid_t	ex_root;		/* mapping for root uid */
	struct	ucred ex_anon;		/* mapping for anonymous user */
	struct	sockaddr *ex_addr;	/* net address to which exported */
	int	ex_addrlen;		/* and the net address length */
	struct	sockaddr *ex_mask;	/* mask of valid bits in saddr */
	int	ex_masklen;		/* and the smask length */
};

/*
 * Filesystem configuration information. One of these exists for each
 * type of filesystem supported by the kernel. These are searched at
 * mount time to identify the requested filesystem.
 */
struct vfsconf {
	struct	vfsops *vfc_vfsops;	/* filesystem operations vector */
	char	vfc_name[MFSNAMELEN];	/* filesystem type name */
	int	vfc_typenum;		/* historic filesystem type number */
	int	vfc_refcount;		/* number mounted of this type */
	int	vfc_flags;		/* permanent flags */
	int	(*vfc_mountroot)(void);	/* if != NULL, routine to mount root */
	struct	vfsconf *vfc_next;	/* next in list */
};

#ifdef KERNEL

extern int maxvfsconf;		/* highest defined filesystem type */
extern struct vfsconf *vfsconf;	/* head of list of filesystem types */

/*
 * Operations supported on mounted file system.
 */
#ifdef __STDC__
struct nameidata;
struct mbuf;
#endif

struct vfsops {
	int	(*vfs_mount)	__P((struct mount *mp, char *path, caddr_t data,
				    struct nameidata *ndp, struct proc *p));
	int	(*vfs_start)	__P((struct mount *mp, int flags,
				    struct proc *p));
	int	(*vfs_unmount)	__P((struct mount *mp, int mntflags,
				    struct proc *p));
	int	(*vfs_root)	__P((struct mount *mp, struct vnode **vpp));
	int	(*vfs_quotactl)	__P((struct mount *mp, int cmds, uid_t uid,
				    caddr_t arg, struct proc *p));
	int	(*vfs_statfs)	__P((struct mount *mp, struct statfs *sbp,
				    struct proc *p));
	int	(*vfs_sync)	__P((struct mount *mp, int waitfor,
				    struct ucred *cred, struct proc *p));
	int	(*vfs_vget)	__P((struct mount *mp, ino_t ino,
				    struct vnode **vpp));
	int	(*vfs_fhtovp)	__P((struct mount *mp, struct fid *fhp,
				    struct mbuf *nam, struct vnode **vpp,
				    int *exflagsp, struct ucred **credanonp));
	int	(*vfs_vptofh)	__P((struct vnode *vp, struct fid *fhp));
	int	(*vfs_init)	__P((struct vfsconf *));
	int	(*vfs_sysctl)	__P((int *, u_int, void *, size_t *, void *,
				    size_t, struct proc *));
};

#define VFS_MOUNT(MP, PATH, DATA, NDP, P) \
	(*(MP)->mnt_op->vfs_mount)(MP, PATH, DATA, NDP, P)
#define VFS_START(MP, FLAGS, P)	  (*(MP)->mnt_op->vfs_start)(MP, FLAGS, P)
#define VFS_UNMOUNT(MP, FORCE, P) (*(MP)->mnt_op->vfs_unmount)(MP, FORCE, P)
#define VFS_ROOT(MP, VPP)	  (*(MP)->mnt_op->vfs_root)(MP, VPP)
#define VFS_QUOTACTL(MP,C,U,A,P)  (*(MP)->mnt_op->vfs_quotactl)(MP, C, U, A, P)
#define VFS_STATFS(MP, SBP, P)	  (*(MP)->mnt_op->vfs_statfs)(MP, SBP, P)
#define VFS_SYNC(MP, WAIT, C, P)  (*(MP)->mnt_op->vfs_sync)(MP, WAIT, C, P)
#define VFS_VGET(MP, INO, VPP)	  (*(MP)->mnt_op->vfs_vget)(MP, INO, VPP)
#define VFS_FHTOVP(MP, FIDP, NAM, VPP, EXFLG, CRED) \
	(*(MP)->mnt_op->vfs_fhtovp)(MP, FIDP, NAM, VPP, EXFLG, CRED)
#define	VFS_VPTOFH(VP, FIDP)	  (*(VP)->v_mount->mnt_op->vfs_vptofh)(VP, FIDP)

/*
 * Network address lookup element
 */
struct netcred {
	struct	radix_node netc_rnodes[2];
	int	netc_exflags;
	struct	ucred netc_anon;
};

/*
 * Network export information
 */
struct netexport {
	struct	netcred ne_defexported;		      /* Default export */
	struct	radix_node_head *ne_rtable[AF_MAX+1]; /* Individual exports */
};

/*
 * exported vnode operations
 */
int	vfs_busy __P((struct mount *, int, struct simplelock *, struct proc *));
int	vfs_export __P((struct mount *, struct netexport *,
	    struct export_args *));
struct	netcred *vfs_export_lookup __P((struct mount *, struct netexport *,
	    struct mbuf *));
void	vfs_getnewfsid __P((struct mount *));
struct	mount *vfs_getvfs __P((fsid_t *));
int	vfs_mountedon __P((struct vnode *));
int	vfs_mountroot __P((void));
int	vfs_rootmountalloc __P((char *, char *, struct mount **));
void	vfs_unbusy __P((struct mount *, struct proc *));
void	vfs_unmountall __P((void));
extern	CIRCLEQ_HEAD(mntlist, mount) mountlist;
extern	struct simplelock mountlist_slock;

#else /* !KERNEL */

#include <sys/cdefs.h>

__BEGIN_DECLS
int	fstatfs __P((int, struct statfs *));
int	getfh __P((const char *, fhandle_t *));
int	getfsstat __P((struct statfs *, long, int));
int	getmntinfo __P((struct statfs **, int));
int	mount __P((const char *, const char *, int, void *));
int	statfs __P((const char *, struct statfs *));
int	unmount __P((const char *, int));
__END_DECLS

#endif /* KERNEL */
