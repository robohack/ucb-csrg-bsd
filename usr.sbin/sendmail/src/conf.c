/*
 * Copyright (c) 1983, 1995 Eric P. Allman
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)conf.c	8.157 (Berkeley) 4/25/95";
#endif /* not lint */

# include "sendmail.h"
# include "pathnames.h"
# include <sys/ioctl.h>
# include <sys/param.h>

/*
**  CONF.C -- Sendmail Configuration Tables.
**
**	Defines the configuration of this installation.
**
**	Configuration Variables:
**		HdrInfo -- a table describing well-known header fields.
**			Each entry has the field name and some flags,
**			which are described in sendmail.h.
**
**	Notes:
**		I have tried to put almost all the reasonable
**		configuration information into the configuration
**		file read at runtime.  My intent is that anything
**		here is a function of the version of UNIX you
**		are running, or is really static -- for example
**		the headers are a superset of widely used
**		protocols.  If you find yourself playing with
**		this file too much, you may be making a mistake!
*/




/*
**  Header info table
**	Final (null) entry contains the flags used for any other field.
**
**	Not all of these are actually handled specially by sendmail
**	at this time.  They are included as placeholders, to let
**	you know that "someday" I intend to have sendmail do
**	something with them.
*/

struct hdrinfo	HdrInfo[] =
{
		/* originator fields, most to least significant  */
	"resent-sender",		H_FROM|H_RESENT,
	"resent-from",			H_FROM|H_RESENT,
	"resent-reply-to",		H_FROM|H_RESENT,
	"sender",			H_FROM,
	"from",				H_FROM,
	"reply-to",			H_FROM,
	"full-name",			H_ACHECK,
	"return-receipt-to",		H_FROM|H_RECEIPTTO,
	"errors-to",			H_FROM|H_ERRORSTO,

		/* destination fields */
	"to",				H_RCPT,
	"resent-to",			H_RCPT|H_RESENT,
	"cc",				H_RCPT,
	"resent-cc",			H_RCPT|H_RESENT,
	"bcc",				H_RCPT|H_STRIPVAL,
	"resent-bcc",			H_RCPT|H_STRIPVAL|H_RESENT,
	"apparently-to",		H_RCPT,

		/* message identification and control */
	"message-id",			0,
	"resent-message-id",		H_RESENT,
	"message",			H_EOH,
	"text",				H_EOH,

		/* date fields */
	"date",				0,
	"resent-date",			H_RESENT,

		/* trace fields */
	"received",			H_TRACE|H_FORCE,
	"x400-received",		H_TRACE|H_FORCE,
	"via",				H_TRACE|H_FORCE,
	"mail-from",			H_TRACE|H_FORCE,

		/* miscellaneous fields */
	"comments",			H_FORCE,
	"return-path",			H_FORCE|H_ACHECK,
	"content-transfer-encoding",	H_CTE,
	"content-type",			H_CTYPE,
	"content-length",		H_ACHECK,

	NULL,			0,
};



/*
**  Location of system files/databases/etc.
*/

char	*PidFile =	_PATH_SENDMAILPID;	/* stores daemon proc id */



/*
**  Privacy values
*/

struct prival PrivacyValues[] =
{
	"public",		PRIV_PUBLIC,
	"needmailhelo",		PRIV_NEEDMAILHELO,
	"needexpnhelo",		PRIV_NEEDEXPNHELO,
	"needvrfyhelo",		PRIV_NEEDVRFYHELO,
	"noexpn",		PRIV_NOEXPN,
	"novrfy",		PRIV_NOVRFY,
	"restrictmailq",	PRIV_RESTRICTMAILQ,
	"restrictqrun",		PRIV_RESTRICTQRUN,
	"authwarnings",		PRIV_AUTHWARNINGS,
	"noreceipts",		PRIV_NORECEIPTS,
	"goaway",		PRIV_GOAWAY,
	NULL,			0,
};



/*
**  Miscellaneous stuff.
*/

int	DtableSize =	50;		/* max open files; reset in 4.2bsd */
/*
**  SETDEFAULTS -- set default values
**
**	Because of the way freezing is done, these must be initialized
**	using direct code.
**
**	Parameters:
**		e -- the default envelope.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Initializes a bunch of global variables to their
**		default values.
*/

#define DAYS		* 24 * 60 * 60

void
setdefaults(e)
	register ENVELOPE *e;
{
	int i;
	extern void inittimeouts();
	extern void setdefuser();
	extern void setupmaps();
	extern void setupmailers();

	SpaceSub = ' ';				/* option B */
	QueueLA = 8;				/* option x */
	RefuseLA = 12;				/* option X */
	WkRecipFact = 30000L;			/* option y */
	WkClassFact = 1800L;			/* option z */
	WkTimeFact = 90000L;			/* option Z */
	QueueFactor = WkRecipFact * 20;		/* option q */
	FileMode = (RealUid != geteuid()) ? 0644 : 0600;
						/* option F */
	DefUid = 1;				/* option u */
	DefGid = 1;				/* option g */
	CheckpointInterval = 10;		/* option C */
	MaxHopCount = 25;			/* option h */
	e->e_sendmode = SM_FORK;		/* option d */
	e->e_errormode = EM_PRINT;		/* option e */
	SevenBitInput = FALSE;			/* option 7 */
	MaxMciCache = 1;			/* option k */
	MciCacheTimeout = 300;			/* option K */
	LogLevel = 9;				/* option L */
	inittimeouts(NULL);			/* option r */
	PrivacyFlags = 0;			/* option p */
	MimeMode = MM_CVTMIME|MM_PASS8BIT;	/* option 8 */
	for (i = 0; i < MAXTOCLASS; i++)
	{
		TimeOuts.to_q_return[i] = 5 DAYS;	/* option T */
		TimeOuts.to_q_warning[i] = 0;		/* option T */
	}
	ServiceSwitchFile = "/etc/service.switch";
	setdefuser();
	setupmaps();
	setupmailers();
}


/*
**  SETDEFUSER -- set/reset DefUser using DefUid (for initgroups())
*/

void
setdefuser()
{
	struct passwd *defpwent;
	static char defuserbuf[40];

	DefUser = defuserbuf;
	if ((defpwent = sm_getpwuid(DefUid)) != NULL)
		strcpy(defuserbuf, defpwent->pw_name);
	else
		strcpy(defuserbuf, "nobody");
}
/*
**  HOST_MAP_INIT -- initialize host class structures
*/

bool	host_map_init __P((MAP *map, char *args));

bool
host_map_init(map, args)
	MAP *map;
	char *args;
{
	register char *p = args;

	for (;;)
	{
		while (isascii(*p) && isspace(*p))
			p++;
		if (*p != '-')
			break;
		switch (*++p)
		{
		  case 'a':
			map->map_app = ++p;
			break;
		}
		while (*p != '\0' && !(isascii(*p) && isspace(*p)))
			p++;
		if (*p != '\0')
			*p++ = '\0';
	}
	if (map->map_app != NULL)
		map->map_app = newstr(map->map_app);
	return TRUE;
}
/*
**  SETUPMAILERS -- initialize default mailers
*/

void
setupmailers()
{
	char buf[100];
	extern void makemailer();

	strcpy(buf, "prog, P=/bin/sh, F=lsoD, T=X-Unix, A=sh -c $u");
	makemailer(buf);

	strcpy(buf, "*file*, P=[FILE], F=lsDFMPEou, T=X-Unix, A=FILE");
	makemailer(buf);

	strcpy(buf, "*include*, P=/dev/null, F=su, A=INCLUDE");
	makemailer(buf);
}
/*
**  SETUPMAPS -- set up map classes
*/

#define MAPDEF(name, ext, flags, parse, open, close, lookup, store) \
	{ \
		extern bool parse __P((MAP *, char *)); \
		extern bool open __P((MAP *, int)); \
		extern void close __P((MAP *)); \
		extern char *lookup __P((MAP *, char *, char **, int *)); \
		extern void store __P((MAP *, char *, char *)); \
		s = stab(name, ST_MAPCLASS, ST_ENTER); \
		s->s_mapclass.map_cname = name; \
		s->s_mapclass.map_ext = ext; \
		s->s_mapclass.map_cflags = flags; \
		s->s_mapclass.map_parse = parse; \
		s->s_mapclass.map_open = open; \
		s->s_mapclass.map_close = close; \
		s->s_mapclass.map_lookup = lookup; \
		s->s_mapclass.map_store = store; \
	}

void
setupmaps()
{
	register STAB *s;

#ifdef NEWDB
	MAPDEF("hash", ".db", MCF_ALIASOK|MCF_REBUILDABLE,
		map_parseargs, hash_map_open, db_map_close,
		db_map_lookup, db_map_store);

	MAPDEF("btree", ".db", MCF_ALIASOK|MCF_REBUILDABLE,
		map_parseargs, bt_map_open, db_map_close,
		db_map_lookup, db_map_store);
#endif

#ifdef NDBM
	MAPDEF("dbm", ".dir", MCF_ALIASOK|MCF_REBUILDABLE,
		map_parseargs, ndbm_map_open, ndbm_map_close,
		ndbm_map_lookup, ndbm_map_store);
#endif

#ifdef NIS
	MAPDEF("nis", NULL, MCF_ALIASOK,
		map_parseargs, nis_map_open, null_map_close,
		nis_map_lookup, null_map_store);
#endif

#ifdef NISPLUS
	MAPDEF("nisplus", NULL, MCF_ALIASOK,
		map_parseargs, nisplus_map_open, null_map_close,
		nisplus_map_lookup, null_map_store);
#endif

#ifdef HESIOD
	MAPDEF("hesiod", NULL, MCF_ALIASOK|MCF_ALIASONLY,
		map_parseargs, null_map_open, null_map_close,
		hes_map_lookup, null_map_store);
#endif

#ifdef NETINFO
	MAPDEF("netinfo", NULL, MCF_ALIASOK,
		map_parseargs, ni_map_open, null_map_close,
		ni_map_lookup, null_map_store);
#endif

#if 0
	MAPDEF("dns", NULL, 0,
		dns_map_init, null_map_open, null_map_close,
		dns_map_lookup, null_map_store);
#endif

#if NAMED_BIND
	/* best MX DNS lookup */
	MAPDEF("bestmx", NULL, MCF_OPTFILE,
		map_parseargs, null_map_open, null_map_close,
		bestmx_map_lookup, null_map_store);
#endif

	MAPDEF("host", NULL, 0,
		host_map_init, null_map_open, null_map_close,
		host_map_lookup, null_map_store);

	MAPDEF("text", NULL, MCF_ALIASOK,
		map_parseargs, text_map_open, null_map_close,
		text_map_lookup, null_map_store);

	MAPDEF("stab", NULL, MCF_ALIASOK|MCF_ALIASONLY,
		map_parseargs, stab_map_open, null_map_close,
		stab_map_lookup, stab_map_store);

	MAPDEF("implicit", NULL, MCF_ALIASOK|MCF_ALIASONLY|MCF_REBUILDABLE,
		map_parseargs, impl_map_open, impl_map_close,
		impl_map_lookup, impl_map_store);

	/* access to system passwd file */
	MAPDEF("user", NULL, MCF_OPTFILE,
		map_parseargs, user_map_open, null_map_close,
		user_map_lookup, null_map_store);

	/* dequote map */
	MAPDEF("dequote", NULL, 0,
		dequote_init, null_map_open, null_map_close,
		dequote_map, null_map_store);

#ifdef USERDB
	/* user database */
	MAPDEF("userdb", ".db", 0,
		map_parseargs, null_map_open, null_map_close,
		udb_map_lookup, null_map_store);
#endif

	/* sequenced maps */
	MAPDEF("sequence", NULL, MCF_ALIASOK,
		seq_map_parse, null_map_open, null_map_close,
		seq_map_lookup, seq_map_store);

	/* switched interface to sequenced maps */
	MAPDEF("switch", NULL, MCF_ALIASOK,
		map_parseargs, switch_map_open, null_map_close,
		seq_map_lookup, seq_map_store);
}

