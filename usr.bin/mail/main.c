/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1980, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)main.c	8.2 (Berkeley) 4/20/95";
#endif /* not lint */

#include "rcv.h"
#include <fcntl.h>
#include "extern.h"

/*
 * Mail -- a mail program
 *
 * Startup -- interface with user.
 */

jmp_buf	hdrjmp;

int
main(argc, argv)
	int argc;
	char *argv[];
{
	register int i;
	struct name *to, *cc, *bcc, *smopts;
	char *subject;
	char *ef;
	char nosrc = 0;
	void hdrstop();
	sig_t prevint;
	void sigchild();
	char *rc;

	/*
	 * Set up a reasonable environment.
	 * Figure out whether we are being run interactively,
	 * start the SIGCHLD catcher, and so forth.
	 */
	(void) signal(SIGCHLD, sigchild);
	if (isatty(0))
		assign("interactive", "");
	image = -1;
	/*
	 * Now, determine how we are being used.
	 * We successively pick off - flags.
	 * If there is anything left, it is the base of the list
	 * of users to mail to.  Argp will be set to point to the
	 * first of these users.
	 */
	ef = NOSTR;
	to = NIL;
	cc = NIL;
	bcc = NIL;
	smopts = NIL;
	subject = NOSTR;
	while ((i = getopt(argc, argv, "INT:b:c:dfins:u:v")) != EOF) {
		switch (i) {
		case 'T':
			/*
			 * Next argument is temp file to write which
			 * articles have been read/deleted for netnews.
			 */
			Tflag = optarg;
			if ((i = creat(Tflag, 0600)) < 0) {
				perror(Tflag);
				exit(1);
			}
			close(i);
			break;
		case 'u':
			/*
			 * Next argument is person to pretend to be.
			 */
			myname = optarg;
			break;
		case 'i':
			/*
			 * User wants to ignore interrupts.
			 * Set the variable "ignore"
			 */
			assign("ignore", "");
			break;
		case 'd':
			debug++;
			break;
		case 's':
			/*
			 * Give a subject field for sending from
			 * non terminal
			 */
			subject = optarg;
			break;
		case 'f':
			/*
			 * User is specifying file to "edit" with Mail,
			 * as opposed to reading system mailbox.
			 * If no argument is given after -f, we read his
			 * mbox file.
			 *
			 * getopt() can't handle optional arguments, so here
			 * is an ugly hack to get around it.
			 */
			if ((argv[optind]) && (argv[optind][0] != '-'))
				ef = argv[optind++];
			else
				ef = "&";
			break;
		case 'n':
			/*
			 * User doesn't want to source /usr/lib/Mail.rc
			 */
			nosrc++;
			break;
		case 'N':
			/*
			 * Avoid initial header printing.
			 */
			assign("noheader", "");
			break;
		case 'v':
			/*
			 * Send mailer verbose flag
			 */
			assign("verbose", "");
			break;
		case 'I':
			/*
			 * We're interactive
			 */
			assign("interactive", "");
			break;
		case 'c':
			/*
			 * Get Carbon Copy Recipient list
			 */
			cc = cat(cc, nalloc(optarg, GCC));
			break;
		case 'b':
			/*
			 * Get Blind Carbon Copy Recipient list
			 */
			bcc = cat(bcc, nalloc(optarg, GBCC));
			break;
		case '?':
			fputs("\
Usage: mail [-iInv] [-s subject] [-c cc-addr] [-b bcc-addr] to-addr ...\n\
            [- sendmail-options ...]\n\
       mail [-iInNv] -f [name]\n\
       mail [-iInNv] [-u user]\n",
				stderr);
			exit(1);
		}
	}
	for (i = optind; (argv[i]) && (*argv[i] != '-'); i++)
		to = cat(to, nalloc(argv[i], GTO));
	for (; argv[i]; i++)
		smopts = cat(smopts, nalloc(argv[i], 0));
	/*
	 * Check for inconsistent arguments.
	 */
	if (to == NIL && (subject != NOSTR || cc != NIL || bcc != NIL)) {
		fputs("You must specify direct recipients with -s, -c, or -b.\n", stderr);
		exit(1);
	}
	if (ef != NOSTR && to != NIL) {
		fprintf(stderr, "Cannot give -f and people to send to.\n");
		exit(1);
	}
	tinit();
	setscreensize();
	input = stdin;
	rcvmode = !to;
	spreserve();
	if (!nosrc)
		load(_PATH_MASTER_RC);
	/*
	 * Expand returns a savestr, but load only uses the file name
	 * for fopen, so it's safe to do this.
	 */
	if ((rc = getenv("MAILRC")) == 0)
		rc = "~/.mailrc";
	load(expand(rc));
	if (!rcvmode) {
		mail(to, cc, bcc, smopts, subject);
		/*
		 * why wait?
		 */
		exit(senderr);
	}
	/*
	 * Ok, we are reading mail.
	 * Decide whether we are editing a mailbox or reading
	 * the system mailbox, and open up the right stuff.
	 */
	if (ef == NOSTR)
		ef = "%";
	if (setfile(ef) < 0)
		exit(1);		/* error already reported */
	if (setjmp(hdrjmp) == 0) {
		extern char *version;

		if ((prevint = signal(SIGINT, SIG_IGN)) != SIG_IGN)
			signal(SIGINT, hdrstop);
		if (value("quiet") == NOSTR)
			printf("Mail version %s.  Type ? for help.\n",
				version);
		announce();
		fflush(stdout);
		signal(SIGINT, prevint);
	}
	commands();
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	quit();
	exit(0);
}

/*
 * Interrupt printing of the headers.
 */
void
hdrstop(signo)
	int signo;
{

	fflush(stdout);
	fprintf(stderr, "\nInterrupt\n");
	longjmp(hdrjmp, 1);
}

/*
 * Compute what the screen size for printing headers should be.
 * We use the following algorithm for the height:
 *	If baud rate < 1200, use  9
 *	If baud rate = 1200, use 14
 *	If baud rate > 1200, use 24 or ws_row
 * Width is either 80 or ws_col;
 */
void
setscreensize()
{
	struct termios tbuf;
	struct winsize ws;
	int ospeed;

	if (ioctl(1, TIOCGWINSZ, (char *) &ws) < 0)
		ws.ws_col = ws.ws_row = 0;
	if (ioctl(1, TIOCGETA, &tbuf) < 0)
		ospeed = B9600;
	else
		ospeed = cfgetospeed(&tbuf);
	if (ospeed < B1200)
		screenheight = 9;
	else if (ospeed == B1200)
		screenheight = 14;
	else if (ws.ws_row != 0)
		screenheight = ws.ws_row;
	else
		screenheight = 24;
	if ((realscreenheight = ws.ws_row) == 0)
		realscreenheight = 24;
	if ((screenwidth = ws.ws_col) == 0)
		screenwidth = 80;
}
