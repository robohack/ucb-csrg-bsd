#	@(#)Makefile	5.3 (Berkeley) 7/1/91

PROG=	nfsstat
CFLAGS+=-DNFS
DPADD=  ${LIBUTIL}
LDADD= -lutil
BINGRP=	kmem
BINMODE=2555

.include <bsd.prog.mk>