#undef MAPDEF
/*
**  INITHOSTMAPS -- initial host-dependent maps
**
**	This should act as an interface to any local service switch
**	provided by the host operating system.
**
**	Parameters:
**		none
**
**	Returns:
**		none
**
**	Side Effects:
**		Should define maps "host" and "users" as necessary
**		for this OS.  If they are not defined, they will get
**		a default value later.  It should check to make sure
**		they are not defined first, since it's possible that
**		the config file has provided an override.
*/

void
inithostmaps()
{
	register int i;
	int nmaps;
	char *maptype[MAXMAPSTACK];
	short mapreturn[MAXMAPACTIONS];
	char buf[MAXLINE];

	/*
	**  Set up default hosts maps.
	*/

#if 0
	nmaps = switch_map_find("hosts", maptype, mapreturn);
	for (i = 0; i < nmaps; i++)
	{
		if (strcmp(maptype[i], "files") == 0 &&
		    stab("hosts.files", ST_MAP, ST_FIND) == NULL)
		{
			strcpy(buf, "hosts.files text -k 0 -v 1 /etc/hosts");
			makemapentry(buf);
		}
#if NAMED_BIND
		else if (strcmp(maptype[i], "dns") == 0 &&
		    stab("hosts.dns", ST_MAP, ST_FIND) == NULL)
		{
			strcpy(buf, "hosts.dns dns A");
			makemapentry(buf);
		}
#endif
#ifdef NISPLUS
		else if (strcmp(maptype[i], "nisplus") == 0 &&
		    stab("hosts.nisplus", ST_MAP, ST_FIND) == NULL)
		{
			strcpy(buf, "hosts.nisplus nisplus -k name -v address -d hosts.org_dir");
			makemapentry(buf);
		}
#endif
#ifdef NIS
		else if (strcmp(maptype[i], "nis") == 0 &&
		    stab("hosts.nis", ST_MAP, ST_FIND) == NULL)
		{
			strcpy(buf, "hosts.nis nis -d -k 0 -v 1 hosts.byname");
			makemapentry(buf);
		}
#endif
	}
#endif

	/*
	**  Make sure we have a host map.
	*/

	if (stab("host", ST_MAP, ST_FIND) == NULL)
	{
		/* user didn't initialize: set up host map */
		strcpy(buf, "host host");
#if NAMED_BIND
		if (ConfigLevel >= 2)
			strcat(buf, " -a.");
#endif
		makemapentry(buf);
	}

	/*
	**  Set up default aliases maps
	*/

	nmaps = switch_map_find("aliases", maptype, mapreturn);
	for (i = 0; i < nmaps; i++)
	{
		if (strcmp(maptype[i], "files") == 0 &&
		    stab("aliases.files", ST_MAP, ST_FIND) == NULL)
		{
			strcpy(buf, "aliases.files implicit /etc/aliases");
			makemapentry(buf);
		}
#ifdef NISPLUS
		else if (strcmp(maptype[i], "nisplus") == 0 &&
		    stab("aliases.nisplus", ST_MAP, ST_FIND) == NULL)
		{
			strcpy(buf, "aliases.nisplus nisplus -kalias -vexpansion -d mail_aliases.org_dir");
			makemapentry(buf);
		}
#endif
#ifdef NIS
		else if (strcmp(maptype[i], "nis") == 0 &&
		    stab("aliases.nis", ST_MAP, ST_FIND) == NULL)
		{
			strcpy(buf, "aliases.nis nis -d mail.aliases");
			makemapentry(buf);
		}
#endif
	}
	if (stab("aliases", ST_MAP, ST_FIND) == NULL)
	{
		strcpy(buf, "aliases switch aliases");
		makemapentry(buf);
	}
	strcpy(buf, "switch:aliases");
	setalias(buf);

#if 0		/* "user" map class is a better choice */
	/*
	**  Set up default users maps.
	*/

	nmaps = switch_map_find("passwd", maptype, mapreturn);
	for (i = 0; i < nmaps; i++)
	{
		if (strcmp(maptype[i], "files") == 0 &&
		    stab("users.files", ST_MAP, ST_FIND) == NULL)
		{
			strcpy(buf, "users.files text -m -z: -k0 -v6 /etc/passwd");
			makemapentry(buf);
		}
#ifdef NISPLUS
		else if (strcmp(maptype[i], "nisplus") == 0 &&
		    stab("users.nisplus", ST_MAP, ST_FIND) == NULL)
		{
			strcpy(buf, "users.nisplus nisplus -m -kname -vhome -d passwd.org_dir");
			makemapentry(buf);
		}
#endif
#ifdef NIS
		else if (strcmp(maptype[i], "nis") == 0 &&
		    stab("users.nis", ST_MAP, ST_FIND) == NULL)
		{
			strcpy(buf, "users.nis nis -m -d passwd.byname");
			makemapentry(buf);
		}
#endif
#ifdef HESIOD
		else if (strcmp(maptype[i], "hesiod") == 0) &&
		    stab("users.hesiod", ST_MAP, ST_FIND) == NULL)
		{
			strcpy(buf, "users.hesiod hesiod");
			makemapentry(buf);
		}
#endif
	}
	if (stab("users", ST_MAP, ST_FIND) == NULL)
	{
		strcpy(buf, "users switch -m passwd");
		makemapentry(buf);
	}
#endif
}
/*
**  SWITCH_MAP_FIND -- find the list of types associated with a map
**
**	This is the system-dependent interface to the service switch.
**
**	Parameters:
**		service -- the name of the service of interest.
**		maptype -- an out-array of strings containing the types
**			of access to use for this service.  There can
**			be at most MAXMAPSTACK types for a single service.
**		mapreturn -- an out-array of return information bitmaps
**			for the map.
**
**	Returns:
**		The number of map types filled in, or -1 for failure.
*/

#ifdef SOLARIS
# include <nsswitch.h>
#endif

#if defined(ultrix) || defined(__osf__)
# include <sys/svcinfo.h>
#endif

int
switch_map_find(service, maptype, mapreturn)
	char *service;
	char *maptype[MAXMAPSTACK];
	short mapreturn[MAXMAPACTIONS];
{
	register FILE *fp;
	int svcno;
	static char buf[MAXLINE];

#ifdef SOLARIS
	struct __nsw_switchconfig *nsw_conf;
	enum __nsw_parse_err pserr;
	struct __nsw_lookup *lk;
	int nsw_rc;
	static struct __nsw_lookup lkp0 =
		{ "files", {1, 0, 0, 0}, NULL, NULL };
	static struct __nsw_switchconfig lkp_default =
		{ 0, "sendmail", 3, &lkp0 };

	if ((nsw_conf = __nsw_getconfig(service, &pserr)) == NULL)
		lk = lkp_default.lookups;
	else
		lk = nsw_conf->lookups;
	svcno = 0;
	while (lk != NULL)
	{
		maptype[svcno] = lk->service_name;
		if (lk->actions[__NSW_NOTFOUND] == __NSW_RETURN)
			mapreturn[MA_NOTFOUND] |= 1 << svcno;
		if (lk->actions[__NSW_TRYAGAIN] == __NSW_RETURN)
			mapreturn[MA_TRYAGAIN] |= 1 << svcno;
		if (lk->actions[__NSW_UNAVAIL] == __NSW_RETURN)
			mapreturn[MA_TRYAGAIN] |= 1 << svcno;
		svcno++;
		lk = lk->next;
	}
	return svcno;
#endif

#if defined(ultrix) || defined(__osf__)
	struct svcinfo *svcinfo;
	int svc;

	svcinfo = getsvc();
	if (svcinfo == NULL)
		goto punt;
	if (strcmp(service, "hosts") == 0)
		svc = SVC_HOSTS;
	else if (strcmp(service, "aliases") == 0)
		svc = SVC_ALIASES;
	else if (strcmp(service, "passwd") == 0)
		svc = SVC_PASSWD;
	else
		return -1;
	for (svcno = 0; svcno < SVC_PATHSIZE; svcno++)
	{
		switch (svcinfo->svcpath[svc][svcno])
		{
		  case SVC_LOCAL:
			maptype[svcno] = "files";
			break;

		  case SVC_YP:
			maptype[svcno] = "nis";
			break;

		  case SVC_BIND:
			maptype[svcno] = "dns";
			break;

#ifdef SVC_HESIOD
		  case SVC_HESIOD:
			maptype[svcno] = "hesiod";
			break;
#endif

		  case SVC_LAST:
			return svcno;
		}
	}
	return svcno;
#endif

#if !defined(SOLARIS) && !defined(ultrix) && !defined(__osf__)
	/*
	**  Fall-back mechanism.
	*/

	svcno = 0;
	fp = fopen(ServiceSwitchFile, "r");
	if (fp != NULL)
	{
		while (fgets(buf, sizeof buf, fp) != NULL)
		{
			register char *p;

			p = strpbrk(buf, "#\n");
			if (p != NULL)
				*p = '\0';
			p = strpbrk(buf, " \t");
			if (p != NULL)
				*p++ = '\0';
			if (strcmp(buf, service) != 0)
				continue;

			/* got the right service -- extract data */
			do
			{
				while (isspace(*p))
					p++;
				if (*p == '\0')
					break;
				maptype[svcno++] = p;
				p = strpbrk(p, " \t");
				if (p != NULL)
					*p++ = '\0';
			} while (p != NULL);
			break;
		}
		fclose(fp);
		return svcno;
	}
#endif

	/* if the service file doesn't work, use an absolute fallback */
  punt:
	if (strcmp(service, "aliases") == 0)
	{
		maptype[svcno++] = "files";
#ifdef AUTO_NIS_ALIASES
# ifdef NISPLUS
		maptype[svcno++] = "nisplus";
# endif
# ifdef NIS
		maptype[svcno++] = "nis";
# endif
#endif
		return svcno;
	}
	if (strcmp(service, "hosts") == 0)
	{
# if NAMED_BIND
		maptype[svcno++] = "dns";
# else
#  if defined(sun) && !defined(BSD) && !defined(SOLARIS)
		/* SunOS */
		maptype[svcno++] = "nis";
#  endif
# endif
		maptype[svcno++] = "files";
		return svcno;
	}
	return -1;
}
/*
**  USERNAME -- return the user id of the logged in user.
**
**	Parameters:
**		none.
**
**	Returns:
**		The login name of the logged in user.
**
**	Side Effects:
**		none.
**
**	Notes:
**		The return value is statically allocated.
*/

