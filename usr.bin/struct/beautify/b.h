/*-
 * %sccs.include.proprietary.c%
 *
 *	@(#)b.h	8.1 (Berkeley) 6/6/93
 */

extern int xxindent, xxval, newflag, xxmaxchars, xxbpertab;
extern int xxlineno;		/* # of lines already output */
#define xxtop	100		/* max size of xxstack */
extern int xxstind, xxstack[xxtop], xxlablast, xxt;
struct node
	{int op;
	char *lit;
	struct node *left;
	struct node *right;
	};
