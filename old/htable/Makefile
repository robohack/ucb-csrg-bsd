#	@(#)Makefile	5.11 (Berkeley) 6/28/90

PROG=	htable
SRCS=	htable.c parse.c scan.c
MAN8=	htable.0
CFLAGS+=-I. -I${.CURDIR}
CLEANFILES+=parse.c scan.c y.tab.h

.include <bsd.prog.mk>