char *
username()
{
	static char *myname = NULL;
	extern char *getlogin();
	register struct passwd *pw;

	/* cache the result */
	if (myname == NULL)
	{
		myname = getlogin();
		if (myname == NULL || myname[0] == '\0')
		{
			pw = sm_getpwuid(RealUid);
			if (pw != NULL)
				myname = newstr(pw->pw_name);
		}
		else
		{
			uid_t uid = RealUid;

			myname = newstr(myname);
			if ((pw = sm_getpwnam(myname)) == NULL ||
			      (uid != 0 && uid != pw->pw_uid))
			{
				pw = sm_getpwuid(uid);
				if (pw != NULL)
					myname = newstr(pw->pw_name);
			}
		}
		if (myname == NULL || myname[0] == '\0')
		{
			syserr("554 Who are you?");
			myname = "postmaster";
		}
	}

	return (myname);
}
/*
**  TTYPATH -- Get the path of the user's tty
**
**	Returns the pathname of the user's tty.  Returns NULL if
**	the user is not logged in or if s/he has write permission
**	denied.
**
**	Parameters:
**		none
**
**	Returns:
**		pathname of the user's tty.
**		NULL if not logged in or write permission denied.
**
**	Side Effects:
**		none.
**
**	WARNING:
**		Return value is in a local buffer.
**
**	Called By:
**		savemail
*/

char *
ttypath()
{
	struct stat stbuf;
	register char *pathn;
	extern char *ttyname();
	extern char *getlogin();

	/* compute the pathname of the controlling tty */
	if ((pathn = ttyname(2)) == NULL && (pathn = ttyname(1)) == NULL &&
	    (pathn = ttyname(0)) == NULL)
	{
		errno = 0;
		return (NULL);
	}

	/* see if we have write permission */
	if (stat(pathn, &stbuf) < 0 || !bitset(02, stbuf.st_mode))
	{
		errno = 0;
		return (NULL);
	}

	/* see if the user is logged in */
	if (getlogin() == NULL)
		return (NULL);

	/* looks good */
	return (pathn);
}
/*
**  CHECKCOMPAT -- check for From and To person compatible.
**
**	This routine can be supplied on a per-installation basis
**	to determine whether a person is allowed to send a message.
**	This allows restriction of certain types of internet
**	forwarding or registration of users.
**
**	If the hosts are found to be incompatible, an error
**	message should be given using "usrerr" and an EX_ code
**	should be returned.  You can also set to->q_status to
**	a DSN-style status code.
**
**	EF_NO_BODY_RETN can be set in e->e_flags to suppress the
**	body during the return-to-sender function; this should be done
**	on huge messages.  This bit may already be set by the ESMTP
**	protocol.
**
**	Parameters:
**		to -- the person being sent to.
**
**	Returns:
**		an exit status
**
**	Side Effects:
**		none (unless you include the usrerr stuff)
*/

int
checkcompat(to, e)
	register ADDRESS *to;
	register ENVELOPE *e;
{
# ifdef lint
	if (to == NULL)
		to++;
# endif /* lint */

	if (tTd(49, 1))
		printf("checkcompat(to=%s, from=%s)\n",
			to->q_paddr, e->e_from.q_paddr);

# ifdef EXAMPLE_CODE
	/* this code is intended as an example only */
	register STAB *s;

	s = stab("arpa", ST_MAILER, ST_FIND);
	if (s != NULL && strcmp(e->e_from.q_mailer->m_name, "local") != 0 &&
	    to->q_mailer == s->s_mailer)
	{
		usrerr("553 No ARPA mail through this machine: see your system administration");
		/* e->e_flags |= EF_NO_BODY_RETN; to supress body on return */
		to->q_status = "5.7.1";
		return (EX_UNAVAILABLE);
	}
# endif /* EXAMPLE_CODE */
	return (EX_OK);
}
/*
**  SETSIGNAL -- set a signal handler
**
**	This is essentially old BSD "signal(3)".
*/

sigfunc_t
setsignal(sig, handler)
	int sig;
	sigfunc_t handler;
{
#if defined(SYS5SIGNALS) || defined(BSD4_3) || defined(_AUX_SOURCE)
	return signal(sig, handler);
#else
	struct sigaction n, o;

	bzero(&n, sizeof n);
	n.sa_handler = handler;
# ifdef SA_RESTART
	n.sa_flags = SA_RESTART;
# endif
	if (sigaction(sig, &n, &o) < 0)
		return SIG_ERR;
	return o.sa_handler;
#endif
}
/*
**  HOLDSIGS -- arrange to hold all signals
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Arranges that signals are held.
*/

void
holdsigs()
{
}
/*
**  RLSESIGS -- arrange to release all signals
**
**	This undoes the effect of holdsigs.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Arranges that signals are released.
*/

void
rlsesigs()
{
}
/*
**  INIT_MD -- do machine dependent initializations
**
**	Systems that have global modes that should be set should do
**	them here rather than in main.
*/

#ifdef _AUX_SOURCE
# include	<compat.h>
#endif

void
init_md(argc, argv)
	int argc;
	char **argv;
{
#ifdef _AUX_SOURCE
	setcompat(getcompat() | COMPAT_BSDPROT);
#endif

#ifdef VENDOR_DEFAULT
	VendorCode = VENDOR_DEFAULT;
#else
	VendorCode = VENDOR_BERKELEY;
#endif
}
/*
**  INIT_VENDOR_MACROS -- vendor-dependent macro initializations
**
**	Called once, on startup.
**
**	Parameters:
**		e -- the global envelope.
**
**	Returns:
**		none.
**
**	Side Effects:
**		vendor-dependent.
*/

void
init_vendor_macros(e)
	register ENVELOPE *e;
{
}
/*
**  GETLA -- get the current load average
**
**	This code stolen from la.c.
**
**	Parameters:
**		none.
**
**	Returns:
**		The current load average as an integer.
**
**	Side Effects:
**		none.
*/

/* try to guess what style of load average we have */
#define LA_ZERO		1	/* always return load average as zero */
#define LA_INT		2	/* read kmem for avenrun; interpret as long */
#define LA_FLOAT	3	/* read kmem for avenrun; interpret as float */
#define LA_SUBR		4	/* call getloadavg */
#define LA_MACH		5	/* MACH load averages (as on NeXT boxes) */
#define LA_SHORT	6	/* read kmem for avenrun; interpret as short */
#define LA_PROCSTR	7	/* read string ("1.17") from /proc/loadavg */

/* do guesses based on general OS type */
#ifndef LA_TYPE
# define LA_TYPE	LA_ZERO
#endif

#if (LA_TYPE == LA_INT) || (LA_TYPE == LA_FLOAT) || (LA_TYPE == LA_SHORT)

#include <nlist.h>

#ifdef IRIX64
# define nlist		nlist64
#endif

#ifndef LA_AVENRUN
# ifdef SYSTEM5
#  define LA_AVENRUN	"avenrun"
# else
#  define LA_AVENRUN	"_avenrun"
# endif
#endif

/* _PATH_UNIX should be defined in <paths.h> */
#ifndef _PATH_UNIX
# if defined(SYSTEM5)
#  define _PATH_UNIX	"/unix"
# else
#  define _PATH_UNIX	"/vmunix"
# endif
#endif

struct	nlist Nl[] =
{
	{ LA_AVENRUN },
#define	X_AVENRUN	0
	{ 0 },
};

#ifndef FSHIFT
# if defined(unixpc)
#  define FSHIFT	5
# endif

# if defined(__alpha) || defined(IRIX)
#  define FSHIFT	10
# endif

# if defined(_AIX3)
#  define FSHIFT	16
# endif
#endif

#ifndef FSHIFT
# define FSHIFT		8
#endif

#ifndef FSCALE
# define FSCALE		(1 << FSHIFT)
#endif

