# include <signal.h>
# include <errno.h>
# include "sendmail.h"
# include <sys/stat.h>
# ifdef LOG
# include <syslog.h>
# endif LOG

static char SccsId[] = "@(#)deliver.c	3.54.1.1	11/21/81";

/*
**  DELIVER -- Deliver a message to a list of addresses.
**
**	This routine delivers to everyone on the same host as the
**	user on the head of the list.  It is clever about mailers
**	that don't handle multiple users.  It is NOT guaranteed
**	that it will deliver to all these addresses however -- so
**	deliver should be called once for each address on the
**	list.
**
**	Parameters:
**		firstto -- head of the address list to deliver to.
**		editfcn -- if non-NULL, we want to call this function
**			to output the letter (instead of just out-
**			putting it raw).
**
**	Returns:
**		zero -- successfully delivered.
**		else -- some failure, see ExitStat for more info.
**
**	Side Effects:
**		The standard input is passed off to someone.
*/

deliver(firstto, editfcn)
	ADDRESS *firstto;
	int (*editfcn)();
{
	char *host;			/* host being sent to */
	char *user;			/* user being sent to */
	char **pvp;
	register char **mvp;
	register char *p;
	register struct mailer *m;	/* mailer for this recipient */
	register int i;
	extern putmessage();
	extern bool checkcompat();
	char *pv[MAXPV+1];
	char tobuf[MAXLINE];		/* text line of to people */
	char buf[MAXNAME];
	ADDRESS *ctladdr;
	extern ADDRESS *getctladdr();
	char tfrombuf[MAXNAME];		/* translated from person */
	extern char **prescan();
	register ADDRESS *to = firstto;
	bool clever = FALSE;		/* running user smtp to this mailer */
	bool tempfail = FALSE;

	errno = 0;
	if (!ForceMail && bitset(QDONTSEND|QPSEUDO, to->q_flags))
		return (0);

# ifdef DEBUG
	if (Debug)
		printf("\n--deliver, mailer=%d, host=`%s', first user=`%s'\n",
			to->q_mailer->m_mno, to->q_host, to->q_user);
# endif DEBUG

	/*
	**  Do initial argv setup.
	**	Insert the mailer name.  Notice that $x expansion is
	**	NOT done on the mailer name.  Then, if the mailer has
	**	a picky -f flag, we insert it as appropriate.  This
	**	code does not check for 'pv' overflow; this places a
	**	manifest lower limit of 4 for MAXPV.
	*/

	m = to->q_mailer;
	host = to->q_host;

	/* rewrite from address, using rewriting rules */
	(void) expand(m->m_from, buf, &buf[sizeof buf - 1]);
	mvp = prescan(buf, '\0');
	if (mvp == NULL)
	{
		syserr("bad mailer from translate \"%s\"", buf);
		return (EX_SOFTWARE);
	}
	rewrite(mvp, 2);
	cataddr(mvp, tfrombuf, sizeof tfrombuf);

	define('g', tfrombuf);		/* translated sender address */
	define('h', host);		/* to host */
	Errors = 0;
	pvp = pv;
	*pvp++ = m->m_argv[0];

	/* insert -f or -r flag as appropriate */
	if (bitset(M_FOPT|M_ROPT, m->m_flags) && FromFlag)
	{
		if (bitset(M_FOPT, m->m_flags))
			*pvp++ = "-f";
		else
			*pvp++ = "-r";
		(void) expand("$g", buf, &buf[sizeof buf - 1]);
		*pvp++ = newstr(buf);
	}

	/*
	**  Append the other fixed parts of the argv.  These run
	**  up to the first entry containing "$u".  There can only
	**  be one of these, and there are only a few more slots
	**  in the pv after it.
	*/

	for (mvp = m->m_argv; (p = *++mvp) != NULL; )
	{
		while ((p = index(p, '$')) != NULL)
			if (*++p == 'u')
				break;
		if (p != NULL)
			break;

		/* this entry is safe -- go ahead and process it */
		(void) expand(*mvp, buf, &buf[sizeof buf - 1]);
		*pvp++ = newstr(buf);
		if (pvp >= &pv[MAXPV - 3])
		{
			syserr("Too many parameters to %s before $u", pv[0]);
			return (-1);
		}
	}

	if (*mvp == NULL)
	{
		/* running SMTP */
		clever = TRUE;
		*pvp = NULL;
		i = smtpinit(m, pv, (ADDRESS *) NULL);
		giveresponse(i, TRUE, m);
		if (i == EX_TEMPFAIL)
		{
			QueueUp = TRUE;
			tempfail = TRUE;
		}
	}

	/*
	**  At this point *mvp points to the argument with $u.  We
	**  run through our address list and append all the addresses
	**  we can.  If we run out of space, do not fret!  We can
	**  always send another copy later.
	*/

	tobuf[0] = '\0';
	To = tobuf;
	ctladdr = NULL;
	for (; to != NULL; to = to->q_next)
	{
		/* avoid sending multiple recipients to dumb mailers */
		if (tobuf[0] != '\0' && !bitset(M_MUSER, m->m_flags))
			break;

		/* if already sent or not for this host, don't send */
		if ((!ForceMail && bitset(QDONTSEND|QPSEUDO, to->q_flags)) ||
		    strcmp(to->q_host, host) != 0)
			continue;

		/* compute effective uid/gid when sending */
		if (to->q_mailer == ProgMailer)
			ctladdr = getctladdr(to);

		user = to->q_user;
		To = to->q_paddr;
		to->q_flags |= QDONTSEND;
		if (tempfail)
			to->q_flags |= QQUEUEUP;
# ifdef DEBUG
		if (Debug)
			printf("   send to `%s'\n", user);
# endif DEBUG

		/*
		**  Check to see that these people are allowed to
		**  talk to each other.
		*/

		if (!checkcompat(to))
		{
			giveresponse(EX_UNAVAILABLE, TRUE, m);
			continue;
		}

		/*
		**  Strip quote bits from names if the mailer is dumb
		**	about them.
		*/

		if (bitset(M_STRIPQ, m->m_flags))
		{
			stripquotes(user, TRUE);
			stripquotes(host, TRUE);
		}
		else
		{
			stripquotes(user, FALSE);
			stripquotes(host, FALSE);
		}

		/*
		**  If an error message has already been given, don't
		**	bother to send to this address.
		**
		**	>>>>>>>>>> This clause assumes that the local mailer
		**	>> NOTE >> cannot do any further aliasing; that
		**	>>>>>>>>>> function is subsumed by sendmail.
		*/

		if (bitset(QBADADDR, to->q_flags))
			continue;

		/* save statistics.... */
		Stat.stat_nt[to->q_mailer->m_mno]++;
		Stat.stat_bt[to->q_mailer->m_mno] += kbytes(MsgSize);

		/*
		**  See if this user name is "special".
		**	If the user name has a slash in it, assume that this
		**	is a file -- send it off without further ado.
		**	Note that this means that editfcn's will not
		**	be applied to the message.  Also note that
		**	this type of addresses is not processed along
		**	with the others, so we fudge on the To person.
		*/

		if (m == LocalMailer)
		{
			if (index(user, '/') != NULL)
			{
				i = mailfile(user, getctladdr(to));
				giveresponse(i, TRUE, m);
				continue;
			}
		}

		/*
		**  Address is verified -- add this user to mailer
		**  argv, and add it to the print list of recipients.
		*/

		/* create list of users for error messages */
		if (tobuf[0] != '\0')
			(void) strcat(tobuf, ",");
		(void) strcat(tobuf, to->q_paddr);
		define('u', user);		/* to user */
		define('z', to->q_home);	/* user's home */

		/*
		**  Expand out this user into argument list or
		**  send it to our SMTP server.
		*/

		if (clever)
		{
			i = smtprcpt(to);
			if (i == EX_TEMPFAIL)
			{
				QueueUp = TRUE;
				to->q_flags |= QQUEUEUP;
			}
			else if (i != EX_OK)
			{
				to->q_flags |= QBADADDR;
				giveresponse(i, TRUE, m);
			}
		}
		else
		{
			(void) expand(*mvp, buf, &buf[sizeof buf - 1]);
			*pvp++ = newstr(buf);
			if (pvp >= &pv[MAXPV - 2])
			{
				/* allow some space for trailing parms */
				break;
			}
		}
	}

	/* see if any addresses still exist */
	if (tobuf[0] == '\0')
	{
		if (clever)
			smtpquit(pv[0]);
		return (0);
	}

	/* print out messages as full list */
	To = tobuf;

	/*
	**  Fill out any parameters after the $u parameter.
	*/

	while (!clever && *++mvp != NULL)
	{
		(void) expand(*mvp, buf, &buf[sizeof buf - 1]);
		*pvp++ = newstr(buf);
		if (pvp >= &pv[MAXPV])
			syserr("deliver: pv overflow after $u for %s", pv[0]);
	}
	*pvp++ = NULL;

	/*
	**  Call the mailer.
	**	The argument vector gets built, pipes
	**	are created as necessary, and we fork & exec as
	**	appropriate.
	**	If we are running SMTP, we just need to clean up.
	*/

	if (editfcn == NULL)
		editfcn = putmessage;
	if (ctladdr == NULL)
		ctladdr = &From;
	if (clever)
	{
		i = smtpfinish(m, editfcn);
		smtpquit(pv[0]);
	}
	else
		i = sendoff(m, pv, editfcn, ctladdr);

	/*
	**  If we got a temporary failure, arrange to queue the
	**  addressees.
	*/

	if (i == EX_TEMPFAIL)
	{
		QueueUp = TRUE;
		for (to = firstto; to != NULL; to = to->q_next)
		{
			if (bitset(QBADADDR, to->q_flags))
				continue;
			to->q_flags |= QQUEUEUP;
		}
	}

	errno = 0;
	return (i);
}
/*
**  DOFORK -- do a fork, retrying a couple of times on failure.
**
**	This MUST be a macro, since after a vfork we are running
**	two processes on the same stack!!!
**
**	Parameters:
**		none.
**
**	Returns:
**		From a macro???  You've got to be kidding!
**
**	Side Effects:
**		Modifies the ==> LOCAL <== variable 'pid', leaving:
**			pid of child in parent, zero in child.
**			-1 on unrecoverable error.
**
**	Notes:
**		I'm awfully sorry this looks so awful.  That's
**		vfork for you.....
*/

