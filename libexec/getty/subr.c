/*
 * Copyright (c) 1983 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)subr.c	5.13 (Berkeley) 5/27/93";
#endif /* not lint */

/*
 * Melbourne getty.
 */
#define USE_OLD_TTY
#include <stdlib.h>
#include <sgtty.h>
#include <string.h>
#include <unistd.h>

#include "gettytab.h"
#include "extern.h"
#include "pathnames.h"

extern	struct sgttyb tmode;
extern	struct tchars tc;
extern	struct ltchars ltc;

/*
 * Get a table entry.
 */
void
gettable(name, buf)
	char *name, *buf;
{
	register struct gettystrs *sp;
	register struct gettynums *np;
	register struct gettyflags *fp;
	long n;
	char *dba[2];
	dba[0] = _PATH_GETTYTAB;
	dba[1] = 0;

	if (cgetent(&buf, dba, name) != 0)
		return;

	for (sp = gettystrs; sp->field; sp++) 
		cgetstr(buf, sp->field, &sp->value);
	for (np = gettynums; np->field; np++) {
		if (cgetnum(buf, np->field, &n) == -1)
			np->set = 0;
		else {
			np->set = 1;
			np->value = n;
		}
	}
	for (fp = gettyflags; fp->field; fp++) {
		if (cgetcap(buf, fp->field, ':') == NULL)
			fp->set = 0;
		else {
			fp->set = 1;
			fp->value = 1 ^ fp->invrt;
		}
	}
#ifdef DEBUG
	printf("name=\"%s\", buf=\"%s\"\n", name, buf);
	for (sp = gettystrs; sp->field; sp++)
		printf("cgetstr: %s=%s\n", sp->field, sp->value);
	for (np = gettynums; np->field; np++)
		printf("cgetnum: %s=%d\n", np->field, np->value);
	for (fp = gettyflags; fp->field; fp++)
		printf("cgetflags: %s='%c' set='%c'\n", fp->field, 
		       fp->value + '0', fp->set + '0');
	exit(1);
#endif /* DEBUG */
}

void
gendefaults()
{
	register struct gettystrs *sp;
	register struct gettynums *np;
	register struct gettyflags *fp;

	for (sp = gettystrs; sp->field; sp++)
		if (sp->value)
			sp->defalt = sp->value;
	for (np = gettynums; np->field; np++)
		if (np->set)
			np->defalt = np->value;
	for (fp = gettyflags; fp->field; fp++)
		if (fp->set)
			fp->defalt = fp->value;
		else
			fp->defalt = fp->invrt;
}

void
setdefaults()
{
	register struct gettystrs *sp;
	register struct gettynums *np;
	register struct gettyflags *fp;

	for (sp = gettystrs; sp->field; sp++)
		if (!sp->value)
			sp->value = sp->defalt;
	for (np = gettynums; np->field; np++)
		if (!np->set)
			np->value = np->defalt;
	for (fp = gettyflags; fp->field; fp++)
		if (!fp->set)
			fp->value = fp->defalt;
}

static char **
charnames[] = {
	&ER, &KL, &IN, &QU, &XN, &XF, &ET, &BK,
	&SU, &DS, &RP, &FL, &WE, &LN, 0
};

static char *
charvars[] = {
	&tmode.sg_erase, &tmode.sg_kill, &tc.t_intrc,
	&tc.t_quitc, &tc.t_startc, &tc.t_stopc,
	&tc.t_eofc, &tc.t_brkc, &ltc.t_suspc,
	&ltc.t_dsuspc, &ltc.t_rprntc, &ltc.t_flushc,
	&ltc.t_werasc, &ltc.t_lnextc, 0
};

void
setchars()
{
	register int i;
	register char *p;

	for (i = 0; charnames[i]; i++) {
		p = *charnames[i];
		if (p && *p)
			*charvars[i] = *p;
		else
			*charvars[i] = '\377';
	}
}

