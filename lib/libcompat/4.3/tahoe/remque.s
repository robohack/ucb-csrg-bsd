#ifdef LIBC_SCCS
	.asciz	"@(#)remque.s	1.1 (Berkeley/CCI) 8/1/86"
#endif LIBC_SCCS

/* remque(entry) */

#include "DEFS.h"

ENTRY(remque, 0)
	remque	*4(fp)
	ret