# define NFORKTRIES	5
# ifdef VFORK
# define XFORK	vfork
# else VFORK
# define XFORK	fork
# endif VFORK

# define DOFORK(fORKfN) \
{\
	register int i;\
\
	for (i = NFORKTRIES; i-- > 0; )\
	{\
		pid = fORKfN();\
		if (pid >= 0)\
			break;\
		sleep((unsigned) NFORKTRIES - i);\
	}\
}
/*
**  DOFORK -- simple fork interface to DOFORK.
**
**	Parameters:
**		none.
**
**	Returns:
**		pid of child in parent.
**		zero in child.
**		-1 on error.
**
**	Side Effects:
**		returns twice, once in parent and once in child.
*/

dofork()
{
	register int pid;

	DOFORK(fork);
	return (pid);
}
/*
**  SENDOFF -- send off call to mailer & collect response.
**
**	Parameters:
**		m -- mailer descriptor.
**		pvp -- parameter vector to send to it.
**		editfcn -- function to pipe it through.
**		ctladdr -- an address pointer controlling the
**			user/groupid etc. of the mailer.
**
**	Returns:
**		exit status of mailer.
**
**	Side Effects:
**		none.
*/

sendoff(m, pvp, editfcn, ctladdr)
	struct mailer *m;
	char **pvp;
	int (*editfcn)();
	ADDRESS *ctladdr;
{
	auto FILE *mfile;
	auto FILE *rfile;
	register int i;
	extern putmessage();
	int pid;

	/*
	**  Create connection to mailer.
	*/

	pid = openmailer(m, pvp, ctladdr, FALSE, &mfile, &rfile);
	if (pid < 0)
		return (-1);

	/*
	**  Format and send message.
	*/

	(void) signal(SIGPIPE, SIG_IGN);
	if (editfcn == NULL)
		editfcn = putmessage;
	
	(*editfcn)(mfile, m, FALSE);
	(void) fclose(mfile);

	i = endmailer(pid, pvp[0]);
	giveresponse(i, TRUE, m);
	return (i);
}
/*
**  ENDMAILER -- Wait for mailer to terminate.
**
**	We should never get fatal errors (e.g., segmentation
**	violation), so we report those specially.  For other
**	errors, we choose a status message (into statmsg),
**	and if it represents an error, we print it.
**
**	Parameters:
**		pid -- pid of mailer.
**		name -- name of mailer (for error messages).
**
**	Returns:
**		exit code of mailer.
**
**	Side Effects:
**		none.
*/

