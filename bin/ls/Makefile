#	@(#)Makefile	5.4 (Berkeley) 3/1/92

PROG=	ls
SRCS=	cmp.c getbsize.c ls.c print.c util.c
.PATH:	${.CURDIR}/../../bin/df
DPADD=	${LIBUTIL}
LDADD=	-lutil

.include <bsd.prog.mk>
