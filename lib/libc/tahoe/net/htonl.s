#ifdef LIBC_SCCS
	.asciz	"@(#)htonl.s	1.1 (Berkeley/CCI) 7/2/86"
#endif LIBC_SCCS

/* netorder = htonl(hostorder) */

#include "DEFS.h"

ENTRY(htonl)
	movl	4(fp),r0
	ret