endmailer(pid, name)
	int pid;
	char *name;
{
	register int i;
	auto int st;

	while ((i = wait(&st)) > 0 && i != pid)
		continue;
	if (i < 0)
	{
		syserr("wait");
		return (-1);
	}
	if ((st & 0377) != 0)
	{
		syserr("%s: stat %o", name, st);
		ExitStat = EX_UNAVAILABLE;
		return (-1);
	}
	i = (st >> 8) & 0377;
	return (i);
}
/*
**  OPENMAILER -- open connection to mailer.
**
**	Parameters:
**		m -- mailer descriptor.
**		pvp -- parameter vector to pass to mailer.
**		ctladdr -- controlling address for user.
**		clever -- create a full duplex connection.
**		pmfile -- pointer to mfile (to mailer) connection.
**		prfile -- pointer to rfile (from mailer) connection.
**
**	Returns:
**		pid of mailer.
**		-1 on error.
**
**	Side Effects:
**		creates a mailer in a subprocess.
*/

openmailer(m, pvp, ctladdr, clever, pmfile, prfile)
	struct mailer *m;
	char **pvp;
	ADDRESS *ctladdr;
	bool clever;
	FILE **pmfile;
	FILE **prfile;
{
	int pid;
	int mpvect[2];
	int rpvect[2];
	FILE *mfile;
	FILE *rfile;
	extern FILE *fdopen();

# ifdef DEBUG
	if (Debug)
	{
		printf("openmailer:\n");
		printav(pvp);
	}
# endif DEBUG
	errno = 0;

	/* create a pipe to shove the mail through */
	if (pipe(mpvect) < 0)
	{
		syserr("pipe (to mailer)");
		return (-1);
	}

	/* if this mailer speaks smtp, create a return pipe */
	if (clever && pipe(rpvect) < 0)
	{
		syserr("pipe (from mailer)");
		(void) close(mpvect[0]);
		(void) close(mpvect[1]);
		return (-1);
	}

	DOFORK(XFORK);
	/* pid is set by DOFORK */
	if (pid < 0)
	{
		syserr("Cannot fork");
		(void) close(mpvect[0]);
		(void) close(mpvect[1]);
		if (clever)
		{
			(void) close(rpvect[0]);
			(void) close(rpvect[1]);
		}
		return (-1);
	}
	else if (pid == 0)
	{
		/* child -- set up input & exec mailer */
		/* make diagnostic output be standard output */
		(void) signal(SIGINT, SIG_IGN);
		(void) signal(SIGHUP, SIG_IGN);
		(void) signal(SIGTERM, SIG_DFL);

		/* arrange to filter standard & diag output of command */
		if (clever)
		{
			(void) close(rpvect[0]);
			(void) close(1);
			(void) dup(rpvect[1]);
			(void) close(rpvect[1]);
		}
		else if (OutChannel != stdout)
		{
			(void) close(1);
			(void) dup(fileno(OutChannel));
		}
		(void) close(2);
		(void) dup(1);

		/* arrange to get standard input */
		(void) close(mpvect[1]);
		(void) close(0);
		if (dup(mpvect[0]) < 0)
		{
			syserr("Cannot dup to zero!");
			_exit(EX_OSERR);
		}
		(void) close(mpvect[0]);
		if (!bitset(M_RESTR, m->m_flags))
		{
			if (ctladdr->q_uid == 0)
			{
				extern int DefUid, DefGid;

				(void) setgid(DefGid);
				(void) setuid(DefUid);
			}
			else
			{
				(void) setgid(ctladdr->q_gid);
				(void) setuid(ctladdr->q_uid);
			}
		}
# ifndef VFORK
		/*
		**  We have to be careful with vfork - we can't mung up the
		**  memory but we don't want the mailer to inherit any extra
		**  open files.  Chances are the mailer won't
		**  care about an extra file, but then again you never know.
		**  Actually, we would like to close(fileno(pwf)), but it's
		**  declared static so we can't.  But if we fclose(pwf), which
		**  is what endpwent does, it closes it in the parent too and
		**  the next getpwnam will be slower.  If you have a weird
		**  mailer that chokes on the extra file you should do the
		**  endpwent().
		**
		**  Similar comments apply to log.  However, openlog is
		**  clever enough to set the FIOCLEX mode on the file,
		**  so it will be closed automatically on the exec.
		*/

		endpwent();
# ifdef LOG
		closelog();
# endif LOG
# endif VFORK
		execv(m->m_mailer, pvp);
		/* syserr fails because log is closed */
		/* syserr("Cannot exec %s", m->m_mailer); */
		printf("Cannot exec '%s' errno=%d\n", m->m_mailer, errno);
		(void) fflush(stdout);
		_exit(EX_UNAVAILABLE);
	}

	/*
	**  Set up return value.
	*/

	(void) close(mpvect[0]);
	mfile = fdopen(mpvect[1], "w");
	if (clever)
	{
		(void) close(rpvect[1]);
		rfile = fdopen(rpvect[0], "r");
	}

	*pmfile = mfile;
	*prfile = rfile;

	return (pid);
}
/*
**  GIVERESPONSE -- Interpret an error response from a mailer
**
**	Parameters:
**		stat -- the status code from the mailer (high byte
**			only; core dumps must have been taken care of
**			already).
**		force -- if set, force an error message output, even
**			if the mailer seems to like to print its own
**			messages.
**		m -- the mailer descriptor for this mailer.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Errors may be incremented.
**		ExitStat may be set.
*/