long
setflags(n)
	int n;
{
	register long f;

	switch (n) {
	case 0:
		if (F0set)
			return(F0);
		break;
	case 1:
		if (F1set)
			return(F1);
		break;
	default:
		if (F2set)
			return(F2);
		break;
	}

	f = 0;

	if (AP)
		f |= ANYP;
	else if (OP)
		f |= ODDP;
	else if (EP)
		f |= EVENP;

	if (UC)
		f |= LCASE;

	if (NL)
		f |= CRMOD;

	f |= delaybits();

	if (n == 1) {		/* read mode flags */
		if (RW)
			f |= RAW;
		else
			f |= CBREAK;
		return (f);
	}

	if (!HT)
		f |= XTABS;

	if (n == 0)
		return (f);

	if (CB)
		f |= CRTBS;

	if (CE)
		f |= CRTERA;

	if (CK)
		f |= CRTKIL;

	if (PE)
		f |= PRTERA;

	if (EC)
		f |= ECHO;

	if (XC)
		f |= CTLECH;

	if (DX)
		f |= DECCTQ;

	return (f);
}

struct delayval {
	unsigned	delay;		/* delay in ms */
	int		bits;
};

/*
 * below are random guesses, I can't be bothered checking
 */

struct delayval	crdelay[] = {
	{ 1,		CR1 },
	{ 2,		CR2 },
	{ 3,		CR3 },
	{ 83,		CR1 },
	{ 166,		CR2 },
	{ 0,		CR3 },
};

struct delayval nldelay[] = {
	{ 1,		NL1 },		/* special, calculated */
	{ 2,		NL2 },
	{ 3,		NL3 },
	{ 100,		NL2 },
	{ 0,		NL3 },
};

struct delayval	bsdelay[] = {
	{ 1,		BS1 },
	{ 0,		0 },
};

struct delayval	ffdelay[] = {
	{ 1,		FF1 },
	{ 1750,		FF1 },
	{ 0,		FF1 },
};

struct delayval	tbdelay[] = {
	{ 1,		 TAB1 },
	{ 2,		 TAB2 },
	{ 3,		XTABS },	/* this is expand tabs */
	{ 100,		 TAB1 },
	{ 0,		 TAB2 },
};

int
delaybits()
{
	register int f;

	f  = adelay(CD, crdelay);
	f |= adelay(ND, nldelay);
	f |= adelay(FD, ffdelay);
	f |= adelay(TD, tbdelay);
	f |= adelay(BD, bsdelay);
	return (f);
}

int
adelay(ms, dp)
	register ms;
	register struct delayval *dp;
{
	if (ms == 0)
		return (0);
	while (dp->delay && ms > dp->delay)
		dp++;
	return (dp->bits);
}

char	editedhost[32];

void
edithost(pat)
	register char *pat;
{
	register char *host = HN;
	register char *res = editedhost;

	if (!pat)
		pat = "";
	while (*pat) {
		switch (*pat) {

		case '#':
			if (*host)
				host++;
			break;

		case '@':
			if (*host)
				*res++ = *host++;
			break;

		default:
			*res++ = *pat;
			break;

		}
		if (res == &editedhost[sizeof editedhost - 1]) {
			*res = '\0';
			return;
		}
		pat++;
	}
	if (*host)
		strncpy(res, host, sizeof editedhost - (res - editedhost) - 1);
	else
		*res = '\0';
	editedhost[sizeof editedhost - 1] = '\0';
}

struct speedtab {
	int	speed;
	int	uxname;
} speedtab[] = {
	{ 50,	  B50 },
	{ 75,	  B75 },
	{ 110,	 B110 },
	{ 134,	 B134 },
	{ 150,	 B150 },
	{ 200,	 B200 },
	{ 300,	 B300 },
	{ 600,	 B600 },
	{ 1200,	B1200 },
	{ 1800,	B1800 },
	{ 2400,	B2400 },
	{ 4800,	B4800 },
	{ 9600,	B9600 },
	{ 19200, EXTA },
	{ 19,	 EXTA },	/* for people who say 19.2K */
	{ 38400, EXTB },
	{ 38,	 EXTB },
	{ 7200,	 EXTB },	/* alternative */
	{ 0 }
};

