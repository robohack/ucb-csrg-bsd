/*-
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)treen.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */

    /*
     *	is there some reason why these aren't #defined?
     */

#include	"0.h"
#include	"tree_ty.h"

struct tnode *
tree1 ( arg1 )
    int		arg1;
    {
	return( tree ( 1 , arg1 ));
    }

struct tnode *
tree2 ( arg1 , arg2 )
    int		arg1 , arg2;
    {
	return( tree ( 2 , arg1 , arg2 ));
    }

struct tnode *
tree3 ( arg1 , arg2 , arg3 )
    int		arg1 , arg2 ;
    struct	tnode *arg3;
    {
	return( tree ( 3 , arg1 , arg2 , arg3 ));
    }

struct tnode *
tree4 ( arg1 , arg2 , arg3 , arg4 )
    int		arg1 , arg2 ;
    struct tnode *arg3 , *arg4;
    {
	return( tree ( 4 , arg1 , arg2 , arg3 , arg4 ));
    }

struct tnode *
tree5 ( arg1 , arg2 , arg3 , arg4 , arg5 )
    int		arg1 , arg2 ;
    struct tnode *arg3 , *arg4 , *arg5;
    {
	return (tree ( 5 , arg1 , arg2 , arg3 , arg4 , arg5 ));
    }

