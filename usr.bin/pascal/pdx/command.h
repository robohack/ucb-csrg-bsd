/*-
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)command.h	8.1 (Berkeley) 6/6/93
 */

/*
 * Definitions for the command module.
 *
 * The command module scans and parses the commands.  This includes
 * input file management (i.e. the "source" command), and some error
 * handling.
 */

char *initfile;		/* file to read initial commands from */
BOOLEAN nlflag;		/* for error and signal recovery */
int prompt();		/* print a prompt */
int yyparse();		/* parser generated by yacc */
int lexinit();		/* initialize debugger symbol table */
int gobble();		/* eat input up to a newline for error recovery */
int remake();		/* recompile program, read in new namelist */
int alias();		/* create an alias */
BOOLEAN isstdin();	/* input is from standard input */
