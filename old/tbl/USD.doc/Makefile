#	@(#)Makefile	5.2 (Berkeley) 6/5/93

DIR=	usd/25.tbl
SRCS=	tbl
MACROS=	-msU

paper.ps: ${SRCS}
	${TBL} ${SRCS} | ${ROFF} > ${.TARGET}

.include <bsd.doc.mk>
