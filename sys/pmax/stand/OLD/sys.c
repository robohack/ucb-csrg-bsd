/*
 * Copyright (c) 1992 Regents of the University of California.
 * All rights reserved.
 *
 * This module is believed to contain source code proprietary to AT&T.
 * Use and redistribution is subject to the Berkeley Software License
 * Agreement and your Software Agreement with AT&T (Western Electric).
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell.
 *
 *	@(#)sys.c	7.1 (Berkeley) 1/7/92
 */

#include "saio.h"
#include "ufs/dir.h"
#ifndef SMALL
#include "stat.h"
#endif
#include "../include/machMon.h"

ino_t	dlook();

int	errno;

struct dirstuff {
	int loc;
	struct iob *io;
};

static
openi(n, io)
	ino_t n;
	register struct iob *io;
{
	register struct dinode *dp;
	int cc;

	io->i_offset = 0;
	io->i_bn = fsbtodb(&io->i_fs, itod(&io->i_fs, n));
	io->i_cc = io->i_fs.fs_bsize;
	io->i_ma = io->i_buf;
	cc = devread(io);
	dp = (struct dinode *)io->i_buf;
	io->i_ino.i_din = dp[itoo(&io->i_fs, n)];
	return (cc);
}

static ino_t
find(path, file)
	register char *path;
	struct iob *file;
{
	register char *q;
	char c;
	ino_t i;

	if (path==NULL || *path=='\0') {
		printf("null path\n");
		return (0);
	}

	if (openi(ROOTINO, file) < 0) {
		printf("can\'t read root inode\n");
		return (0);
	}
	while (*path) {
		while (*path == '/')
			path++;
		q = path;
		while (*q != '/' && *q != '\0')
			q++;
		c = *q;
		*q = '\0';
		if (q == path)
			path = "." ;	/* "/" means "/." */

		if ((i = dlook(path, file)) == 0) {
			printf("%s: not found\n", path);
			return (0);
		}
		if (c == '\0')
			break;
		if (openi(i, file) < 0)
			return (0);
		*q = c;
		path = q;
	}
	return (i);
}

/*
 * Return the disk block number for the file offset block 'bn' or zero.
 */
static daddr_t
sbmap(io, bn)
	register struct iob *io;
	daddr_t bn;
{
	register struct inode *ip;
	int i, j, sh;
	daddr_t nb, *bap;

	ip = &io->i_ino;
	if (bn < 0) {
		printf("bn negative\n");
		return ((daddr_t)0);
	}

	/*
	 * blocks 0..NDADDR are direct blocks
	 */
	if (bn < NDADDR)
		return (ip->i_db[bn]);

	/*
	 * addresses NIADDR have single and double indirect blocks.
	 * the first step is to determine how many levels of indirection.
	 */
	sh = 1;
	bn -= NDADDR;
	for (j = NIADDR; j > 0; j--) {
		sh *= NINDIR(&io->i_fs);
		if (bn < sh)
			break;
		bn -= sh;
	}
	if (j == 0) {
		printf("bn ovf %d\n", bn);
		return ((daddr_t)0);
	}

	/*
	 * fetch the first indirect block address from the inode
	 */
	nb = ip->i_ib[NIADDR - j];
	if (nb == 0) {
		printf("bn void %d\n", bn);
		return ((daddr_t)0);
	}

	/*
	 * fetch through the indirect blocks
	 */
	for (; j <= NIADDR; j++) {
		if (blknos[j] != nb) {
			io->i_bn = fsbtodb(&io->i_fs, nb);
			io->i_ma = b[j];
			io->i_cc = io->i_fs.fs_bsize;
			if (devread(io) != io->i_fs.fs_bsize) {
				if (io->i_error)
					errno = io->i_error;
				printf("bn %d: read error\n", io->i_bn);
				return ((daddr_t)0);
			}
			blknos[j] = nb;
		}
		bap = (daddr_t *)b[j];
		sh /= NINDIR(&io->i_fs);
		i = (bn / sh) % NINDIR(&io->i_fs);
		nb = bap[i];
		if (nb == 0) {
			printf("bn void %d\n", bn);
			return ((daddr_t)0);
		}
	}
	return (nb);
}

