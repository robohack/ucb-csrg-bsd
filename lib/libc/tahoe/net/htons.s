#ifdef LIBC_SCCS
	.asciz	"@(#)htons.s	1.1 (Berkeley/CCI) 7/2/86"
#endif LIBC_SCCS

/* hostorder = htons(netorder) */

#include "DEFS.h"

ENTRY(htons)
	movzwl	6(fp),r0
	ret