getla()
{
	static int kmem = -1;
#if LA_TYPE == LA_INT
	long avenrun[3];
#else
# if LA_TYPE == LA_SHORT
	short avenrun[3];
# else
	double avenrun[3];
# endif
#endif
	extern off_t lseek();
	extern int errno;

	if (kmem < 0)
	{
		kmem = open("/dev/kmem", 0, 0);
		if (kmem < 0)
		{
			if (tTd(3, 1))
				printf("getla: open(/dev/kmem): %s\n",
					errstring(errno));
			return (-1);
		}
		(void) fcntl(kmem, F_SETFD, 1);
#ifdef _AIX3
		if (knlist(Nl, 1, sizeof Nl[0]) < 0)
#else
		if (nlist(_PATH_UNIX, Nl) < 0)
#endif
		{
			if (tTd(3, 1))
				printf("getla: nlist(%s): %s\n", _PATH_UNIX,
					errstring(errno));
			return (-1);
		}
		if (Nl[X_AVENRUN].n_value == 0)
		{
			if (tTd(3, 1))
				printf("getla: nlist(%s, %s) ==> 0\n",
					_PATH_UNIX, LA_AVENRUN);
			return (-1);
		}
#ifdef NAMELISTMASK
		Nl[X_AVENRUN].n_value &= NAMELISTMASK;
#endif
	}
	if (tTd(3, 20))
		printf("getla: symbol address = %#x\n", Nl[X_AVENRUN].n_value);
	if (lseek(kmem, (off_t) Nl[X_AVENRUN].n_value, SEEK_SET) == -1 ||
	    read(kmem, (char *) avenrun, sizeof(avenrun)) < sizeof(avenrun))
	{
		/* thank you Ian */
		if (tTd(3, 1))
			printf("getla: lseek or read: %s\n", errstring(errno));
		return (-1);
	}
#if (LA_TYPE == LA_INT) || (LA_TYPE == LA_SHORT)
	if (tTd(3, 5))
	{
		printf("getla: avenrun = %d", avenrun[0]);
		if (tTd(3, 15))
			printf(", %d, %d", avenrun[1], avenrun[2]);
		printf("\n");
	}
	if (tTd(3, 1))
		printf("getla: %d\n", (int) (avenrun[0] + FSCALE/2) >> FSHIFT);
	return ((int) (avenrun[0] + FSCALE/2) >> FSHIFT);
#else
	if (tTd(3, 5))
	{
		printf("getla: avenrun = %g", avenrun[0]);
		if (tTd(3, 15))
			printf(", %g, %g", avenrun[1], avenrun[2]);
		printf("\n");
	}
	if (tTd(3, 1))
		printf("getla: %d\n", (int) (avenrun[0] +0.5));
	return ((int) (avenrun[0] + 0.5));
#endif
}

#else
#if LA_TYPE == LA_SUBR

#ifdef DGUX

#include <sys/dg_sys_info.h>

int
getla()
{
	struct dg_sys_info_load_info load_info;

	dg_sys_info((long *)&load_info,
		DG_SYS_INFO_LOAD_INFO_TYPE, DG_SYS_INFO_LOAD_VERSION_0);

        if (tTd(3, 1))
                printf("getla: %d\n", (int) (load_info.one_minute + 0.5));

	return((int) (load_info.one_minute + 0.5));
}

#else
# ifdef __hpux

struct pst_dynamic;

#  include <sys/param.h>
#  include <sys/pstat.h>

int
getla()
{
	struct pst_dynamic pstd;

	if (pstat_getdynamic(&pstd, sizeof(struct pst_dynamic),
			     (size_t) 1, 0) == -1)
		return 0;

        if (tTd(3, 1))
                printf("getla: %d\n", (int) (pstd.psd_avg_1_min + 0.5));

	return (int) (pstd.psd_avg_1_min + 0.5);
}

# else

int
getla()
{
	double avenrun[3];

	if (getloadavg(avenrun, sizeof(avenrun) / sizeof(avenrun[0])) < 0)
	{
		if (tTd(3, 1))
			perror("getla: getloadavg failed:");
		return (-1);
	}
	if (tTd(3, 1))
		printf("getla: %d\n", (int) (avenrun[0] +0.5));
	return ((int) (avenrun[0] + 0.5));
}

# endif /* __hpux */
#endif /* DGUX */
#else
#if LA_TYPE == LA_MACH

/*
**  This has been tested on NEXTSTEP release 2.1/3.X.
*/

#if defined(NX_CURRENT_COMPILER_RELEASE) && NX_CURRENT_COMPILER_RELEASE > NX_COMPILER_RELEASE_3_0
# include <mach/mach.h>
#else
# include <mach.h>
#endif

getla()
{
	processor_set_t default_set;
	kern_return_t error;
	unsigned int info_count;
	struct processor_set_basic_info info;
	host_t host;

	error = processor_set_default(host_self(), &default_set);
	if (error != KERN_SUCCESS)
		return -1;
	info_count = PROCESSOR_SET_BASIC_INFO_COUNT;
	if (processor_set_info(default_set, PROCESSOR_SET_BASIC_INFO,
			       &host, (processor_set_info_t)&info,
			       &info_count) != KERN_SUCCESS)
	{
		return -1;
	}
	return (int) (info.load_average + (LOAD_SCALE / 2)) / LOAD_SCALE;
}


#else
#if LA_TYPE == LA_PROCSTR

/*
**  Read /proc/loadavg for the load average.  This is assumed to be
**  in a format like "0.15 0.12 0.06".
**
**	Initially intended for Linux.  This has been in the kernel
**	since at least 0.99.15.
*/

# ifndef _PATH_LOADAVG
#  define _PATH_LOADAVG	"/proc/loadavg"
# endif

int
getla()
{
	double avenrun;
	register int result;
	FILE *fp;

	fp = fopen(_PATH_LOADAVG, "r");
	if (fp == NULL) 
	{
		if (tTd(3, 1))
			printf("getla: fopen(%s): %s\n",
				_PATH_LOADAVG, errstring(errno));
		return -1;
	}
	result = fscanf(fp, "%lf", &avenrun);
	fclose(fp);
	if (result != 1)
	{
		if (tTd(3, 1))
			printf("getla: fscanf() = %d: %s\n",
				result, errstring(errno));
		return -1;
	}

	if (tTd(3, 1))
		printf("getla(): %.2f\n", avenrun);

	return ((int) (avenrun + 0.5));
}

#else

getla()
{
	if (tTd(3, 1))
		printf("getla: ZERO\n");
	return (0);
}

#endif
#endif
#endif
#endif


/*
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors:  Many and varied...
 */

/* Non Apollo stuff removed by Don Lewis 11/15/93 */
#ifndef lint
static char  rcsid[] = "@(#)$Id: getloadavg.c,v 1.16 1991/06/21 12:51:15 paul Exp $";
#endif /* !lint */

#ifdef apollo
# undef volatile
#    include <apollo/base.h>

/* ARGSUSED */
int getloadavg( call_data )
     caddr_t	call_data;	/* pointer to (double) return value */
{
     double *avenrun = (double *) call_data;
     int i;
     status_$t      st;
     long loadav[3];
     proc1_$get_loadav(loadav, &st);
     *avenrun = loadav[0] / (double) (1 << 16);
     return(0);
}
#   endif /* apollo */
/*
**  SHOULDQUEUE -- should this message be queued or sent?
**
**	Compares the message cost to the load average to decide.
**
**	Parameters:
**		pri -- the priority of the message in question.
**		ctime -- the message creation time.
**
**	Returns:
**		TRUE -- if this message should be queued up for the
**			time being.
**		FALSE -- if the load is low enough to send this message.
**
**	Side Effects:
**		none.
*/

bool
shouldqueue(pri, ctime)
	long pri;
	time_t ctime;
{
	bool rval;

	if (tTd(3, 30))
		printf("shouldqueue: CurrentLA=%d, pri=%d: ", CurrentLA, pri);
	if (CurrentLA < QueueLA)
	{
		if (tTd(3, 30))
			printf("FALSE (CurrentLA < QueueLA)\n");
		return (FALSE);
	}
	if (CurrentLA >= RefuseLA)
	{
		if (tTd(3, 30))
			printf("TRUE (CurrentLA >= RefuseLA)\n");
		return (TRUE);
	}
	rval = pri > (QueueFactor / (CurrentLA - QueueLA + 1));
	if (tTd(3, 30))
		printf("%s (by calculation)\n", rval ? "TRUE" : "FALSE");
	return rval;
}
/*
**  REFUSECONNECTIONS -- decide if connections should be refused
**
**	Parameters:
**		none.
**
**	Returns:
**		TRUE if incoming SMTP connections should be refused
**			(for now).
**		FALSE if we should accept new work.
**
**	Side Effects:
**		none.
*/

bool
refuseconnections()
{
	extern bool enoughspace();

#ifdef XLA
	if (!xla_smtp_ok())
		return TRUE;
#endif

	/* this is probably too simplistic */
	return CurrentLA >= RefuseLA || !enoughspace(MinBlocksFree + 1);
}
/*
**  SETPROCTITLE -- set process title for ps
**
**	Parameters:
**		fmt -- a printf style format string.
**		a, b, c -- possible parameters to fmt.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Clobbers argv of our main procedure so ps(1) will
**		display the title.
*/

#define SPT_NONE	0	/* don't use it at all */
#define SPT_REUSEARGV	1	/* cover argv with title information */
#define SPT_BUILTIN	2	/* use libc builtin */
#define SPT_PSTAT	3	/* use pstat(PSTAT_SETCMD, ...) */
#define SPT_PSSTRINGS	4	/* use PS_STRINGS->... */
#define SPT_WRITEUDOT	5	/* write u. area in kmem */

#ifndef SPT_TYPE
# define SPT_TYPE	SPT_REUSEARGV
#endif

#if SPT_TYPE != SPT_NONE && SPT_TYPE != SPT_BUILTIN

# if SPT_TYPE == SPT_PSTAT
#  include <sys/pstat.h>
# endif
# if SPT_TYPE == SPT_PSSTRINGS
#  include <machine/vmparam.h>
#  include <sys/exec.h>
#  ifndef PS_STRINGS	/* hmmmm....  apparently not available after all */
#   undef SPT_TYPE
#   define SPT_TYPE	SPT_REUSEARGV
#  else
#   ifndef NKPDE			/* FreeBSD 2.0 */
#    define NKPDE 63
typedef unsigned int	*pt_entry_t;
#   endif
#  endif
# endif

# if SPT_TYPE == SPT_PSSTRINGS
#  define SETPROC_STATIC	static
# else
#  define SETPROC_STATIC
# endif

# ifndef SPT_PADCHAR
#  define SPT_PADCHAR	' '
# endif

#endif /* SPT_TYPE != SPT_NONE && SPT_TYPE != SPT_BUILTIN */

#if SPT_TYPE != SPT_BUILTIN

/*VARARGS1*/
void
# ifdef __STDC__
setproctitle(char *fmt, ...)
# else
setproctitle(fmt, va_alist)
	char *fmt;
	va_dcl