/*
 * Search the current inode (directory) for the entry 's' and
 * return it's inode number or zero if not found.
 */
static ino_t
dlook(s, io)
	char *s;
	register struct iob *io;
{
	register struct direct *dp;
	register struct inode *ip;
	struct dirstuff dirp;
	int len;
	extern struct direct *readdir();

	if (s == NULL || *s == '\0')
		return (0);
	ip = &io->i_ino;
	if ((ip->i_mode & IFMT) != IFDIR) {
		printf("%s: not a directory\n", s);
		return (0);
	}
	if (ip->i_size == 0) {
		printf("%s: zero length directory\n", s);
		return (0);
	}
	len = strlen(s);
	dirp.loc = 0;
	dirp.io = io;
	for (dp = readdir(&dirp); dp != NULL; dp = readdir(&dirp)) {
		if (dp->d_namlen == len && !strcmp(s, dp->d_name))
			return (dp->d_ino);
	}
	return (0);
}

/*
 * get next entry in a directory.
 */
static struct direct *
readdir(dirp)
	register struct dirstuff *dirp;
{
	register struct direct *dp;
	register struct iob *io;
	daddr_t lbn, d;
	int off;

	io = dirp->io;
	for (;;) {
		if (dirp->loc >= io->i_ino.i_size)
			return (NULL);
		off = blkoff(&io->i_fs, dirp->loc);
		if (off == 0) {
			lbn = lblkno(&io->i_fs, dirp->loc);
			d = sbmap(io, lbn);
			if (d == 0)
				return (NULL);
			io->i_bn = fsbtodb(&io->i_fs, d);
			io->i_ma = io->i_buf;
			io->i_cc = blksize(&io->i_fs, &io->i_ino, lbn);
			if (devread(io) < 0) {
				errno = io->i_error;
				printf("bn %d: directory read error\n",
					io->i_bn);
				return (NULL);
			}
		}
		dp = (struct direct *)(io->i_buf + off);
		dirp->loc += dp->d_reclen;
		if (dp->d_ino == 0)
			continue;
		return (dp);
	}
}

Lseek(fdesc, addr, ptr)
	int fdesc, ptr;
	off_t addr;
{
	register struct iob *io;

#ifdef SMALL
	if (ptr != 0) {
		printf("Seek not from beginning of file\n");
		errno = EOFFSET;
		return (-1);
	}
#endif SMALL
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES ||
	    ((io = &iob[fdesc])->i_flgs & F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}
	io->i_offset = addr;
	io->i_bn = addr / DEV_BSIZE;
	io->i_cc = 0;
	return (0);
}

Getc(fdesc)
	int fdesc;
{
	register struct iob *io;
	register struct fs *fs;
	register char *p;
	int c, lbn, off, size, diff;

	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES ||
	    ((io = &iob[fdesc])->i_flgs & F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}
	p = io->i_ma;
	if (io->i_cc <= 0) {
		if ((io->i_flgs & F_FILE) != 0) {
			diff = io->i_ino.i_size - io->i_offset;
			if (diff <= 0)
				return (-1);
			fs = &io->i_fs;
			lbn = lblkno(fs, io->i_offset);
			io->i_bn = fsbtodb(fs, sbmap(io, lbn));
			off = blkoff(fs, io->i_offset);
			size = blksize(fs, &io->i_ino, lbn);
		} else {
			io->i_bn = io->i_offset / DEV_BSIZE;
			off = 0;
			size = DEV_BSIZE;
		}
		io->i_ma = io->i_buf;
		io->i_cc = size;
		if (devread(io) < 0) {
			errno = io->i_error;
			return (-1);
		}
		if ((io->i_flgs & F_FILE) != 0) {
			if (io->i_offset - off + size >= io->i_ino.i_size)
				io->i_cc = diff + off;
			io->i_cc -= off;
		}
		p = &io->i_buf[off];
	}
	io->i_cc--;
	io->i_offset++;
	c = (unsigned)*p++;
	io->i_ma = p;
	return (c);
}