giveresponse(stat, force, m)
	int stat;
	int force;
	register struct mailer *m;
{
	register char *statmsg;
	extern char *SysExMsg[];
	register int i;
	extern int N_SysEx;
	char buf[30];

	/*
	**  Compute status message from code.
	*/

	i = stat - EX__BASE;
	if (i < 0 || i > N_SysEx)
		statmsg = NULL;
	else
		statmsg = SysExMsg[i];
	if (stat == 0)
	{
		if (bitset(M_LOCAL, m->m_flags))
			statmsg = "delivered";
		else
			statmsg = "queued";
		if (Verbose)
			message(Arpa_Info, statmsg);
	}
	else if (stat == EX_TEMPFAIL)
	{
		if (Verbose)
			message(Arpa_Info, "transmission deferred");
	}
	else
	{
		Errors++;
		if (statmsg == NULL && m->m_badstat != 0)
		{
			stat = m->m_badstat;
			i = stat - EX__BASE;
# ifdef DEBUG
			if (i < 0 || i >= N_SysEx)
				syserr("Bad m_badstat %d", stat);
			else
# endif DEBUG
			statmsg = SysExMsg[i];
		}
		if (statmsg == NULL)
			usrerr("unknown mailer response %d", stat);
		else if (force || !bitset(M_QUIET, m->m_flags) || Verbose)
			usrerr("%s", statmsg);
	}

	/*
	**  Final cleanup.
	**	Log a record of the transaction.  Compute the new
	**	ExitStat -- if we already had an error, stick with
	**	that.
	*/

	if (statmsg == NULL)
	{
		(void) sprintf(buf, "error %d", stat);
		statmsg = buf;
	}

# ifdef LOG
	syslog(LOG_INFO, "%s->%s: %ld: %s", From.q_paddr, To, MsgSize, statmsg);
# endif LOG
	if (stat != EX_TEMPFAIL)
		setstat(stat);
}
/*
**  PUTMESSAGE -- output a message to the final mailer.
**
**	This routine takes care of recreating the header from the
**	in-core copy, etc.
**
**	Parameters:
**		fp -- file to output onto.
**		m -- a mailer descriptor.
**		xdot -- if set, hide lines beginning with dot.
**
**	Returns:
**		none.
**
**	Side Effects:
**		The message is written onto fp.
*/

