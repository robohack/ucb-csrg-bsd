/*
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

# include "sendmail.h"

#ifndef lint
#ifdef SMTP
static char sccsid[] = "@(#)srvrsmtp.c	6.37 (Berkeley) 4/1/93 (with SMTP)";
#else
static char sccsid[] = "@(#)srvrsmtp.c	6.37 (Berkeley) 4/1/93 (without SMTP)";
#endif
#endif /* not lint */

# include <errno.h>
# include <signal.h>

# ifdef SMTP

/*
**  SMTP -- run the SMTP protocol.
**
**	Parameters:
**		none.
**
**	Returns:
**		never.
**
**	Side Effects:
**		Reads commands from the input channel and processes
**			them.
*/

struct cmd
{
	char	*cmdname;	/* command name */
	int	cmdcode;	/* internal code, see below */
};

/* values for cmdcode */
# define CMDERROR	0	/* bad command */
# define CMDMAIL	1	/* mail -- designate sender */
# define CMDRCPT	2	/* rcpt -- designate recipient */
# define CMDDATA	3	/* data -- send message text */
# define CMDRSET	4	/* rset -- reset state */
# define CMDVRFY	5	/* vrfy -- verify address */
# define CMDEXPN	6	/* expn -- expand address */
# define CMDNOOP	7	/* noop -- do nothing */
# define CMDQUIT	8	/* quit -- close connection and die */
# define CMDHELO	9	/* helo -- be polite */
# define CMDHELP	10	/* help -- give usage info */
# define CMDEHLO	11	/* ehlo -- extended helo (RFC 1425) */
/* non-standard commands */
# define CMDONEX	16	/* onex -- sending one transaction only */
# define CMDVERB	17	/* verb -- go into verbose mode */
/* debugging-only commands, only enabled if SMTPDEBUG is defined */
# define CMDDBGQSHOW	24	/* showq -- show send queue */
# define CMDDBGDEBUG	25	/* debug -- set debug mode */

static struct cmd	CmdTab[] =
{
	"mail",		CMDMAIL,
	"rcpt",		CMDRCPT,
	"data",		CMDDATA,
	"rset",		CMDRSET,
	"vrfy",		CMDVRFY,
	"expn",		CMDEXPN,
	"help",		CMDHELP,
	"noop",		CMDNOOP,
	"quit",		CMDQUIT,
	"helo",		CMDHELO,
	"ehlo",		CMDEHLO,
	"verb",		CMDVERB,
	"onex",		CMDONEX,
	/*
	 * remaining commands are here only
	 * to trap and log attempts to use them
	 */
	"showq",	CMDDBGQSHOW,
	"debug",	CMDDBGDEBUG,
	NULL,		CMDERROR,
};

bool	InChild = FALSE;		/* true if running in a subprocess */
bool	OneXact = FALSE;		/* one xaction only this run */

#define EX_QUIT		22		/* special code for QUIT command */