Read(fdesc, buf, count)
	int fdesc, count;
	char *buf;
{
	register i, size;
	register struct iob *file;
	register struct fs *fs;
	int lbn, off;

	errno = 0;
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES ||
	    ((file = &iob[fdesc])->i_flgs & F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}
	if ((file->i_flgs & F_READ) == 0) {
		errno = EBADF;
		return (-1);
	}
	if ((file->i_flgs & F_FILE) == 0) {
		file->i_cc = count;
		file->i_ma = buf;
		file->i_bn = file->i_offset / DEV_BSIZE;
		i = devread(file);
		file->i_offset += count;
		if (i < 0)
			errno = file->i_error;
		return (i);
	}
	if (file->i_offset + count > file->i_ino.i_size)
		count = file->i_ino.i_size - file->i_offset;
	if ((i = count) <= 0)
		return (0);
	/*
	 * While reading full blocks, do I/O into user buffer.
	 * Anything else uses Getc().
	 */
	fs = &file->i_fs;
	while (i) {
		off = blkoff(fs, file->i_offset);
		lbn = lblkno(fs, file->i_offset);
		size = blksize(fs, &file->i_ino, lbn);
		if (off == 0 && size <= i) {
			file->i_bn = fsbtodb(fs, sbmap(file, lbn));
			file->i_cc = size;
			file->i_ma = buf;
			if (devread(file) < 0) {
				errno = file->i_error;
				return (-1);
			}
			file->i_offset += size;
			file->i_cc = 0;
			buf += size;
			i -= size;
		} else {
			size -= off;
			if (size > i)
				size = i;
			i -= size;
			do {
				*buf++ = Getc(fdesc + 3);
			} while (--size);
		}
	}
	return (count);
}

#ifndef SMALL
write(fdesc, buf, count)
	int fdesc, count;
	char *buf;
{
	register i;
	register struct iob *file;

	errno = 0;
	if (fdesc >= 0 && fdesc <= 2) {
		i = count;
		while (i--)
			putchar(0, *buf++);
		return (count);
	}
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES ||
	    ((file = &iob[fdesc])->i_flgs & F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}
	if ((file->i_flgs & F_WRITE) == 0) {
		errno = EBADF;
		return (-1);
	}
	file->i_cc = count;
	file->i_ma = buf;
	file->i_bn = file->i_offset / DEV_BSIZE;
	i = devwrite(file);
	file->i_offset += count;
	if (i < 0)
		errno = file->i_error;
	return (i);
}
#endif SMALL

/*
 * Open a file or device and return a file descriptor or -1 if error.
 */
Open(str, how)
	char *str;
	int how;
{
	register char *cp;
	register struct iob *file;
	register struct devsw *dp;
	int fdesc;
	ino_t i;
	char c;
	static int first = 1;

	/*
	 * I think this is the only place where we would otherwise have
	 * to clear bss variables.
	 */
	if (first) {
		first = 0;
		for (fdesc = 0; fdesc < NFILES; fdesc++)
			iob[fdesc].i_flgs = 0;
	}

	for (fdesc = 0, file = iob; fdesc < NFILES; fdesc++, file++)
		if (file->i_flgs == 0)
			goto gotfile;
	printf("No more file slots\n");
	return (-1);
gotfile:
	file->i_flgs |= F_ALLOC;

	for (cp = str; *cp && *cp != '('; cp++)
		;
	if (*cp != '(') {
	baddev:
		printf("Unknown or bad device name \'%s\'\n", str);
		errno = ENXIO;
	err:
		file->i_flgs = 0;
		return (-1);
	}
	*cp = '\0';
	for (dp = devsw; dp->dv_name; dp++)
		if (!strcmp(str, dp->dv_name))
			break;
	*cp++ = '(';
	if (dp->dv_name == NULL)
		goto baddev;
	file->i_dev = dp - devsw;
	while (*cp != ')') {
		if (*cp++ == '\0')
			goto baddev;
	}
	c = *++cp;
	*cp = '\0';
	if ((file->i_unit = open(str, how)) <= 0) {
		printf("Can\'t open device \'%s\'\n", str);
		goto err;
	}
	*cp = c;
	if (c == '\0') {
		/* I/O to raw device */
		file->i_flgs |= how + 1;
		file->i_cc = 0;
		file->i_offset = 0;
		return (fdesc + 3);
	}
#ifdef SMALL
	if (how != 0) {
		printf("Can\'t write files yet.. Sorry\n");
		goto err;
	}
#endif SMALL
	file->i_ma = (char *)&file->i_fs;
	file->i_cc = SBSIZE;
	file->i_bn = SBLOCK;
	file->i_boff = 0;
	file->i_offset = 0;
	if (devread(file) < 0) {
		errno = file->i_error;
		printf("Super block read error\n");
		goto err;
	}
	/* check to make sure this really is a super block */
	if (file->i_fs.fs_magic != FS_MAGIC ||
	    file->i_fs.fs_bsize > MAXBSIZE ||
	    file->i_fs.fs_bsize < sizeof(struct fs)) {
		printf("Bad super block (magic %x)\n",
			file->i_fs.fs_magic);
		goto err;
	}
	/* start of file system (skip boot & super blk) */
	file->i_boff = 0;
	if ((i = find(cp, file)) == 0) {
		errno = ESRCH;
		goto err;
	}
	if (openi(i, file) < 0) {
		errno = file->i_error;
		goto err;
	}
	file->i_offset = 0;
	file->i_cc = 0;
	file->i_flgs |= F_FILE | (how + 1);
	return (fdesc + 3);
}

