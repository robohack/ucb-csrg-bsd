#	@(#)Makefile	8.1 (Berkeley) 6/6/93

PROG=	enroll
CFLAGS+=-I${.CURDIR}/../common_source
SRCS=	enroll.c lib.c
NOMAN=	man
DPADD=	${LIBMP} ${LIBCOMPAT}
LDADD=	-lmp -lcompat
.PATH:	${.CURDIR}/../common_source

.include "../../Makefile.inc"
.include <bsd.prog.mk>
