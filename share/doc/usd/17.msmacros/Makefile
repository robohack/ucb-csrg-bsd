#	@(#)Makefile	5.3 (Berkeley) 8/3/93

DIR=	usd/17.msmacros
SRCS=	ms
MACROS=	-msU

paper.ps: ${SRCS}
	${SOELIM} ${SRCS} | ${PIC} | ${TBL} | ${ROFF} > ${.TARGET}

.include <bsd.doc.mk>