Close(fdesc)
	int fdesc;
{
	struct iob *file;

	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES ||
	    ((file = &iob[fdesc])->i_flgs & F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}
	if ((file->i_flgs & F_FILE) == 0)
		close(file->i_unit);
	file->i_flgs = 0;
	return (0);
}

Exit()
{
	_stop("Exit called");
}

_stop(s)
	char *s;
{
	static int stopped = 0;
	int i;

	if (!stopped) {
		stopped++;
		for (i = 0; i < NFILES; i++)
			if (iob[i].i_flgs != 0)
				Close(i);
	}
	printf("%s\n", s);
	(mach_MonFuncs.m_reinit)();
}

#ifndef SMALL
ioctl(fdesc, cmd, arg)
	int fdesc, cmd;
	char *arg;
{
	register struct iob *file;
	int error = 0;

	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES ||
	    ((file = &iob[fdesc])->i_flgs & F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}
	switch (cmd) {

	case SAIOHDR:
		file->i_flgs |= F_HDR;
		break;

	case SAIOCHECK:
		file->i_flgs |= F_CHECK;
		break;

	case SAIOHCHECK:
		file->i_flgs |= F_HCHECK;
		break;

	case SAIONOBAD:
		file->i_flgs |= F_NBSF;
		break;

	case SAIODOBAD:
		file->i_flgs &= ~F_NBSF;
		break;

	default:
#ifdef notdef
		error = (*mach_MonFuncs.m_ioctl)(io->i_unit, cmd, arg);
#else
		error = ECMD;
#endif
		break;
	}
	if (error < 0)
		errno = file->i_error;
	return (error);
}

extern char end;
static caddr_t theend = &end;

caddr_t
brk(addr)
	char *addr;
{
	char stkloc;

	if (addr > &stkloc || addr < &end)
		return((caddr_t)-1);
	if (addr > theend)
		bzero(theend, addr-theend);
	theend = addr;
	return(0);
}

caddr_t
sbrk(incr)
	int incr;
{
	caddr_t obrk, brk();

	if (theend == (caddr_t)0)
		theend = &end;
	obrk = theend;
	if (brk(theend + incr) == (caddr_t)-1)
		return((caddr_t)-1);
	return(obrk);
}

getpagesize()
{
	return(NBPG);
}

getdtablesize()
{
	return(NFILES);
}

fstat(fdesc, sb)
	struct stat *sb;
{
	register struct iob *io;

	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES ||
	    ((io = &iob[fdesc])->i_flgs & F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}
	/* only important stuff */
	sb->st_mode = io->i_ino.i_mode;
	sb->st_uid = io->i_ino.i_uid;
	sb->st_gid = io->i_ino.i_gid;
	sb->st_size = io->i_ino.i_size;
	return (0);
}

stat(str, sb)
{
	/* the easy way */
	int f, rv = 0;

	f = open(str, 0);
	if (f < 0 || fstat(f, sb) < 0)
		rv = -1;
	(void) Close(f);
	return(rv);
}
#endif SMALL
