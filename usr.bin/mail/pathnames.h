/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 *
 *	@(#)pathnames.h	5.5 (Berkeley) 6/16/88
 */

/*
 * Declarations and constants specific to an installation.
 *
 * Vax/Unix version 7.
 */

#define SENDMAIL	"/usr/lib/sendmail"
					/* Name of classy mail deliverer */
#define	EDITOR		"/usr/ucb/ex"	/* Name of text editor */
#define	VISUAL		"/usr/ucb/vi"	/* Name of display editor */
#define	SHELL		"/bin/csh"	/* Standard shell */
#define	MORE		"/usr/ucb/more"	/* Standard output pager */
#define	HELPFILE	"/usr/lib/Mail.help"
					/* Name of casual help file */
#define	THELPFILE	"/usr/lib/Mail.tildehelp"
					/* Name of casual tilde help */
#define	POSTAGE		"/usr/adm/maillog"
					/* Where to audit mail sending */
#define	MASTER		"/usr/lib/Mail.rc"
#define	APPEND				/* New mail goes to end of mailbox */