# endif
{
# if SPT_TYPE != SPT_NONE
	register char *p;
	register int i;
	SETPROC_STATIC char buf[MAXLINE];
	VA_LOCAL_DECL
#  if SPT_TYPE == SPT_PSTAT
	union pstun pst;
#  endif
	extern char **Argv;
	extern char *LastArgv;

	p = buf;

	/* print sendmail: heading for grep */
	(void) strcpy(p, "sendmail: ");
	p += strlen(p);

	/* print the argument string */
	VA_START(fmt);
	(void) vsprintf(p, fmt, ap);
	VA_END;

	i = strlen(buf);

#  if SPT_TYPE == SPT_PSTAT
	pst.pst_command = buf;
	pstat(PSTAT_SETCMD, pst, i, 0, 0);
#  else
#   if SPT_TYPE == SPT_PSSTRINGS
	PS_STRINGS->ps_nargvstr = 1;
	PS_STRINGS->ps_argvstr = buf;
#   else
	if (i > LastArgv - Argv[0] - 2)
	{
		i = LastArgv - Argv[0] - 2;
		buf[i] = '\0';
	}
	(void) strcpy(Argv[0], buf);
	p = &Argv[0][i];
	while (p < LastArgv)
		*p++ = SPT_PADCHAR;
	Argv[1] = NULL;
#   endif /* SPT_TYPE == SPT_PSSTRINGS */
#  endif /* SPT_TYPE == SPT_PSTAT */
# endif /* SPT_TYPE != SPT_NONE */
}

#endif /* SPT_TYPE != SPT_BUILTIN */
/*
**  REAPCHILD -- pick up the body of my child, lest it become a zombie
**
**	Parameters:
**		sig -- the signal that got us here (unused).
**
**	Returns:
**		none.
**
**	Side Effects:
**		Picks up extant zombies.
*/

void
reapchild(sig)
	int sig;
{
	int olderrno = errno;
# ifdef HASWAITPID
	auto int status;
	int count;
	int pid;

	count = 0;
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
	{
		if (count++ > 1000)
		{
#ifdef LOG
			syslog(LOG_ALERT, "reapchild: waitpid loop: pid=%d, status=%x",
				pid, status);
#endif
			break;
		}
	}
# else
# ifdef WNOHANG
	union wait status;

	while (wait3(&status, WNOHANG, (struct rusage *) NULL) > 0)
		continue;
# else /* WNOHANG */
	auto int status;

	while (wait(&status) > 0)
		continue;
# endif /* WNOHANG */
# endif
# ifdef SYS5SIGNALS
	(void) setsignal(SIGCHLD, reapchild);
# endif
	errno = olderrno;
}
/*
**  UNSETENV -- remove a variable from the environment
**
**	Not needed on newer systems.
**
**	Parameters:
**		name -- the string name of the environment variable to be
**			deleted from the current environment.
**
**	Returns:
**		none.
**
**	Globals:
**		environ -- a pointer to the current environment.
**
**	Side Effects:
**		Modifies environ.
*/

#ifndef HASUNSETENV

void
unsetenv(name)
	char *name;
{
	extern char **environ;
	register char **pp;
	int len = strlen(name);

	for (pp = environ; *pp != NULL; pp++)
	{
		if (strncmp(name, *pp, len) == 0 &&
		    ((*pp)[len] == '=' || (*pp)[len] == '\0'))
			break;
	}

	for (; *pp != NULL; pp++)
		*pp = pp[1];
}

#endif
/*
**  GETDTABLESIZE -- return number of file descriptors
**
**	Only on non-BSD systems
**
**	Parameters:
**		none
**
**	Returns:
**		size of file descriptor table
**
**	Side Effects:
**		none
*/

#ifdef SOLARIS
# include <sys/resource.h>
#endif

int
getdtsize()
{
#ifdef RLIMIT_NOFILE
	struct rlimit rl;

	if (getrlimit(RLIMIT_NOFILE, &rl) >= 0)
		return rl.rlim_cur;
#endif

# ifdef HASGETDTABLESIZE
	return getdtablesize();
# else
#  ifdef _SC_OPEN_MAX
	return sysconf(_SC_OPEN_MAX);
#  else
	return NOFILE;
#  endif
# endif
}
/*
**  UNAME -- get the UUCP name of this system.
*/

#ifndef HASUNAME

int
uname(name)
	struct utsname *name;
{
	FILE *file;
	char *n;

	name->nodename[0] = '\0';

	/* try /etc/whoami -- one line with the node name */
	if ((file = fopen("/etc/whoami", "r")) != NULL)
	{
		(void) fgets(name->nodename, NODE_LENGTH + 1, file);
		(void) fclose(file);
		n = strchr(name->nodename, '\n');
		if (n != NULL)
			*n = '\0';
		if (name->nodename[0] != '\0')
			return (0);
	}

	/* try /usr/include/whoami.h -- has a #define somewhere */
	if ((file = fopen("/usr/include/whoami.h", "r")) != NULL)
	{
		char buf[MAXLINE];

		while (fgets(buf, MAXLINE, file) != NULL)
			if (sscanf(buf, "#define sysname \"%*[^\"]\"",
					NODE_LENGTH, name->nodename) > 0)
				break;
		(void) fclose(file);
		if (name->nodename[0] != '\0')
			return (0);
	}

#ifdef TRUST_POPEN
	/*
	**  Popen is known to have security holes.
	*/

	/* try uuname -l to return local name */
	if ((file = popen("uuname -l", "r")) != NULL)
	{
		(void) fgets(name, NODE_LENGTH + 1, file);
		(void) pclose(file);
		n = strchr(name, '\n');
		if (n != NULL)
			*n = '\0';
		if (name->nodename[0] != '\0')
			return (0);
	}
#endif
	
	return (-1);
}
#endif /* HASUNAME */
/*
**  INITGROUPS -- initialize groups
**
**	Stub implementation for System V style systems
*/

#ifndef HASINITGROUPS

initgroups(name, basegid)
	char *name;
	int basegid;
{
	return 0;
}

#endif
/*
**  SETSID -- set session id (for non-POSIX systems)
*/

#ifndef HASSETSID

pid_t
setsid __P ((void))
{
#ifdef TIOCNOTTY
	int fd;

	fd = open("/dev/tty", O_RDWR, 0);
	if (fd >= 0)
	{
		(void) ioctl(fd, (int) TIOCNOTTY, (char *) 0);
		(void) close(fd);
	}
#endif /* TIOCNOTTY */
# ifdef SYS5SETPGRP
	return setpgrp();
# else
	return setpgid(0, getpid());
# endif
}

#endif
/*
**  FSYNC -- dummy fsync
*/

#ifdef NEEDFSYNC

fsync(fd)
	int fd;
{
# ifdef O_SYNC
	return fcntl(fd, F_SETFL, O_SYNC);
# else
	/* nothing we can do */
	return 0;
# endif
}

#endif
/*
**  DGUX_INET_ADDR -- inet_addr for DG/UX
**
**	Data General DG/UX version of inet_addr returns a struct in_addr
**	instead of a long.  This patches things.  Only needed on versions
**	prior to 5.4.3.
*/

#ifdef DGUX_5_4_2

#undef inet_addr

long
dgux_inet_addr(host)
	char *host;
{
	struct in_addr haddr;

	haddr = inet_addr(host);
	return haddr.s_addr;
}

#endif
/*
**  GETOPT -- for old systems or systems with bogus implementations
*/

#ifdef NEEDGETOPT

/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */


/*
** this version hacked to add `atend' flag to allow state machine
** to reset if invoked by the program to scan args for a 2nd time
*/

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getopt.c	4.3 (Berkeley) 3/9/86";
#endif /* LIBC_SCCS and not lint */

#include <stdio.h>

/*
 * get option letter from argument vector
 */
#ifdef _CONVEX_SOURCE
extern int	optind, opterr, optopt;
extern char	*optarg;
#else
int	opterr = 1;		/* if error message should be printed */
int	optind = 1;		/* index into parent argv vector */
int	optopt = 0;		/* character checked for validity */
char	*optarg = NULL;		/* argument associated with option */
#endif

#define BADCH	(int)'?'
#define EMSG	""
#define tell(s)	if (opterr) {fputs(*nargv,stderr);fputs(s,stderr); \
		fputc(optopt,stderr);fputc('\n',stderr);return(BADCH);}

getopt(nargc,nargv,ostr)
	int		nargc;
	char *const	*nargv;
	const char	*ostr;
{
	static char	*place = EMSG;	/* option letter processing */
	static char	atend = 0;
	register char	*oli;		/* option letter list index */

	if (atend) {
		atend = 0;
		place = EMSG;
	}
	if(!*place) {			/* update scanning pointer */
		if (optind >= nargc || *(place = nargv[optind]) != '-' || !*++place) {
			atend++;
			return(EOF);
		}
		if (*place == '-') {	/* found "--" */
			++optind;
			atend++;
			return(EOF);
		}
	}				/* option letter okay? */
	if ((optopt = (int)*place++) == (int)':' || !(oli = strchr(ostr,optopt))) {
		if (!*place) ++optind;
		tell(": illegal option -- ");
	}
	if (*++oli != ':') {		/* don't need argument */
		optarg = NULL;
		if (!*place) ++optind;
	}
	else {				/* need an argument */
		if (*place) optarg = place;	/* no white space */
		else if (nargc <= ++optind) {	/* no arg */
			place = EMSG;
			tell(": option requires an argument -- ");
		}
	 	else optarg = nargv[optind];	/* white space */
		place = EMSG;
		++optind;
	}
	return(optopt);			/* dump back option letter */
}

#endif
/*
**  VFPRINTF, VSPRINTF -- for old 4.3 BSD systems missing a real version
*/

#ifdef NEEDVPRINTF

#define MAXARG	16

vfprintf(fp, fmt, ap)
	FILE *	fp;
	char *	fmt;
	char **	ap;
{
	char *	bp[MAXARG];
	int	i = 0;

	while (*ap && i < MAXARG)
		bp[i++] = *ap++;
	fprintf(fp, fmt, bp[0], bp[1], bp[2], bp[3],
			 bp[4], bp[5], bp[6], bp[7],
			 bp[8], bp[9], bp[10], bp[11],
			 bp[12], bp[13], bp[14], bp[15]);
}