smtp(e)
	register ENVELOPE *e;
{
	register char *p;
	register struct cmd *c;
	char *cmd;
	static char *skipword();
	auto ADDRESS *vrfyqueue;
	ADDRESS *a;
	char *sendinghost;
	bool gotmail;			/* mail command received */
	bool gothello;			/* helo command received */
	bool vrfy;			/* set if this is a vrfy command */
	char *protocol;			/* sending protocol */
	long msize;			/* approximate maximum message size */
	auto char *delimptr;
	char *id;
	char inp[MAXLINE];
	char cmdbuf[MAXLINE];
	char hostbuf[MAXNAME];
	extern char Version[];
	extern char *macvalue();
	extern ADDRESS *recipient();
	extern ENVELOPE BlankEnvelope;
	extern ENVELOPE *newenvelope();
	extern char *anynet_ntoa();

	if (OutChannel != stdout)
	{
		/* arrange for debugging output to go to remote host */
		(void) close(1);
		(void) dup(fileno(OutChannel));
	}
	settime(e);
	CurHostName = RealHostName;
	setproctitle("srvrsmtp %s", CurHostName);
	expand("\201e", inp, &inp[sizeof inp], e);
	message("220 %s", inp);
	SmtpPhase = "startup";
	sendinghost = NULL;
	protocol = NULL;
	gothello = FALSE;
	gotmail = FALSE;
	for (;;)
	{
		/* arrange for backout */
		if (setjmp(TopFrame) > 0 && InChild)
			finis();
		QuickAbort = FALSE;
		HoldErrs = FALSE;
		LogUsrErrs = FALSE;
		e->e_flags &= ~EF_VRFYONLY;

		/* setup for the read */
		e->e_to = NULL;
		Errors = 0;
		(void) fflush(stdout);

		/* read the input line */
		p = sfgets(inp, sizeof inp, InChannel, TimeOuts.to_nextcommand);

		/* handle errors */
		if (p == NULL)
		{
			/* end of file, just die */
			message("421 %s Lost input channel from %s",
				MyHostName, CurHostName);
#ifdef LOG
			if (LogLevel > 1)
				syslog(LOG_NOTICE, "lost input channel from %s",
					CurHostName);
#endif
			if (InChild)
				ExitStat = EX_QUIT;
			finis();
		}

		/* clean up end of line */
		fixcrlf(inp, TRUE);

		/* echo command to transcript */
		if (e->e_xfp != NULL)
			fprintf(e->e_xfp, "<<< %s\n", inp);

		/* break off command */
		for (p = inp; isascii(*p) && isspace(*p); p++)
			continue;
		cmd = cmdbuf;
		while (*p != '\0' &&
		       !(isascii(*p) && isspace(*p)) &&
		       cmd < &cmdbuf[sizeof cmdbuf - 2])
			*cmd++ = *p++;
		*cmd = '\0';

		/* throw away leading whitespace */
		while (isascii(*p) && isspace(*p))
			p++;

		/* decode command */
		for (c = CmdTab; c->cmdname != NULL; c++)
		{
			if (!strcasecmp(c->cmdname, cmdbuf))
				break;
		}

		/* reset errors */
		errno = 0;

		/* process command */
		switch (c->cmdcode)
		{
		  case CMDHELO:		/* hello -- introduce yourself */
		  case CMDEHLO:		/* extended hello */
			if (c->cmdcode == CMDEHLO)
			{
				protocol = "ESMTP";
				SmtpPhase = "EHLO";
			}
			else
			{
				protocol = "SMTP";
				SmtpPhase = "HELO";
			}
			setproctitle("%s: %s", CurHostName, inp);
			if (strcasecmp(p, MyHostName) == 0)
			{
				/*
				**  Didn't know about alias or MX,
				**  or connected to an echo server
				*/

				message("553 %s config error: mail loops back to myself",
					MyHostName);
				break;
			}
			(void) strcpy(hostbuf, p);
			(void) strcat(hostbuf, " (");
			(void) strcat(hostbuf, anynet_ntoa(&RealHostAddr));
			if (strcasecmp(p, RealHostName) != 0)
			{
				auth_warning(e, "Host %s claimed to be %s",
					RealHostName, p);
				(void) strcat(hostbuf, "; ");
				(void) strcat(hostbuf, RealHostName);
			}
			(void) strcat(hostbuf, ")");
			sendinghost = newstr(hostbuf);

			/* send ext. message -- old systems must ignore */
			message("250-%s Hello %s, pleased to meet you",
				MyHostName, sendinghost);
			if (!bitset(PRIV_NOEXPN, PrivacyFlags))
				message("250-EXPN");
			message("250-SIZE");
			message("250 HELP");
			gothello = TRUE;
			break;

		  case CMDMAIL:		/* mail -- designate sender */
			SmtpPhase = "MAIL";

			/* force a sending host even if no HELO given */
			if (sendinghost == NULL && macvalue('s', e) == NULL)
				sendinghost = RealHostName;

			/* check for validity of this command */
			if (!gothello)
			{
				if (bitset(PRIV_NEEDMAILHELO, PrivacyFlags))
				{
					message("503 Polite people say HELO first");
					break;
				}
				else
				{
					auth_warning(e,
						"Host %s didn't use HELO protocol",
						RealHostName);
				}
			}
			if (gotmail)
			{
				message("503 Sender already specified");
				break;
			}
			if (InChild)
			{
				errno = 0;
				syserr("503 Nested MAIL command: MAIL %s", p);
				finis();
			}

			/* fork a subprocess to process this command */
			if (runinchild("SMTP-MAIL", e) > 0)
				break;
			if (sendinghost != NULL)
				define('s', sendinghost, e);
			if (protocol == NULL)
				protocol = "SMTP";
			define('r', protocol, e);
			initsys(e);
			setproctitle("%s %s: %s", e->e_id, CurHostName, inp);

			/* child -- go do the processing */
			p = skipword(p, "from");
			if (p == NULL)
				break;
			if (setjmp(TopFrame) > 0)
			{
				/* this failed -- undo work */
				if (InChild)
					finis();
				break;
			}
			QuickAbort = TRUE;

			/* must parse sender first */
			delimptr = NULL;
			setsender(p, e, &delimptr, FALSE);
			p = delimptr;
			if (p != NULL && *p != '\0')
				*p++ = '\0';

			/* now parse ESMTP arguments */
			msize = 0;
			for (; p != NULL && *p != '\0'; p++)
			{
				char *kp;
				char *vp;

				/* locate the beginning of the keyword */
				while (isascii(*p) && isspace(*p))
					p++;
				if (*p == '\0')
					break;
				kp = p;

				/* skip to the value portion */
				while (isascii(*p) && isalnum(*p) || *p == '-')
					p++;
				if (*p == '=')
				{
					*p++ = '\0';
					vp = p;

					/* skip to the end of the value */
					while (*p != '\0' && *p != ' ' &&
					       !(isascii(*p) && iscntrl(*p)) &&
					       *p != '=')
						p++;
				}

				if (*p != '\0')
					*p++ = '\0';

				if (tTd(19, 1))
					printf("MAIL: got arg %s=%s\n", kp,
						vp == NULL ? "<null>" : vp);

				if (strcasecmp(kp, "size") == 0)
				{
					if (kp == NULL)
					{
						usrerr("501 SIZE requires a value");
						/* NOTREACHED */
					}
					msize = atol(vp);
				}
				else
				{
					usrerr("501 %s parameter unrecognized", kp);
					/* NOTREACHED */
				}
			}
				
			if (!enoughspace(msize))
			{
				message("452 Insufficient disk space; try again later");
				break;
			}
			message("250 Sender ok");
			gotmail = TRUE;
			break;

		  case CMDRCPT:		/* rcpt -- designate recipient */
			if (!gotmail)
			{
				usrerr("503 Need MAIL before RCPT");
				break;
			}
			SmtpPhase = "RCPT";
			setproctitle("%s %s: %s", e->e_id, CurHostName, inp);
			if (setjmp(TopFrame) > 0)
			{
				e->e_flags &= ~EF_FATALERRS;
				break;
			}
			QuickAbort = TRUE;
			LogUsrErrs = TRUE;

			e->e_flags |= EF_VRFYONLY;

			p = skipword(p, "to");
			if (p == NULL)
				break;
			a = parseaddr(p, (ADDRESS *) NULL, 1, ' ', NULL, e);
			if (a == NULL)
				break;
			a->q_flags |= QPRIMARY;
			a = recipient(a, &e->e_sendqueue, e);
			if (Errors != 0)
				break;

			/* no errors during parsing, but might be a duplicate */
			e->e_to = p;
			if (!bitset(QBADADDR, a->q_flags))
				message("250 Recipient ok");
			else
			{
				/* punt -- should keep message in ADDRESS.... */
				message("550 Addressee unknown");
			}
			e->e_to = NULL;
			break;

		  case CMDDATA:		/* data -- text of mail */
			SmtpPhase = "DATA";
			if (!gotmail)
			{
				message("503 Need MAIL command");
				break;
			}
			else if (e->e_nrcpts <= 0)
			{
				message("503 Need RCPT (recipient)");
				break;
			}

			/* collect the text of the message */
			SmtpPhase = "collect";
			setproctitle("%s %s: %s", e->e_id, CurHostName, inp);
			collect(TRUE, e);
			if (Errors != 0)
				break;

			/*
			**  Arrange to send to everyone.
			**	If sending to multiple people, mail back
			**		errors rather than reporting directly.
			**	In any case, don't mail back errors for
			**		anything that has happened up to
			**		now (the other end will do this).
			**	Truncate our transcript -- the mail has gotten
			**		to us successfully, and if we have
			**		to mail this back, it will be easier
			**		on the reader.
			**	Then send to everyone.
			**	Finally give a reply code.  If an error has
			**		already been given, don't mail a
			**		message back.
			**	We goose error returns by clearing error bit.
			*/

			SmtpPhase = "delivery";
			if (e->e_nrcpts != 1)
			{
				HoldErrs = TRUE;
				e->e_errormode = EM_MAIL;
			}
			e->e_flags &= ~EF_FATALERRS;
			e->e_xfp = freopen(queuename(e, 'x'), "w", e->e_xfp);
			id = e->e_id;

			/* check to see if we need to re-expand aliases */
			for (a = e->e_sendqueue; a != NULL; a = a->q_next)
			{
				if (bitset(QVERIFIED, a->q_flags))
					break;
			}

			/* send to all recipients */
			sendall(e, a == NULL ? SM_DEFAULT : SM_QUEUE);
			e->e_to = NULL;

			/* save statistics */
			markstats(e, (ADDRESS *) NULL);

			/* issue success if appropriate and reset */
			if (Errors == 0 || HoldErrs)
				message("250 %s Message accepted for delivery", id);
			else
				e->e_flags &= ~EF_FATALERRS;

			/* if we just queued, poke it */
			if (a != NULL && e->e_sendmode != SM_QUEUE)
			{
				unlockqueue(e);
				dowork(id, TRUE, TRUE, e);
				e->e_id = NULL;
			}

			/* if in a child, pop back to our parent */
			if (InChild)
				finis();

			/* clean up a bit */
			gotmail = FALSE;
			dropenvelope(e);
			CurEnv = e = newenvelope(e, CurEnv);
			e->e_flags = BlankEnvelope.e_flags;
			break;

		  case CMDRSET:		/* rset -- reset state */
			message("250 Reset state");
			if (InChild)
				finis();

			/* clean up a bit */
			gotmail = FALSE;
			dropenvelope(e);
			CurEnv = e = newenvelope(e, CurEnv);
			break;

		  case CMDVRFY:		/* vrfy -- verify address */
		  case CMDEXPN:		/* expn -- expand address */
			vrfy = c->cmdcode == CMDVRFY;
			if (bitset(vrfy ? PRIV_NOVRFY : PRIV_NOEXPN,
						PrivacyFlags))
			{
				if (vrfy)
					message("252 Who's to say?");
				else
					message("502 That's none of your business");
				break;
			}
			else if (!gothello &&
				 bitset(vrfy ? PRIV_NEEDVRFYHELO : PRIV_NEEDEXPNHELO,
						PrivacyFlags))
			{
				message("503 I demand that you introduce yourself first");
				break;
			}
			if (runinchild(vrfy ? "SMTP-VRFY" : "SMTP-EXPN", e) > 0)
				break;
			setproctitle("%s: %s", CurHostName, inp);
#ifdef LOG
			if (LogLevel > 5)
				syslog(LOG_INFO, "%s: %s", CurHostName, inp);
#endif
			vrfyqueue = NULL;
			QuickAbort = TRUE;
			if (vrfy)
				e->e_flags |= EF_VRFYONLY;
			(void) sendtolist(p, (ADDRESS *) NULL, &vrfyqueue, e);
			if (Errors != 0)
			{
				if (InChild)
					finis();
				break;
			}
			while (vrfyqueue != NULL)
			{
				register ADDRESS *a = vrfyqueue->q_next;
				char *code;

				while (a != NULL && bitset(QDONTSEND|QBADADDR, a->q_flags))
					a = a->q_next;

				if (!bitset(QDONTSEND|QBADADDR, vrfyqueue->q_flags))
					printvrfyaddr(vrfyqueue, a == NULL);
				else if (a == NULL)
					message("554 Self destructive alias loop");
				vrfyqueue = a;
			}
			if (InChild)
				finis();
			break;

		  case CMDHELP:		/* help -- give user info */
			help(p);
			break;

		  case CMDNOOP:		/* noop -- do nothing */
			message("200 OK");
			break;

		  case CMDQUIT:		/* quit -- leave mail */
			message("221 %s closing connection", MyHostName);
			if (InChild)
				ExitStat = EX_QUIT;
			finis();

		  case CMDVERB:		/* set verbose mode */
			Verbose = TRUE;
			e->e_sendmode = SM_DELIVER;
			message("200 Verbose mode");
			break;

		  case CMDONEX:		/* doing one transaction only */
			OneXact = TRUE;
			message("200 Only one transaction");
			break;

# ifdef SMTPDEBUG
		  case CMDDBGQSHOW:	/* show queues */
			printf("Send Queue=");
			printaddr(e->e_sendqueue, TRUE);
			break;

		  case CMDDBGDEBUG:	/* set debug mode */
			tTsetup(tTdvect, sizeof tTdvect, "0-99.1");
			tTflag(p);
			message("200 Debug set");
			break;

# else /* not SMTPDEBUG */

		  case CMDDBGQSHOW:	/* show queues */
		  case CMDDBGDEBUG:	/* set debug mode */
# ifdef LOG
			if (LogLevel > 0)
				syslog(LOG_NOTICE,
				    "\"%s\" command from %s (%s)",
				    c->cmdname, RealHostName,
				    anynet_ntoa(&RealHostAddr));
# endif
			/* FALL THROUGH */
# endif /* SMTPDEBUG */

		  case CMDERROR:	/* unknown command */
			message("500 Command unrecognized");
			break;

		  default:
			errno = 0;
			syserr("500 smtp: unknown code %d", c->cmdcode);
			break;
		}
	}
}
/*
**  SKIPWORD -- skip a fixed word.
**
**	Parameters:
**		p -- place to start looking.
**		w -- word to skip.
**
**	Returns:
**		p following w.
**		NULL on error.
**
**	Side Effects:
**		clobbers the p data area.
*/