putmessage(fp, m, xdot)
	FILE *fp;
	struct mailer *m;
	bool xdot;
{
	char buf[BUFSIZ];
	register HDR *h;
	extern char *arpadate();
	bool anyheader = FALSE;
	extern char *capitalize();
	extern char *hvalue();
	extern bool samefrom();
	char *of_line;

	/*
	**  Output "From" line unless supressed
	*/

	if (!bitset(M_NHDR, m->m_flags))
	{
		(void) expand("$l", buf, &buf[sizeof buf - 1]);
		fprintf(fp, "%s\n", buf);
	}

	/*
	**  Output all header lines
	*/

	of_line = hvalue("original-from");
	for (h = Header; h != NULL; h = h->h_link)
	{
		register char *p;
		char *origfrom = OrigFrom;
		bool nooutput;

		nooutput = FALSE;
		if (bitset(H_CHECK|H_ACHECK, h->h_flags) && !bitset(h->h_mflags, m->m_flags))
		{
			p = ")><(";		/* can't happen (I hope) */
			nooutput = TRUE;
		}

		/* use From: line from message if generated is the same */
		if (strcmp(h->h_field, "from") == 0 && origfrom != NULL &&
		    strcmp(m->m_from, "$f") == 0 && of_line == NULL)
		{
			p = origfrom;
			origfrom = NULL;
		}
		else if (bitset(H_DEFAULT, h->h_flags))
		{
			(void) expand(h->h_value, buf, &buf[sizeof buf]);
			p = buf;
		}
		else
			p = h->h_value;
		if (p == NULL || *p == '\0')
			continue;

		/* hack, hack -- output Original-From field if different */
		if (strcmp(h->h_field, "from") == 0 && origfrom != NULL)
		{
			/* output new Original-From line if needed */
			if (of_line == NULL && !samefrom(p, origfrom))
			{
				fprintf(fp, "Original-From: %s\n", origfrom);
				anyheader = TRUE;
			}
			if (of_line != NULL && !nooutput && samefrom(p, of_line))
			{
				/* delete Original-From: line if redundant */
				p = of_line;
				of_line = NULL;
			}
		}
		else if (strcmp(h->h_field, "original-from") == 0 && of_line == NULL)
			nooutput = TRUE;

		/* finally, output the header line */
		if (!nooutput)
		{
			fprintf(fp, "%s: %s\n", capitalize(h->h_field), p);
			h->h_flags |= H_USED;
			anyheader = TRUE;
		}
	}
	if (anyheader)
		fprintf(fp, "\n");

	/*
	**  Output the body of the message
	*/

	if (TempFile != NULL)
	{
		rewind(TempFile);
		while (!ferror(fp) && fgets(buf, sizeof buf, TempFile) != NULL)
			fprintf(fp, "%s%s", xdot && buf[0] == '.' ? "." : "", buf);

		if (ferror(TempFile))
		{
			syserr("putmessage: read error");
			setstat(EX_IOERR);
		}
	}

	(void) fflush(fp);
	if (ferror(fp) && errno != EPIPE)
	{
		syserr("putmessage: write error");
		setstat(EX_IOERR);
	}
	errno = 0;
}
/*
**  SAMEFROM -- tell if two text addresses represent the same from address.
**
**	Parameters:
**		ifrom -- internally generated form of from address.
**		efrom -- external form of from address.
**
**	Returns:
**		TRUE -- if they convey the same info.
**		FALSE -- if any information has been lost.
**
**	Side Effects:
**		none.
*/

