/*
 * Copyright (c) 1988, 1990 Regents of the University of California.
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
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1988 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)wall.c	5.11 (Berkeley) 2/24/90";
#endif /* not lint */

/*
 * This program is not related to David Wall, whose Stanford Ph.D. thesis
 * is entitled "Mechanisms for Broadcast and Selective Broadcast".
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <utmp.h>
#include <pwd.h>
#include <stdio.h>
#include <paths.h>

#define	IGNOREUSER	"sleeper"

int nobanner;
int mbufsize;
char *mbuf;

/* ARGSUSED */
main(argc, argv)
	int argc;
	char **argv;
{
	extern int optind;
	int ch;
	struct iovec iov;
	struct utmp utmp;
	FILE *fp;
	char *p, *ttymsg();

	while ((ch = getopt(argc, argv, "n")) != EOF)
		switch (ch) {
		case 'n':
			/* undoc option for shutdown: suppress banner */
			if (geteuid() == 0)
				nobanner = 1;
			break;
		case '?':
		default:
usage:
			(void)fprintf(stderr, "usage: wall [file]\n");
			exit(1);
		}
	argc -= optind;
	argv += optind;
	if (argc > 1)
		goto usage;

	makemsg(*argv);

	if (!(fp = fopen(_PATH_UTMP, "r"))) {
		(void)fprintf(stderr, "wall: cannot read %s.\n", _PATH_UTMP);
		exit(1);
	}
	iov.iov_base = mbuf;
	iov.iov_len = mbufsize;
	/* NOSTRICT */
	while (fread((char *)&utmp, sizeof(utmp), 1, fp) == 1) {
		if (!utmp.ut_name[0] ||
		    !strncmp(utmp.ut_name, IGNOREUSER, sizeof(utmp.ut_name)))
			continue;
		if (p = ttymsg(&iov, 1, utmp.ut_line, 1))
			(void)fprintf(stderr, "wall: %s\n", p);
	}
	exit(0);
}

makemsg(fname)
	char *fname;
{
	register int ch, cnt;
	struct tm *lt;
	struct passwd *pw, *getpwuid();
	struct stat sbuf;
	time_t now, time();
	FILE *fp;
	int fd;
	char *p, *whom, hostname[MAXHOSTNAMELEN], lbuf[100], tmpname[15];
	char *getlogin(), *malloc(), *strcpy(), *ttyname();

	(void)strcpy(tmpname, _PATH_TMP);
	(void)strcat(tmpname, "/wall.XXXXXX");
	if (!(fd = mkstemp(tmpname)) || !(fp = fdopen(fd, "r+"))) {
		(void)fprintf(stderr, "wall: can't open temporary file.\n");
		exit(1);
	}
	(void)unlink(tmpname);

	if (!nobanner) {
		if (!(whom = getlogin()))
			whom = (pw = getpwuid(getuid())) ? pw->pw_name : "???";
		(void)gethostname(hostname, sizeof(hostname));
		(void)time(&now);
		lt = localtime(&now);

		/*
		 * all this stuff is to blank out a square for the message;
		 * we wrap message lines at column 79, not 80, because some
		 * terminals wrap after 79, some do not, and we can't tell.
		 * Which means that we may leave a non-blank character
		 * in column 80, but that can't be helped.
		 */
		(void)fprintf(fp, "\r%79s\r\n", " ");
		(void)sprintf(lbuf, "Broadcast Message from %s@%s",
		    whom, hostname);
		(void)fprintf(fp, "%-79.79s\007\007\r\n", lbuf);
		(void)sprintf(lbuf, "        (%s) at %d:%02d ...", ttyname(2),
		    lt->tm_hour, lt->tm_min);
		(void)fprintf(fp, "%-79.79s\r\n", lbuf);
	}
	(void)fprintf(fp, "%79s\r\n", " ");

	if (*fname && !(freopen(fname, "r", stdin))) {
		(void)fprintf(stderr, "wall: can't read %s.\n", fname);
		exit(1);
	}
	while (fgets(lbuf, sizeof(lbuf), stdin))
		for (cnt = 0, p = lbuf; ch = *p; ++p, ++cnt) {
			if (cnt == 79 || ch == '\n') {
				for (; cnt < 79; ++cnt)
					putc(' ', fp);
				putc('\r', fp);
				putc('\n', fp);
				cnt = 0;
			} else
				putc(ch, fp);
		}
	(void)fprintf(fp, "%79s\r\n", " ");
	rewind(fp);

	if (fstat(fd, &sbuf)) {
		(void)fprintf(stderr, "wall: can't stat temporary file.\n");
		exit(1);
	}
	mbufsize = sbuf.st_size;
	if (!(mbuf = malloc((u_int)mbufsize))) {
		(void)fprintf(stderr, "wall: out of memory.\n");
		exit(1);
	}
	if (fread(mbuf, sizeof(*mbuf), mbufsize, fp) != mbufsize) {
		(void)fprintf(stderr, "wall: can't read temporary file.\n");
		exit(1);
	}
	(void)close(fd);
}