static char *
skipword(p, w)
	register char *p;
	char *w;
{
	register char *q;

	/* find beginning of word */
	while (isascii(*p) && isspace(*p))
		p++;
	q = p;

	/* find end of word */
	while (*p != '\0' && *p != ':' && !(isascii(*p) && isspace(*p)))
		p++;
	while (isascii(*p) && isspace(*p))
		*p++ = '\0';
	if (*p != ':')
	{
	  syntax:
		message("501 Syntax error");
		Errors++;
		return (NULL);
	}
	*p++ = '\0';
	while (isascii(*p) && isspace(*p))
		p++;

	/* see if the input word matches desired word */
	if (strcasecmp(q, w))
		goto syntax;

	return (p);
}
/*
**  PRINTVRFYADDR -- print an entry in the verify queue
**
**	Parameters:
**		a -- the address to print
**		last -- set if this is the last one.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Prints the appropriate 250 codes.
*/

printvrfyaddr(a, last)
	register ADDRESS *a;
	bool last;
{
	char fmtbuf[20];

	strcpy(fmtbuf, "250");
	fmtbuf[3] = last ? ' ' : '-';

	if (strchr(a->q_paddr, '<') != NULL)
		strcpy(&fmtbuf[4], "%s");
	else if (a->q_fullname == NULL)
		strcpy(&fmtbuf[4], "<%s>");
	else
	{
		strcpy(&fmtbuf[4], "%s <%s>");
		message(fmtbuf, a->q_fullname, a->q_paddr);
		return;
	}
	message(fmtbuf, a->q_paddr);
}
/*
**  HELP -- implement the HELP command.
**
**	Parameters:
**		topic -- the topic we want help for.
**
**	Returns:
**		none.
**
**	Side Effects:
**		outputs the help file to message output.
*/

