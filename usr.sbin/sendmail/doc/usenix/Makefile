#	@(#)Makefile	5.4 (Berkeley) 6/1/93

SRCS=	usenix.me
MACROS=	-me

all: usenix.${PRINTER}

usenix.${PRINTER}: ${SRCS}
	${PIC} ${SRCS} | ${ROFF} > ${.TARGET}

.include <bsd.doc.mk>
