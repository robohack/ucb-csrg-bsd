#	@(#)Makefile	5.1 (Berkeley) 3/2/91

PROG=	uux
CFLAGS+=-I${.CURDIR}/../includes
BINMODE=6555
DPADD=	${LIBCOMPAT}
LDADD=	${LIBUU} -lcompat

.include <bsd.prog.mk>
