#ifdef LIBC_SCCS
	.asciz	"@(#)ntohl.s	1.1 (Berkeley/CCI) 7/2/86"
#endif LIBC_SCCS

/* hostorder = ntohl(netorder) */

#include "DEFS.h"

ENTRY(ntohl)
	movl	4(fp),r0
	ret
