#	@(#)Makefile	5.5 (Berkeley) 3/3/93

PROG=	ls
SRCS=	cmp.c getbsize.c stat_flags.c ls.c print.c util.c
.PATH:	${.CURDIR}/../../bin/df
DPADD=	${LIBUTIL}
LDADD=	-lutil

.include <bsd.prog.mk>
