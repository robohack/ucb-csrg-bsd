#	@(#)Makefile	5.4 (Berkeley) 4/27/93

PROG=	nfsd
CFLAGS+=-DNFS
MAN8=	nfsd.0
DPADD=	${LIBRPC}
LDADD=	-lrpc

.include <bsd.prog.mk>
