#	@(#)Makefile	5.3 (Berkeley) 8/3/93

DIR=	usd/16.awk
SRCS=	awk
MACROS=	-msU

paper.ps: ${SRCS}
	${TBL} ${SRCS} | ${ROFF} > ${.TARGET}

.include <bsd.doc.mk>
