#	@(#)Makefile	5.6 (Berkeley) 11/4/91

PROG=	fstat
SRCS=	fstat.c devname.c
.PATH:	${.CURDIR}/../../bin/ps
DPADD=	${LIBUTIL}
LDADD=	-lutil
BINGRP=	kmem
BINMODE=2555

.include <bsd.prog.mk>
