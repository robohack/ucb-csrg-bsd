#	@(#)Makefile	5.5 (Berkeley) 6/5/93

SRCS=	usenix.me
MACROS=	-me

all: usenix.ps

usenix.ps: ${SRCS}
	${PIC} ${SRCS} | ${ROFF} > ${.TARGET}

.include <bsd.doc.mk>