bool
samefrom(ifrom, efrom)
	char *ifrom;
	char *efrom;
{
	register char *p;
	char buf[MAXNAME + 4];

# ifdef DEBUG
	if (Debug > 7)
		printf("samefrom(%s,%s)-->", ifrom, efrom);
# endif DEBUG
	if (strcmp(ifrom, efrom) == 0)
		goto success;
	p = index(ifrom, '@');
	if (p == NULL)
		goto failure;
	*p = '\0';
	strcpy(buf, ifrom);
	strcat(buf, " at ");
	*p++ = '@';
	strcat(buf, p);
	if (strcmp(buf, efrom) == 0)
		goto success;

  failure:
# ifdef DEBUG
	if (Debug > 7)
		printf("FALSE\n");
# endif DEBUG
	return (FALSE);

  success:
# ifdef DEBUG
	if (Debug > 7)
		printf("TRUE\n");
# endif DEBUG
	return (TRUE);
}
/*
**  MAILFILE -- Send a message to a file.
**
**	If the file has the setuid/setgid bits set, but NO execute
**	bits, sendmail will try to become the owner of that file
**	rather than the real user.  Obviously, this only works if
**	sendmail runs as root.
**
**	Parameters:
**		filename -- the name of the file to send to.
**		ctladdr -- the controlling address header -- includes
**			the userid/groupid to be when sending.
**
**	Returns:
**		The exit code associated with the operation.
**
**	Side Effects:
**		none.
*/