vsprintf(s, fmt, ap)
	char *	s;
	char *	fmt;
	char **	ap;
{
	char *	bp[MAXARG];
	int	i = 0;

	while (*ap && i < MAXARG)
		bp[i++] = *ap++;
	sprintf(s, fmt, bp[0], bp[1], bp[2], bp[3],
			bp[4], bp[5], bp[6], bp[7],
			bp[8], bp[9], bp[10], bp[11],
			bp[12], bp[13], bp[14], bp[15]);
}

#endif
/*
**  USERSHELLOK -- tell if a user's shell is ok for unrestricted use
**
**	Parameters:
**		shell -- the user's shell from /etc/passwd
**
**	Returns:
**		TRUE -- if it is ok to use this for unrestricted access.
**		FALSE -- if the shell is restricted.
*/

#if !HASGETUSERSHELL

# ifndef _PATH_SHELLS
#  define _PATH_SHELLS	"/etc/shells"
# endif

char	*DefaultUserShells[] =
{
	"/bin/sh",		/* standard shell */
	"/usr/bin/sh",
	"/bin/csh",		/* C shell */
	"/usr/bin/csh",
#ifdef __hpux
# ifdef V4FS
	"/usr/bin/rsh",		/* restricted Bourne shell */
	"/usr/bin/ksh",		/* Korn shell */
	"/usr/bin/rksh",	/* restricted Korn shell */
	"/usr/bin/pam",
	"/usr/bin/keysh",	/* key shell (extended Korn shell) */
	"/usr/bin/posix/sh",
# else
	"/bin/rsh",		/* restricted Bourne shell */
	"/bin/ksh",		/* Korn shell */
	"/bin/rksh",		/* restricted Korn shell */
	"/bin/pam",
	"/usr/bin/keysh",	/* key shell (extended Korn shell) */
	"/bin/posix/sh",
# endif
#endif
#ifdef _AIX3
	"/bin/ksh",		/* Korn shell */
	"/usr/bin/ksh",
	"/bin/tsh",		/* trusted shell */
	"/usr/bin/tsh",
	"/bin/bsh",		/* Bourne shell */
	"/usr/bin/bsh",
#endif
	NULL
};

#endif

#define WILDCARD_SHELL	"/SENDMAIL/ANY/SHELL/"

bool
usershellok(shell)
	char *shell;
{
#if HASGETUSERSHELL
	register char *p;
	extern char *getusershell();

	if (shell == NULL || shell[0] == '\0')
		return TRUE;

	setusershell();
	while ((p = getusershell()) != NULL)
		if (strcmp(p, shell) == 0 || strcmp(p, WILDCARD_SHELL) == 0)
			break;
	endusershell();
	return p != NULL;
#else
	register FILE *shellf;
	char buf[MAXLINE];

	if (shell == NULL || shell[0] == '\0')
		return TRUE;

	shellf = fopen(_PATH_SHELLS, "r");
	if (shellf == NULL)
	{
		/* no /etc/shells; see if it is one of the std shells */
		char **d;

		for (d = DefaultUserShells; *d != NULL; d++)
		{
			if (strcmp(shell, *d) == 0)
				return TRUE;
		}
		return FALSE;
	}

	while (fgets(buf, sizeof buf, shellf) != NULL)
	{
		register char *p, *q;

		p = buf;
		while (*p != '\0' && *p != '#' && *p != '/')
			p++;
		if (*p == '#' || *p == '\0')
			continue;
		q = p;
		while (*p != '\0' && *p != '#' && !isspace(*p))
			p++;
		*p = '\0';
		if (strcmp(shell, q) == 0 || strcmp(WILDCARD_SHELL, q) == 0)
		{
			fclose(shellf);
			return TRUE;
		}
	}
	fclose(shellf);
	return FALSE;
#endif
}
/*
**  FREESPACE -- see how much free space is on the queue filesystem
**
**	Only implemented if you have statfs.
**
**	Parameters:
**		dir -- the directory in question.
**		bsize -- a variable into which the filesystem
**			block size is stored.
**
**	Returns:
**		The number of bytes free on the queue filesystem.
**		-1 if the statfs call fails.
**
**	Side effects:
**		Puts the filesystem block size into bsize.
*/

/* statfs types */
#define SFS_NONE	0	/* no statfs implementation */
#define SFS_USTAT	1	/* use ustat */
#define SFS_4ARGS	2	/* use four-argument statfs call */
#define SFS_VFS		3	/* use <sys/vfs.h> implementation */
#define SFS_MOUNT	4	/* use <sys/mount.h> implementation */
#define SFS_STATFS	5	/* use <sys/statfs.h> implementation */
#define SFS_STATVFS	6	/* use <sys/statvfs.h> implementation */

#ifndef SFS_TYPE
# define SFS_TYPE	SFS_NONE
#endif

#if SFS_TYPE == SFS_USTAT
# include <ustat.h>
#endif
#if SFS_TYPE == SFS_4ARGS || SFS_TYPE == SFS_STATFS
# include <sys/statfs.h>
#endif
#if SFS_TYPE == SFS_VFS
# include <sys/vfs.h>
#endif
#if SFS_TYPE == SFS_MOUNT
# include <sys/mount.h>
#endif
#if SFS_TYPE == SFS_STATVFS
# include <sys/statvfs.h>
#endif

long
freespace(dir, bsize)
	char *dir;
	long *bsize;
{
#if SFS_TYPE != SFS_NONE
# if SFS_TYPE == SFS_USTAT
	struct ustat fs;
	struct stat statbuf;
#  define FSBLOCKSIZE	DEV_BSIZE
#  define SFS_BAVAIL	f_tfree
# else
#  if defined(ultrix)
	struct fs_data fs;
#   define SFS_BAVAIL	fd_bfreen
#   define FSBLOCKSIZE	1024L
#  else
#   if SFS_TYPE == SFS_STATVFS
	struct statvfs fs;
#    define FSBLOCKSIZE	fs.f_frsize
#   else
	struct statfs fs;
#    define FSBLOCKSIZE	fs.f_bsize
#   endif
#  endif
# endif
# ifndef SFS_BAVAIL
#  define SFS_BAVAIL f_bavail
# endif

# if SFS_TYPE == SFS_USTAT
	if (stat(dir, &statbuf) == 0 && ustat(statbuf.st_dev, &fs) == 0)
# else
#  if SFS_TYPE == SFS_4ARGS
	if (statfs(dir, &fs, sizeof fs, 0) == 0)
#  else
#   if SFS_TYPE == SFS_STATVFS
	if (statvfs(dir, &fs) == 0)
#   else
#    if defined(ultrix)
	if (statfs(dir, &fs) > 0)
#    else
	if (statfs(dir, &fs) == 0)
#    endif
#   endif
#  endif
# endif
	{
		if (bsize != NULL)
			*bsize = FSBLOCKSIZE;
		return (fs.SFS_BAVAIL);
	}
#endif
	return (-1);
}
/*
**  ENOUGHSPACE -- check to see if there is enough free space on the queue fs
**
**	Only implemented if you have statfs.
**
**	Parameters:
**		msize -- the size to check against.  If zero, we don't yet
**		know how big the message will be, so just check for
**		a "reasonable" amount.
**
**	Returns:
**		TRUE if there is enough space.
**		FALSE otherwise.
*/

bool
enoughspace(msize)
	long msize;
{
	long bfree, bsize;

	if (MinBlocksFree <= 0 && msize <= 0)
	{
		if (tTd(4, 80))
			printf("enoughspace: no threshold\n");
		return TRUE;
	}

	if ((bfree = freespace(QueueDir, &bsize)) >= 0)
	{
		if (tTd(4, 80))
			printf("enoughspace: bavail=%ld, need=%ld\n",
				bfree, msize);

		/* convert msize to block count */
		msize = msize / bsize + 1;
		if (MinBlocksFree >= 0)
			msize += MinBlocksFree;

		if (bfree < msize)
		{
#ifdef LOG
			if (LogLevel > 0)
				syslog(LOG_ALERT,
					"%s: low on space (have %ld, %s needs %ld in %s)",
					CurEnv->e_id == NULL ? "[NOQUEUE]" : CurEnv->e_id,
					bfree,
					CurHostName == NULL ? "SMTP-DAEMON" : CurHostName,
					msize, QueueDir);
#endif
			return FALSE;
		}
	}
	else if (tTd(4, 80))
		printf("enoughspace failure: min=%ld, need=%ld: %s\n",
			MinBlocksFree, msize, errstring(errno));
	return TRUE;
}
/*
**  TRANSIENTERROR -- tell if an error code indicates a transient failure
**
**	This looks at an errno value and tells if this is likely to
**	go away if retried later.
**
**	Parameters:
**		err -- the errno code to classify.
**
**	Returns:
**		TRUE if this is probably transient.
**		FALSE otherwise.
*/

