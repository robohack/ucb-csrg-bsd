#	@(#)Makefile	5.4 (Berkeley) 6/1/93

DIR=	smm/16.sendmail
SRCS=	intro.me
MACROS=	-me

all: intro.${PRINTER}

intro.${PRINTER}: ${SRCS}
	${PIC} ${SRCS} | ${ROFF} > ${.TARGET}

.include <bsd.doc.mk>
