#	@(#)Makefile	5.9 (Berkeley) 6/23/92

PROG=	vmstat
CFLAGS+=-I/sys
MAN8=	vmstat.0
BINGRP=	kmem
BINMODE=2555
DPADD=	names.c ${LIBKVM}
LDADD=	-lkvm

.include <bsd.prog.mk>