bool
transienterror(err)
	int err;
{
	switch (err)
	{
	  case EIO:			/* I/O error */
	  case ENXIO:			/* Device not configured */
	  case EAGAIN:			/* Resource temporarily unavailable */
	  case ENOMEM:			/* Cannot allocate memory */
	  case ENODEV:			/* Operation not supported by device */
	  case ENFILE:			/* Too many open files in system */
	  case EMFILE:			/* Too many open files */
	  case ENOSPC:			/* No space left on device */
#ifdef ETIMEDOUT
	  case ETIMEDOUT:		/* Connection timed out */
#endif
#ifdef ESTALE
	  case ESTALE:			/* Stale NFS file handle */
#endif
#ifdef ENETDOWN
	  case ENETDOWN:		/* Network is down */
#endif
#ifdef ENETUNREACH
	  case ENETUNREACH:		/* Network is unreachable */
#endif
#ifdef ENETRESET
	  case ENETRESET:		/* Network dropped connection on reset */
#endif
#ifdef ECONNABORTED
	  case ECONNABORTED:		/* Software caused connection abort */
#endif
#ifdef ECONNRESET
	  case ECONNRESET:		/* Connection reset by peer */
#endif
#ifdef ENOBUFS
	  case ENOBUFS:			/* No buffer space available */
#endif
#ifdef ESHUTDOWN
	  case ESHUTDOWN:		/* Can't send after socket shutdown */
#endif
#ifdef ECONNREFUSED
	  case ECONNREFUSED:		/* Connection refused */
#endif
#ifdef EHOSTDOWN
	  case EHOSTDOWN:		/* Host is down */
#endif
#ifdef EHOSTUNREACH
	  case EHOSTUNREACH:		/* No route to host */
#endif
#ifdef EDQUOT
	  case EDQUOT:			/* Disc quota exceeded */
#endif
#ifdef EPROCLIM
	  case EPROCLIM:		/* Too many processes */
#endif
#ifdef EUSERS
	  case EUSERS:			/* Too many users */
#endif
#ifdef EDEADLK
	  case EDEADLK:			/* Resource deadlock avoided */
#endif
#ifdef EISCONN
	  case EISCONN:			/* Socket already connected */
#endif
#ifdef EINPROGRESS
	  case EINPROGRESS:		/* Operation now in progress */
#endif
#ifdef EALREADY
	  case EALREADY:		/* Operation already in progress */
#endif
#ifdef EADDRINUSE
	  case EADDRINUSE:		/* Address already in use */
#endif
#ifdef EADDRNOTAVAIL
	  case EADDRNOTAVAIL:		/* Can't assign requested address */
#endif
#ifdef ETXTBSY
	  case ETXTBSY:			/* (Apollo) file locked */
#endif
#if defined(ENOSR) && (!defined(ENOBUFS) || (ENOBUFS != ENOSR))
	  case ENOSR:			/* Out of streams resources */
#endif
		return TRUE;
	}

	/* nope, must be permanent */
	return FALSE;
}
/*
**  LOCKFILE -- lock a file using flock or (shudder) fcntl locking
**
**	Parameters:
**		fd -- the file descriptor of the file.
**		filename -- the file name (for error messages).
**		ext -- the filename extension.
**		type -- type of the lock.  Bits can be:
**			LOCK_EX -- exclusive lock.
**			LOCK_NB -- non-blocking.
**
**	Returns:
**		TRUE if the lock was acquired.
**		FALSE otherwise.
*/

bool
lockfile(fd, filename, ext, type)
	int fd;
	char *filename;
	char *ext;
	int type;
{
# if !HASFLOCK
	int action;
	struct flock lfd;

	if (ext == NULL)
		ext = "";
		
	bzero(&lfd, sizeof lfd);
	if (bitset(LOCK_UN, type))
		lfd.l_type = F_UNLCK;
	else if (bitset(LOCK_EX, type))
		lfd.l_type = F_WRLCK;
	else
		lfd.l_type = F_RDLCK;

	if (bitset(LOCK_NB, type))
		action = F_SETLK;
	else
		action = F_SETLKW;

	if (tTd(55, 60))
		printf("lockfile(%s%s, action=%d, type=%d): ",
			filename, ext, action, lfd.l_type);

	if (fcntl(fd, action, &lfd) >= 0)
	{
		if (tTd(55, 60))
			printf("SUCCESS\n");
		return TRUE;
	}

	if (tTd(55, 60))
		printf("(%s) ", errstring(errno));

	/*
	**  On SunOS, if you are testing using -oQ/tmp/mqueue or
	**  -oA/tmp/aliases or anything like that, and /tmp is mounted
	**  as type "tmp" (that is, served from swap space), the
	**  previous fcntl will fail with "Invalid argument" errors.
	**  Since this is fairly common during testing, we will assume
	**  that this indicates that the lock is successfully grabbed.
	*/

	if (errno == EINVAL)
	{
		if (tTd(55, 60))
			printf("SUCCESS\n");
		return TRUE;
	}

	if (!bitset(LOCK_NB, type) || (errno != EACCES && errno != EAGAIN))
	{
		int omode = -1;
#  ifdef F_GETFL
		int oerrno = errno;

		(void) fcntl(fd, F_GETFL, &omode);
		errno = oerrno;
#  endif
		syserr("cannot lockf(%s%s, fd=%d, type=%o, omode=%o, euid=%d)",
			filename, ext, fd, type, omode, geteuid());
	}
# else
	if (ext == NULL)
		ext = "";

	if (tTd(55, 60))
		printf("lockfile(%s%s, type=%o): ", filename, ext, type);

	if (flock(fd, type) >= 0)
	{
		if (tTd(55, 60))
			printf("SUCCESS\n");
		return TRUE;
	}

	if (tTd(55, 60))
		printf("(%s) ", errstring(errno));

	if (!bitset(LOCK_NB, type) || errno != EWOULDBLOCK)
	{
		int omode = -1;
#  ifdef F_GETFL
		int oerrno = errno;

		(void) fcntl(fd, F_GETFL, &omode);
		errno = oerrno;
#  endif
		syserr("cannot flock(%s%s, fd=%d, type=%o, omode=%o, euid=%d)",
			filename, ext, fd, type, omode, geteuid());
	}
# endif
	if (tTd(55, 60))
		printf("FAIL\n");
	return FALSE;
}
/*
**  CHOWNSAFE -- tell if chown is "safe" (executable only by root)
**
**	Parameters:
**		fd -- the file descriptor to check.
**
**	Returns:
**		TRUE -- if only root can chown the file to an arbitrary
**			user.
**		FALSE -- if an arbitrary user can give away a file.
*/

bool
chownsafe(fd)
	int fd;
{
#ifdef __hpux
	char *s;
	int tfd;
	uid_t o_uid, o_euid;
	gid_t o_gid, o_egid;
	bool rval;
	struct stat stbuf;

	o_uid = getuid();
	o_euid = geteuid();
	o_gid = getgid();
	o_egid = getegid();
	fstat(fd, &stbuf);
	setresuid(stbuf.st_uid, stbuf.st_uid, -1);
	setresgid(stbuf.st_gid, stbuf.st_gid, -1);
	s = tmpnam(NULL);
	tfd = open(s, O_RDONLY|O_CREAT, 0600);
	rval = fchown(tfd, DefUid, DefGid) != 0;
	close(tfd);
	unlink(s);
	setreuid(o_uid, o_euid);
	setresgid(o_gid, o_egid, -1);
	return rval;
#else
# ifdef _POSIX_CHOWN_RESTRICTED
#  if _POSIX_CHOWN_RESTRICTED == -1
	return FALSE;
#  else
	return TRUE;
#  endif
# else
#  ifdef _PC_CHOWN_RESTRICTED
	int rval;

	/*
	**  Some systems (e.g., SunOS) seem to have the call and the
	**  #define _PC_CHOWN_RESTRICTED, but don't actually implement
	**  the call.  This heuristic checks for that.
	*/

	errno = 0;
	rval = fpathconf(fd, _PC_CHOWN_RESTRICTED);
	if (errno == 0)
		return rval > 0;
#  endif
#  ifdef BSD
	return TRUE;
#  else
	return FALSE;
#  endif
# endif
#endif
}
/*
**  RESETLIMITS -- reset system controlled resource limits
**
**	This is to avoid denial-of-service attacks
**
**	Parameters:
**		none
**
**	Returns:
**		none
*/

#if HASSETRLIMIT
# include <sys/resource.h>
#endif

void
resetlimits()
{
#if HASSETRLIMIT
	struct rlimit lim;

	lim.rlim_cur = lim.rlim_max = RLIM_INFINITY;
	(void) setrlimit(RLIMIT_CPU, &lim);
	(void) setrlimit(RLIMIT_FSIZE, &lim);
#else
# if HASULIMIT
	(void) ulimit(2, 0x3fffff);
# endif
#endif
}
/*
**  GETCFNAME -- return the name of the .cf file.
**
**	Some systems (e.g., NeXT) determine this dynamically.
*/

char *
getcfname()
{
	int i;
	static char cbuf[200];

	if (ConfFile != NULL)
		return ConfFile;
#ifdef NETINFO
	{
		extern char *ni_propval();
		char *cflocation;

		cflocation = ni_propval("/locations", NULL, "sendmail",
					"sendmail.cf", '\0');
		if (cflocation != NULL)
			return cflocation;
	}
#endif

	/*
	**  Try sendmail.8.6.12.cf, then sendmail.8.6.cf, then
	**  sendmail.8.cf, and finally sendmail.cf.
	**
	**	I suppose it should really try a search path here --
	**	e.g., /etc/sendmail.cf, /etc/mail/sendmail.cf,
	**	/usr/lib/sendmail.cf, and so forth.
	*/

	strcpy(cbuf, _PATH_SENDMAILCF);
	i = strlen(cbuf);
	if (strcmp(&cbuf[i - 3], ".cf") == 0)
	{
		char *p;
		extern char Version[];

		strcpy(&cbuf[i - 2], Version);
		p = strchr(&cbuf[i - 2], '/');
		if (p != NULL)
			*p = '\0';
		p = &cbuf[strlen(cbuf)];
		do
		{
			int fd;

			strcpy(p, ".cf");
			if ((fd = open(cbuf, O_RDONLY, 0)) >= 0)
			{
				close(fd);
				return cbuf;
			}
			*p = '\0';
		} while ((p = strrchr(&cbuf[i - 2], '.')) != NULL);
	}
	return _PATH_SENDMAILCF;
}
/*
**  SETVENDOR -- process vendor code from V configuration line
**
**	Parameters:
**		vendor -- string representation of vendor.
**
**	Returns:
**		TRUE -- if ok.
**		FALSE -- if vendor code could not be processed.
**
**	Side Effects:
**		It is reasonable to set mode flags here to tweak
**		processing in other parts of the code if necessary.
**		For example, if you are a vendor that uses $%y to
**		indicate YP lookups, you could enable that here.
*/

bool
setvendor(vendor)
	char *vendor;
{
	if (strcasecmp(vendor, "Berkeley") == 0)
	{
		VendorCode = VENDOR_BERKELEY;
		return TRUE;
	}

	/* add vendor extensions here */

#ifdef SUN_EXTENSIONS
	if (strcasecmp(vendor, "Sun") == 0)
	{
		VendorCode = VENDOR_SUN;
		return TRUE;
	}
#endif

	return FALSE;
}
/*
**  STRTOL -- convert string to long integer
**
**	For systems that don't have it in the C library.
**
**	This is taken verbatim from the 4.4-Lite C library.
*/

#ifdef NEEDSTRTOL

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)strtol.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */

#include <limits.h>

/*
 * Convert a string to a long integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */

