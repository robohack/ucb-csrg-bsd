#	@(#)Makefile	6.3 (Berkeley) 6/5/93

DIR=	psd/06.Clang
SRCS=	Clang.ms
MACROS=	-msU

paper.ps: ${SRCS}
	${TBL} ${SRCS} | ${ROFF} > ${.TARGET}

.include <bsd.doc.mk>
