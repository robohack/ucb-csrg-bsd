#	@(#)Makefile	5.1 (Berkeley) 3/2/91

PROG=	uuxqt
CFLAGS+=-I${.CURDIR}/../includes
BINDIR=	${LIBDIR}
BINMODE=6550
MAN8=	uuxqt.0
DPADD=	${LIBCOMPAT}
LDADD=	${LIBUU} -lcompat

.include <bsd.prog.mk>
