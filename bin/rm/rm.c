/*-
 * Copyright (c) 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1990, 1993, 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)rm.c	8.7 (Berkeley) 10/18/94";
#endif /* not lint */

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int dflag, eval, fflag, iflag, Pflag, Wflag, stdin_ok;

int	check __P((char *, char *, struct stat *));
void	checkdot __P((char **));
void	rm_file __P((char **));
void	rm_overwrite __P((char *, struct stat *));
void	rm_tree __P((char **));
void	usage __P((void));

/*
 * rm --
 *	This rm is different from historic rm's, but is expected to match
 *	POSIX 1003.2 behavior.  The most visible difference is that -f
 *	has two specific effects now, ignore non-existent files and force
 * 	file removal.
 */
int
main(argc, argv)
	int argc;
	char *argv[];
{
	int ch, rflag;

	Pflag = rflag = 0;
	while ((ch = getopt(argc, argv, "dfiPRrW")) != EOF)
		switch(ch) {
		case 'd':
			dflag = 1;
			break;
		case 'f':
			fflag = 1;
			iflag = 0;
			break;
		case 'i':
			fflag = 0;
			iflag = 1;
			break;
		case 'P':
			Pflag = 1;
			break;
		case 'R':
		case 'r':			/* Compatibility. */
			rflag = 1;
			break;
		case 'W':
			Wflag = 1;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc < 1)
		usage();

	checkdot(argv);
	if (!*argv)
		exit (eval);

	stdin_ok = isatty(STDIN_FILENO);

	if (rflag)
		rm_tree(argv);
	else
		rm_file(argv);
	exit (eval);
}

void
rm_tree(argv)
	char **argv;
{
	FTS *fts;
	FTSENT *p;
	int needstat;
	int flags;

	/*
	 * Remove a file hierarchy.  If forcing removal (-f), or interactive
	 * (-i) or can't ask anyway (stdin_ok), don't stat the file.
	 */
	needstat = !fflag && !iflag && stdin_ok;

	/*
	 * If the -i option is specified, the user can skip on the pre-order
	 * visit.  The fts_number field flags skipped directories.
	 */
#define	SKIPPED	1

	flags = FTS_PHYSICAL;
	if (!needstat)
		flags |= FTS_NOSTAT;
	if (Wflag)
		flags |= FTS_WHITEOUT;
	if (!(fts = fts_open(argv, flags, (int (*)())NULL)))
		err(1, NULL);
	while ((p = fts_read(fts)) != NULL) {
		switch (p->fts_info) {
		case FTS_DNR:
			if (!fflag || p->fts_errno != ENOENT) {
				warnx("%s: %s",
				    p->fts_path, strerror(p->fts_errno));
				eval = 1;
			}
			continue;
		case FTS_ERR:
			errx(1, "%s: %s", p->fts_path, strerror(p->fts_errno));
		case FTS_NS:
			/*
			 * FTS_NS: assume that if can't stat the file, it
			 * can't be unlinked.
			 */
			if (!needstat)
				break;
			if (!fflag || p->fts_errno != ENOENT) {
				warnx("%s: %s",
				    p->fts_path, strerror(p->fts_errno));
				eval = 1;
			}
			continue;
		case FTS_D:
			/* Pre-order: give user chance to skip. */
			if (iflag && !check(p->fts_path, p->fts_accpath,
			    p->fts_statp)) {
				(void)fts_set(fts, p, FTS_SKIP);
				p->fts_number = SKIPPED;
			}
			continue;
		case FTS_DP:
			/* Post-order: see if user skipped. */
			if (p->fts_number == SKIPPED)
				continue;
			break;
		}
		if (!fflag &&
		    !check(p->fts_path, p->fts_accpath, p->fts_statp))
			continue;

		/*
		 * If we can't read or search the directory, may still be
		 * able to remove it.  Don't print out the un{read,search}able
		 * message unless the remove fails.
		 */
		switch (p->fts_info) {
		case FTS_DP:
		case FTS_DNR:
			if (!rmdir(p->fts_accpath))
				continue;
			if (errno == ENOENT) {
				if (fflag)
					continue;
			} else if (p->fts_info != FTS_DP)
				warnx("%s: unable to read", p->fts_path);
			break;

		case FTS_W:
			if (!undelete(p->fts_accpath) ||
			    fflag && errno == ENOENT)
				continue;
			break;

		default:
			if (Pflag)
				rm_overwrite(p->fts_accpath, NULL);
			if (!unlink(p->fts_accpath) || fflag && errno == ENOENT)
				continue;
		}
		warn("%s", p->fts_path);
		eval = 1;
	}
	if (errno)
		err(1, "fts_read");
}

