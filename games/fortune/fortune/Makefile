#	@(#)Makefile	5.2 (Berkeley) 6/20/92

PROG=	fortune
MAN6=	fortune.0
CFLAGS+=-I${.CURDIR}/../strfile
DPADD=	${COMPAT}
LDADD=	-lcompat

.include "${.CURDIR}/../../Makefile.inc"
.include <bsd.prog.mk>
