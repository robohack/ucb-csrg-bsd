/*-
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1988, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)make_keypair.c	8.1 (Berkeley) 6/1/93";
#endif /* not lint */

#include <sys/types.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <kerberosIV/des.h>
#include <kerberosIV/krb.h>
#include "pathnames.h"
#include "register_proto.h"

extern void random_key(), herror();
void make_key(), usage();

main(argc, argv)
	int	argc;
	char	**argv;
{
	struct hostent	*hp;
	char		*addr;
	int		i;
	struct sockaddr_in	sin;

	if (argc != 2) {
		usage(argv[0]);
		exit(1);
	}

	if ((hp = gethostbyname(argv[1])) == NULL) {
		herror(argv[1]);
		exit(1);
	}

	for (i = 0; addr = hp->h_addr_list[i]; i++) {
		addr = hp->h_addr_list[i];
		bcopy(addr, &sin.sin_addr, hp->h_length);

		printf("Making key for host %s (%s)\n",
			argv[1], inet_ntoa(sin.sin_addr));
		make_key(sin.sin_addr);
	}
	printf("==========\n");
	printf("One copy of the each key should be put in %s on the\n",
		SERVER_KEYDIR);
	printf("Kerberos server machine (mode 600, owner root).\n");
	printf("Another copy of each key should be put on the named\n");
	printf("client as %sXXX.XXX.XXX.XXX (same modes as above),\n",
		CLIENT_KEYFILE);
	printf("where the X's refer to digits of the host's inet address.\n");
	(void)fflush(stdout);
	exit(0);
}

void
make_key(addr)
	struct in_addr	addr;
{
	struct keyfile_data	kfile;
	char		namebuf[255];
	int		fd;

	(void)sprintf(namebuf, ".%s%s",
		CLIENT_KEYFILE,
		inet_ntoa(addr));
	fd = open(namebuf, O_WRONLY|O_CREAT, 0600);
	if (fd < 0) {
		perror("open");
		exit(1);
	}
	random_key(kfile.kf_key);
	printf("writing to file -> %s ...", namebuf);
	if (write(fd, &kfile, sizeof(kfile)) != sizeof(kfile)) {
		fprintf(stderr, "error writing file %s\n", namebuf);
	}
	printf("done.\n");
	(void)close(fd);
	return;
}

void
usage(name)
	char	*name;
{
	fprintf(stderr, "usage: %s host\n", name);
}