int
speed(val)
	int val;
{
	register struct speedtab *sp;

	if (val <= 15)
		return (val);

	for (sp = speedtab; sp->speed; sp++)
		if (sp->speed == val)
			return (sp->uxname);
	
	return (B300);		/* default in impossible cases */
}

void
makeenv(env)
	char *env[];
{
	static char termbuf[128] = "TERM=";
	register char *p, *q;
	register char **ep;

	ep = env;
	if (TT && *TT) {
		strcat(termbuf, TT);
		*ep++ = termbuf;
	}
	if (p = EV) {
		q = p;
		while (q = strchr(q, ',')) {
			*q++ = '\0';
			*ep++ = p;
			p = q;
		}
		if (*p)
			*ep++ = p;
	}
	*ep = (char *)0;
}

/*
 * This speed select mechanism is written for the Develcon DATASWITCH.
 * The Develcon sends a string of the form "B{speed}\n" at a predefined
 * baud rate. This string indicates the user's actual speed.
 * The routine below returns the terminal type mapped from derived speed.
 */
struct	portselect {
	char	*ps_baud;
	char	*ps_type;
} portspeeds[] = {
	{ "B110",	"std.110" },
	{ "B134",	"std.134" },
	{ "B150",	"std.150" },
	{ "B300",	"std.300" },
	{ "B600",	"std.600" },
	{ "B1200",	"std.1200" },
	{ "B2400",	"std.2400" },
	{ "B4800",	"std.4800" },
	{ "B9600",	"std.9600" },
	{ "B19200",	"std.19200" },
	{ 0 }
};

char *
portselector()
{
	char c, baud[20], *type = "default";
	register struct portselect *ps;
	int len;

	alarm(5*60);
	for (len = 0; len < sizeof (baud) - 1; len++) {
		if (read(STDIN_FILENO, &c, 1) <= 0)
			break;
		c &= 0177;
		if (c == '\n' || c == '\r')
			break;
		if (c == 'B')
			len = 0;	/* in case of leading garbage */
		baud[len] = c;
	}
	baud[len] = '\0';
	for (ps = portspeeds; ps->ps_baud; ps++)
		if (strcmp(ps->ps_baud, baud) == 0) {
			type = ps->ps_type;
			break;
		}
	sleep(2);	/* wait for connection to complete */
	return (type);
}

/*
 * This auto-baud speed select mechanism is written for the Micom 600
 * portselector. Selection is done by looking at how the character '\r'
 * is garbled at the different speeds.
 */
#include <sys/time.h>

char *
autobaud()
{
	int rfds;
	struct timeval timeout;
	char c, *type = "9600-baud";
	int null = 0;

	ioctl(0, TIOCFLUSH, &null);
	rfds = 1 << 0;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	if (select(32, (fd_set *)&rfds, (fd_set *)NULL,
	    (fd_set *)NULL, &timeout) <= 0)
		return (type);
	if (read(STDIN_FILENO, &c, sizeof(char)) != sizeof(char))
		return (type);
	timeout.tv_sec = 0;
	timeout.tv_usec = 20;
	(void) select(32, (fd_set *)NULL, (fd_set *)NULL,
	    (fd_set *)NULL, &timeout);
	ioctl(0, TIOCFLUSH, &null);
	switch (c & 0377) {

	case 0200:		/* 300-baud */
		type = "300-baud";
		break;

	case 0346:		/* 1200-baud */
		type = "1200-baud";
		break;

	case  015:		/* 2400-baud */
	case 0215:
		type = "2400-baud";
		break;

	default:		/* 4800-baud */
		type = "4800-baud";
		break;

	case 0377:		/* 9600-baud */
		type = "9600-baud";
		break;
	}
	return (type);
}
