#	@(#)Makefile	5.1 (Berkeley) 5/3/91

PROG=	umount
SRCS=	umount.c
CFLAGS+=-DLETS_GET_SMALL
DPADD=	${LIBRPC}
LDADD=	-lrpc
NOMAN=	noman
LDFLAGS=-N
.PATH:	${.CURDIR}/../../../../sbin/umount

.include <bsd.prog.mk>