void
rm_file(argv)
	char **argv;
{
	struct stat sb;
	int df, rval;
	char *f;

	df = dflag;
	/*
	 * Remove a file.  POSIX 1003.2 states that, by default, attempting
	 * to remove a directory is an error, so must always stat the file.
	 */
	while ((f = *argv++) != NULL) {
		/* Assume if can't stat the file, can't unlink it. */
		if (lstat(f, &sb)) {
			if (Wflag) {
				sb.st_mode = S_IFWHT|S_IWUSR|S_IRUSR;
			} else {
				if (!fflag || errno != ENOENT) {
					warn("%s", f);
					eval = 1;
				}
				continue;
			}
		} else if (Wflag) {
			warnx("%s: %s", f, strerror(EEXIST));
			eval = 1;
			continue;
		}

		if (S_ISDIR(sb.st_mode) && !df) {
			warnx("%s: is a directory", f);
			eval = 1;
			continue;
		}
		if (!fflag && !S_ISWHT(sb.st_mode) && !check(f, f, &sb))
			continue;
		if (S_ISWHT(sb.st_mode))
			rval = undelete(f);
		else if (S_ISDIR(sb.st_mode))
			rval = rmdir(f);
		else {
			if (Pflag)
				rm_overwrite(f, &sb);
			rval = unlink(f);
		}
		if (rval && (!fflag || errno != ENOENT)) {
			warn("%s", f);
			eval = 1;
		}
	}
}

/*
 * rm_overwrite --
 *	Overwrite the file 3 times with varying bit patterns.
 *
 * XXX
 * This is a cheap way to *really* delete files.  Note that only regular
 * files are deleted, directories (and therefore names) will remain.
 * Also, this assumes a fixed-block file system (like FFS, or a V7 or a
 * System V file system).  In a logging file system, you'll have to have
 * kernel support.
 */
void
rm_overwrite(file, sbp)
	char *file;
	struct stat *sbp;
{
	struct stat sb;
	off_t len;
	int fd, wlen;
	char buf[8 * 1024];

	fd = -1;
	if (sbp == NULL) {
		if (lstat(file, &sb))
			goto err;
		sbp = &sb;
	}
	if (!S_ISREG(sbp->st_mode))
		return;
	if ((fd = open(file, O_WRONLY, 0)) == -1)
		goto err;

#define	PASS(byte) {							\
	memset(buf, byte, sizeof(buf));					\
	for (len = sbp->st_size; len > 0; len -= wlen) {		\
		wlen = len < sizeof(buf) ? len : sizeof(buf);		\
		if (write(fd, buf, wlen) != wlen)			\
			goto err;					\
	}								\
}
	PASS(0xff);
	if (fsync(fd) || lseek(fd, (off_t)0, SEEK_SET))
		goto err;
	PASS(0x00);
	if (fsync(fd) || lseek(fd, (off_t)0, SEEK_SET))
		goto err;
	PASS(0xff);
	if (!fsync(fd) && !close(fd))
		return;

err:	eval = 1;
	warn("%s", file);
}


int
check(path, name, sp)
	char *path, *name;
	struct stat *sp;
{
	int ch, first;
	char modep[15];

	/* Check -i first. */
	if (iflag)
		(void)fprintf(stderr, "remove %s? ", path);
	else {
		/*
		 * If it's not a symbolic link and it's unwritable and we're
		 * talking to a terminal, ask.  Symbolic links are excluded
		 * because their permissions are meaningless.  Check stdin_ok
		 * first because we may not have stat'ed the file.
		 */
		if (!stdin_ok || S_ISLNK(sp->st_mode) || !access(name, W_OK))
			return (1);
		strmode(sp->st_mode, modep);
		(void)fprintf(stderr, "override %s%s%s/%s for %s? ",
		    modep + 1, modep[9] == ' ' ? "" : " ",
		    user_from_uid(sp->st_uid, 0),
		    group_from_gid(sp->st_gid, 0), path);
	}
	(void)fflush(stderr);

	first = ch = getchar();
	while (ch != '\n' && ch != EOF)
		ch = getchar();
	return (first == 'y');
}

#define ISDOT(a)	((a)[0] == '.' && (!(a)[1] || (a)[1] == '.' && !(a)[2]))
void
checkdot(argv)
	char **argv;
{
	char *p, **save, **t;
	int complained;

	complained = 0;
	for (t = argv; *t;) {
		if ((p = strrchr(*t, '/')) != NULL)
			++p;
		else
			p = *t;
		if (ISDOT(p)) {
			if (!complained++)
				warnx("\".\" and \"..\" may not be removed");
			eval = 1;
			for (save = t; (t[0] = t[1]) != NULL; ++t);
			t = save;
		} else
			++t;
	}
}

void
usage()
{

	(void)fprintf(stderr, "usage: rm [-dfiRr] file ...\n");
	exit(1);
}