help(topic)
	char *topic;
{
	register FILE *hf;
	int len;
	char buf[MAXLINE];
	bool noinfo;

	if (HelpFile == NULL || (hf = fopen(HelpFile, "r")) == NULL)
	{
		/* no help */
		errno = 0;
		message("502 HELP not implemented");
		return;
	}

	if (topic == NULL || *topic == '\0')
		topic = "smtp";
	else
		makelower(topic);

	len = strlen(topic);
	noinfo = TRUE;

	while (fgets(buf, sizeof buf, hf) != NULL)
	{
		if (strncmp(buf, topic, len) == 0)
		{
			register char *p;

			p = strchr(buf, '\t');
			if (p == NULL)
				p = buf;
			else
				p++;
			fixcrlf(p, TRUE);
			message("214-%s", p);
			noinfo = FALSE;
		}
	}

	if (noinfo)
		message("504 HELP topic unknown");
	else
		message("214 End of HELP info");
	(void) fclose(hf);
}
/*
**  RUNINCHILD -- return twice -- once in the child, then in the parent again
**
**	Parameters:
**		label -- a string used in error messages
**
**	Returns:
**		zero in the child
**		one in the parent
**
**	Side Effects:
**		none.
*/

runinchild(label, e)
	char *label;
	register ENVELOPE *e;
{
	int childpid;

	if (!OneXact)
	{
		childpid = dofork();
		if (childpid < 0)
		{
			syserr("%s: cannot fork", label);
			return (1);
		}
		if (childpid > 0)
		{
			auto int st;

			/* parent -- wait for child to complete */
			st = waitfor(childpid);
			if (st == -1)
				syserr("%s: lost child", label);

			/* if we exited on a QUIT command, complete the process */
			if (st == (EX_QUIT << 8))
				finis();

			return (1);
		}
		else
		{
			/* child */
			InChild = TRUE;
			QuickAbort = FALSE;
			clearenvelope(e, FALSE);
		}
	}

	/* open alias database */
	initaliases(AliasFile, FALSE, e);

	return (0);
}

# endif /* SMTP */