long
strtol(nptr, endptr, base)
	const char *nptr;
	char **endptr;
	register int base;
{
	register const char *s = nptr;
	register unsigned long acc;
	register int c;
	register unsigned long cutoff;
	register int neg = 0, any, cutlim;

	/*
	 * Skip white space and pick up leading +/- sign if any.
	 * If base is 0, allow 0x for hex and 0 for octal, else
	 * assume decimal; if base is already 16, allow 0x.
	 */
	do {
		c = *s++;
	} while (isspace(c));
	if (c == '-') {
		neg = 1;
		c = *s++;
	} else if (c == '+')
		c = *s++;
	if ((base == 0 || base == 16) &&
	    c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;

	/*
	 * Compute the cutoff value between legal numbers and illegal
	 * numbers.  That is the largest legal value, divided by the
	 * base.  An input number that is greater than this value, if
	 * followed by a legal input character, is too big.  One that
	 * is equal to this value may be valid or not; the limit
	 * between valid and invalid numbers is then based on the last
	 * digit.  For instance, if the range for longs is
	 * [-2147483648..2147483647] and the input base is 10,
	 * cutoff will be set to 214748364 and cutlim to either
	 * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
	 * a value > 214748364, or equal but the next digit is > 7 (or 8),
	 * the number is too big, and we will return a range error.
	 *
	 * Set any if any `digits' consumed; make it negative to indicate
	 * overflow.
	 */
	cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
	cutlim = cutoff % (unsigned long)base;
	cutoff /= (unsigned long)base;
	for (acc = 0, any = 0;; c = *s++) {
		if (isdigit(c))
			c -= '0';
		else if (isalpha(c))
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0 || acc > cutoff || acc == cutoff && c > cutlim)
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if (any < 0) {
		acc = neg ? LONG_MIN : LONG_MAX;
		errno = ERANGE;
	} else if (neg)
		acc = -acc;
	if (endptr != 0)
		*endptr = (char *)(any ? s - 1 : nptr);
	return (acc);
}

#endif
/*
**  SM_GETHOSTBY{NAME,ADDR} -- compatibility routines for gethostbyXXX
**
**	Some operating systems have wierd problems with the gethostbyXXX
**	routines.  For example, Solaris versions at least through 2.3
**	don't properly deliver a canonical h_name field.  This tries to
**	work around these problems.
*/

struct hostent *
sm_gethostbyname(name)
	char *name;
{
#if defined(SOLARIS) && SOLARIS < 204
	extern int h_errno;

# if SOLARIS == 203
	static struct hostent hp;
	static char buf[1000];
	extern struct hostent *_switch_gethostbyname_r();

	return _switch_gethostbyname_r(name, &hp, buf, sizeof(buf), &h_errno);
# else
	extern struct hostent *__switch_gethostbyname();

	return __switch_gethostbyname(name);
# endif
#else
	return gethostbyname(name);
#endif
}

struct hostent *
sm_gethostbyaddr(addr, len, type)
	char *addr;
	int len;
	int type;
{
#if defined(SOLARIS) && SOLARIS < 204
	extern int h_errno;

# if SOLARIS == 203
	static struct hostent hp;
	static char buf[1000];
	extern struct hostent *_switch_gethostbyaddr_r();

	return _switch_gethostbyaddr_r(addr, len, type, &hp, buf, sizeof(buf), &h_errno);
# else
	extern struct hostent *__switch_gethostbyaddr();

	return __switch_gethostbyaddr(addr, len, type);
# endif
#else
	return gethostbyaddr(addr, len, type);
#endif
}
/*
**  SM_GETPW{NAM,UID} -- wrapper for getpwnam and getpwuid
*/

struct passwd *
sm_getpwnam(user)
	char *user;
{
	return getpwnam(user);
}

struct passwd *
sm_getpwuid(uid)
	uid_t uid;
{
	return getpwuid(uid);
}
/*
**  NI_PROPVAL -- netinfo property value lookup routine
**
**	Parameters:
**		keydir -- the Netinfo directory name in which to search
**			for the key.
**		keyprop -- the name of the property in which to find the
**			property we are interested.  Defaults to "name".
**		keyval -- the value for which we are really searching.
**		valprop -- the property name for the value in which we
**			are interested.
**		sepchar -- if non-nil, this can be multiple-valued, and
**			we should return a string separated by this
**			character.
**
**	Returns:
**		NULL -- if:
**			1. the directory is not found
**			2. the property name is not found
**			3. the property contains multiple values
**			4. some error occured
**		else -- the location of the config file.
**
**	Example:
**		To search for an alias value, use:
**		  ni_propval("/aliases", "name", aliasname, "members", ',')
**
**	Notes:
**      	Caller should free the return value of ni_proval
*/

#ifdef NETINFO

# include <netinfo/ni.h>

# define LOCAL_NETINFO_DOMAIN    "."
# define PARENT_NETINFO_DOMAIN   ".."
# define MAX_NI_LEVELS           256

char *
ni_propval(keydir, keyprop, keyval, valprop, sepchar)
	char *keydir;
	char *keyprop;
	char *keyval;
	char *valprop;
	char sepchar;
{
	char *propval = NULL;
	int i;
	int j, alen;
	void *ni = NULL;
	void *lastni = NULL;
	ni_status nis;
	ni_id nid;
	ni_namelist ninl;
	register char *p;
	char keybuf[1024];

	/*
	**  Create the full key from the two parts.
	**
	**	Note that directory can end with, e.g., "name=" to specify
	**	an alternate search property.
	*/

	i = strlen(keydir) + strlen(keyval) + 2;
	if (keyprop != NULL)
		i += strlen(keyprop) + 1;
	if (i > sizeof keybuf)
		return NULL;
	strcpy(keybuf, keydir);
	strcat(keybuf, "/");
	if (keyprop != NULL)
	{
		strcat(keybuf, keyprop);
		strcat(keybuf, "=");
	}
	strcat(keybuf, keyval);

	/*
	**  If the passed directory and property name are found
	**  in one of netinfo domains we need to search (starting
	**  from the local domain moving all the way back to the
	**  root domain) set propval to the property's value
	**  and return it.
	*/

	for (i = 0; i < MAX_NI_LEVELS; ++i)
	{
		if (i == 0)
		{
			nis = ni_open(NULL, LOCAL_NETINFO_DOMAIN, &ni);
		}
		else
		{
			if (lastni != NULL)
				ni_free(lastni);
			lastni = ni;
			nis = ni_open(lastni, PARENT_NETINFO_DOMAIN, &ni);
		}

		/*
		**  Don't bother if we didn't get a handle on a
		**  proper domain.  This is not necessarily an error.
		**  We would get a positive ni_status if, for instance
		**  we never found the directory or property and tried
		**  to open the parent of the root domain!
		*/

		if (nis != 0)
			break;

		/*
		**  Find the path to the server information.
		*/

		if (ni_pathsearch(ni, &nid, keybuf) != 0)
			continue;

		/*
		**  Find associated value information.
		*/

		if (ni_lookupprop(ni, &nid, valprop, &ninl) != 0)
			continue;

		/*
		**  See if we have an acceptable number of values.
		*/

		if (ninl.ni_namelist_len <= 0)
			continue;

		if (sepchar == '\0' && ninl.ni_namelist_len > 1)
		{
			ni_namelist_free(&ninl);
			continue;
		}

		/* 
		**  Calculate number of bytes needed and build result
		*/

		alen = 1;
		for (j = 0; j < ninl.ni_namelist_len; j++)
			alen += strlen(ninl.ni_namelist_val[j]) + 1;
		propval = p = xalloc(alen);
		for (j = 0; j < ninl.ni_namelist_len; j++)
		{
			strcpy(p, ninl.ni_namelist_val[j]);
			p += strlen(p);
			*p++ = sepchar;
		} 
		*--p = '\0';

		ni_namelist_free(&ninl);
	}

	/*
	**  Clean up.
	*/

	if (ni != NULL)
		ni_free(ni);
	if (lastni != NULL && ni != lastni)
		ni_free(lastni);

	return propval;
}

#endif /* NETINFO */
/*
**  HARD_SYSLOG -- call syslog repeatedly until it works
**
**	Needed on HP-UX, which apparently doesn't guarantee that
**	syslog succeeds during interrupt handlers.
*/

#ifdef __hpux

# define MAXSYSLOGTRIES	100
# undef syslog

# ifdef __STDC__
hard_syslog(int pri, char *msg, ...)
# else
hard_syslog(pri, msg, va_alist)
	int pri;
	char *msg;
	va_dcl
# endif
{
	int i;
	char buf[SYSLOG_BUFSIZE * 2];
	VA_LOCAL_DECL;

	VA_START(msg);
	vsprintf(buf, msg, ap);
	VA_END;

	for (i = MAXSYSLOGTRIES; --i >= 0 && syslog(pri, "%s", buf) < 0; )
		continue;
}

#endif
/*
**  LOCAL_HOSTNAME_LENGTH
**
**	This is required to get sendmail to compile against BIND 4.9.x
**	on Ultrix.
*/

#if defined(ultrix) && NAMED_BIND

# include <resolv.h>
# if __RES >= 19931104

int
local_hostname_length(hostname)
	char *hostname;
{
	int len_host, len_domain;

	if (!*_res.defdname)
		res_init();
	len_host = strlen(hostname);
	len_domain = strlen(_res.defdname);
	if (len_host > len_domain &&
	    (strcasecmp(hostname + len_host - len_domain,_res.defdname) == 0) &&
	    hostname[len_host - len_domain - 1] == '.')
		return len_host - len_domain - 1;
	else
		return 0;
}

# endif
#endif
/*
**  Compile-Time options
*/

char	*CompileOptions[] =
{
#if HESIOD
	"HESIOD",
#endif
#if LOG
	"LOG",
#endif
#if MATCHGECOS
	"MATCHGECOS",
#endif
#if NAMED_BIND
	"NAMED_BIND",
#endif
#if NDBM
	"NDBM",
#endif
#if NETINET
	"NETINET",
#endif
#if NETINFO
	"NETINFO",
#endif
#if NETISO
	"NETISO",
#endif
#if NETNS
	"NETNS",
#endif
#if NETUNIX
	"NETUNIX",
#endif
#if NETX25
	"NETX25",
#endif
#if NEWDB
	"NEWDB",
#endif
#if NIS
	"NIS",
#endif
#if SCANF
	"SCANF",
#endif
#if SUID_ROOT_FILES_OK
	"SUID_ROOT_FILES_OK",
#endif
#if USERDB
	"USERDB",
#endif
#if XDEBUG
	"XDEBUG",
#endif
#if XLA
	"XLA",
#endif
	NULL
};
