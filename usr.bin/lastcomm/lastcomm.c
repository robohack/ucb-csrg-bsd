/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1980, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)lastcomm.c	8.2 (Berkeley) 4/29/95";
#endif /* not lint */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/acct.h>

#include <ctype.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <struct.h>
#include <unistd.h>
#include <utmp.h>
#include "pathnames.h"

time_t	 expand __P((u_int));
char	*flagbits __P((int));
char	*getdev __P((dev_t));
int	 requested __P((char *[], struct acct *));
void	 usage __P((void));
char	*user_from_uid();

int
main(argc, argv)
	int argc;
	char *argv[];
{
	register char *p;
	struct acct ab;
	struct stat sb;
	FILE *fp;
	off_t size;
	time_t t;
	int ch;
	char *acctfile;

	acctfile = _PATH_ACCT;
	while ((ch = getopt(argc, argv, "f:")) != EOF)
		switch((char)ch) {
		case 'f':
			acctfile = optarg;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	/* Open the file. */
	if ((fp = fopen(acctfile, "r")) == NULL || fstat(fileno(fp), &sb))
		err(1, "%s", acctfile);

	/*
	 * Round off to integral number of accounting records, probably
	 * not necessary, but it doesn't hurt.
	 */
	size = sb.st_size - sb.st_size % sizeof(struct acct);

	/* Check if any records to display. */
	if (size < sizeof(struct acct))
		exit(0);

	/*
	 * Seek to before the last entry in the file; use lseek(2) in case
	 * the file is bigger than a "long".
	 */
	size -= sizeof(struct acct);
	if (lseek(fileno(fp), size, SEEK_SET) == -1)
		err(1, "%s", acctfile);

	for (;;) {
		if (fread(&ab, sizeof(struct acct), 1, fp) != 1)
			err(1, "%s", acctfile);

		if (fseek(fp, 2 * -(long)sizeof(struct acct), SEEK_CUR) == -1)
			err(1, "%s", acctfile);

		if (size == 0)
			break;
		size -= sizeof(struct acct);

		if (ab.ac_comm[0] == '\0') {
			ab.ac_comm[0] = '?';
			ab.ac_comm[1] = '\0';
		} else
			for (p = &ab.ac_comm[0];
			    p < &ab.ac_comm[fldsiz(acct, ac_comm)] && *p; ++p)
				if (!isprint(*p))
					*p = '?';
		if (*argv && !requested(argv, &ab))
			continue;

		t = expand(ab.ac_utime) + expand(ab.ac_stime);
		(void)printf("%-*s %-7s %-*s %-*s %6.2f secs %.16s\n",
			fldsiz(acct, ac_comm), ab.ac_comm, flagbits(ab.ac_flag),
			UT_NAMESIZE, user_from_uid(ab.ac_uid, 0),
			UT_LINESIZE, getdev(ab.ac_tty),
			t / (double)AHZ, ctime(&ab.ac_btime));
	}
	exit(0);
}

time_t
expand(t)
	u_int t;
{
	register time_t nt;

	nt = t & 017777;
	t >>= 13;
	while (t) {
		t--;
		nt <<= 3;
	}
	return (nt);
}

char *
flagbits(f)
	register int f;
{
	static char flags[20] = "-";
	char *p;

#define	BIT(flag, ch)	if (f & flag) *p++ = ch

	p = flags + 1;
	BIT(ASU, 'S');
	BIT(AFORK, 'F');
	BIT(ACOMPAT, 'C');
	BIT(ACORE, 'D');
	BIT(AXSIG, 'X');
	*p = '\0';
	return (flags);
}

int
requested(argv, acp)
	register char *argv[];
	register struct acct *acp;
{
	do {
		if (!strcmp(user_from_uid(acp->ac_uid, 0), *argv))
			return (1);
		if (!strcmp(getdev(acp->ac_tty), *argv))
			return (1);
		if (!strncmp(acp->ac_comm, *argv, fldsiz(acct, ac_comm)))
			return (1);
	} while (*++argv);
	return (0);
}

char *
getdev(dev)
	dev_t dev;
{
	static dev_t lastdev = (dev_t)-1;
	static char *lastname;

	if (dev == NODEV)			/* Special case. */
		return ("__");
	if (dev == lastdev)			/* One-element cache. */
		return (lastname);
	lastdev = dev;
	if ((lastname = devname(dev, S_IFCHR)) == NULL)
		lastname = "??";
	return (lastname);
}

void
usage()
{
	(void)fprintf(stderr,
	    "lastcomm [ -f file ] [command ...] [user ...] [tty ...]\n");
	exit(1);
}
