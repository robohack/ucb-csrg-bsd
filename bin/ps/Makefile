#	@(#)Makefile	5.8 (Berkeley) 4/3/92

PROG=	ps
SRCS=	devname.c fmt.c keyword.c nlist.c print.c ps.c
CFLAGS+=-I/sys
DPADD=	${LIBMATH} ${LIBUTIL}
LDADD=	-lm -lutil
BINGRP=	kmem
BINMODE=2555

.include <bsd.prog.mk>