mailfile(filename, ctladdr)
	char *filename;
	ADDRESS *ctladdr;
{
	register FILE *f;
	register int pid;

	/*
	**  Fork so we can change permissions here.
	**	Note that we MUST use fork, not vfork, because of
	**	the complications of calling subroutines, etc.
	*/

	DOFORK(fork);

	if (pid < 0)
		return (EX_OSERR);
	else if (pid == 0)
	{
		/* child -- actually write to file */
		struct stat stb;
		extern int DefUid, DefGid;

		(void) signal(SIGINT, SIG_DFL);
		(void) signal(SIGHUP, SIG_DFL);
		(void) signal(SIGTERM, SIG_DFL);
		umask(OldUmask);
		if (stat(filename, &stb) < 0)
			stb.st_mode = 0666;
		if (bitset(0111, stb.st_mode))
			exit(EX_CANTCREAT);
		if (ctladdr == NULL)
			ctladdr = &From;
		if (!bitset(S_ISGID, stb.st_mode) || setgid(stb.st_gid) < 0)
		{
			if (ctladdr->q_uid == 0)
				(void) setgid(DefGid);
			else
				(void) setgid(ctladdr->q_gid);
		}
		if (!bitset(S_ISUID, stb.st_mode) || setuid(stb.st_uid) < 0)
		{
			if (ctladdr->q_uid == 0)
				(void) setuid(DefUid);
			else
				(void) setuid(ctladdr->q_uid);
		}
		f = fopen(filename, "a");
		if (f == NULL)
			exit(EX_CANTCREAT);

		putmessage(f, Mailer[1], FALSE);
		fputs("\n", f);
		(void) fclose(f);
		(void) fflush(stdout);

		/* reset ISUID & ISGID bits */
		(void) chmod(filename, (int) stb.st_mode);
		exit(EX_OK);
		/*NOTREACHED*/
	}
	else
	{
		/* parent -- wait for exit status */
		register int i;
		auto int stat;

		while ((i = wait(&stat)) != pid)
		{
			if (i < 0)
			{
				stat = EX_OSERR << 8;
				break;
			}
		}
		if ((stat & 0377) != 0)
			stat = EX_UNAVAILABLE << 8;
		return ((stat >> 8) & 0377);
	}
}
/*
**  SENDALL -- actually send all the messages.
**
**	Parameters:
**		verifyonly -- if set, only give verification messages.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Scans the send lists and sends everything it finds.
*/

sendall(verifyonly)
	bool verifyonly;
{
	register int i;
	typedef int (*fnptr)();

	for (i = 0; Mailer[i] != NULL; i++)
	{
		ADDRESS *q;

		for (q = Mailer[i]->m_sendq; q != NULL; q = q->q_next)
		{
			if (verifyonly)
			{
				To = q->q_paddr;
				if (!bitset(QDONTSEND|QBADADDR|QPSEUDO, q->q_flags))
				{
					if (bitset(M_LOCAL, q->q_mailer->m_flags))
						message(Arpa_Info, "deliverable");
					else
						message(Arpa_Info, "queueable");
				}
			}
			else
				(void) deliver(q, (fnptr) NULL);
		}
	}
}
