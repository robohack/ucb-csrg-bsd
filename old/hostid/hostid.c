/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)hostid.c	5.3 (Berkeley) 11/28/85";
#endif not lint

#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <netdb.h>

extern	char *index();
extern	unsigned long inet_addr();
extern	long gethostid();

main(argc, argv)
	int argc;
	char **argv;
{
	register char *id;
	u_long addr;
	long hostid;
	struct hostent *hp;

	if (argc < 2) {
		printf("%#x\n", gethostid());
		exit(0);
	}
	id = argv[1];

	if ((hostid = inet_addr(id)) == -1) {
		if (*id == '0' && (id[1] == 'x' || id[1] == 'X'))
			id += 2;
		if (sscanf(id, "%x", &hostid) != 1) {
			if (hp = gethostbyname(argv[1])) {
				bcopy(hp->h_addr, &addr, sizeof(addr));
				hostid = addr;
			} else {
				fprintf(stderr,
			"usage: %s [hexnum or internet address]\n", argv[0]);
				exit(1);
			}
		}
	}

	if (sethostid(hostid) < 0) {
		perror("sethostid");
		exit(1);
	}

	exit(0);
}
