/*  VPLOT: version 4.1				updated 3/10/83
 *
 *  Reads standard graphics input and produces a plot on the
 *  Varian or Versatec
 *  -- creates in /usr/tmp a raster file of 500 1K-blocks.
 */
#include <stdio.h>
#include <signal.h>
#include <vfont.h>

#define LPR "/usr/ucb/lpr"
#define VAX		/* Machine Flag (don't simulate VM on vax) */

#ifdef VAX
#define NB	1024		/* Number of blocks in virtual memory */
#else
#define	NB	88		/* Number of blocks kept in memory */
#endif
#define BSIZ	512		/* Size of blocks */
#define LOGBSIZ	9		/* log base 2 of BSIZ */

#define	mapx(x)	((DevRange*((x)-botx)/del)+centx)
#define	mapy(y)	((DevRange*(del-(y)+boty)/del)-centy)
#define SOLID -1
#define DOTTED 014
#define SHORTDASHED 034
#define DOTDASHED 054
#define LONGDASHED 074

char *Sid = "@(#)\t3/10/83";

int	linmod	= SOLID;
int	done1;
char	chrtab[][16];
char	blocks[NB][BSIZ];
int	lastx;
int	lasty;
int	radius, startx, starty, endx, endy;
double	topx;
double	topy;
double	botx;
double	boty;
int	centx = 0;
int	centy = 0;
double	delx;
double	dely;
double	del;

int	warned = 0;	/* Indicates whether the warning message about
			 * unimplemented routines has been printed */

#ifdef VAX
char	dirty[NB];		/* marks if a block has been written into */
#else
struct	buf {
	int	bno;
	char	*block;
} bufs[NB];
#endif

FILE	*infile;
int	fd;
char	picture[] = "/usr/tmp/rastAXXXXXX";
int	run = 13;		/* index of 'a' in picture[] */
int	DevRange = 1536;	/* output array size (square) in pixels */
int	BytesPerLine = 264;	/* Bytes per raster line (physical) */
int	lparg = 7;		/* index into lpargs */

char	*lpargs[50] = { "lpr", "-Pvarian", "-v", "-s", "-r", "-J", "vplot" };

/* variables for used to print from font file */
int	fontSet = 0;		/* Has the font file been read */
struct	header header;
struct	dispatch dispatch[256];
char	*bits;
char	*fontFile = "/usr/lib/vfont/R.6";

main(argc, argv)
int argc;
char **argv;
{
	extern int cleanup();
	extern char *malloc();
	register i, j;
	register char *arg;
	int again;

	infile = stdin;
	while (argc > 1 && argv[1][0] == '-') {
		argc--;
		arg = *++argv;
		switch (*++arg) {
		case 'W':
			DevRange = 2047;
			BytesPerLine = 880;
			lpargs[1] = "-Pversatec";
			break;
		case 'V':
			DevRange = 1536;
			BytesPerLine = 264;
			lpargs[1] = "-Pvarian";
			break;
		case 'b':
			if (argc-- > 1)
				lpargs[lparg-1] = *++argv;
			break;
		default:
			fprintf(stderr, "vplot: %s option unknown\n", *argv);
			break;
		}
	}
	if (argc > 1) {
		if ((infile = fopen(*++argv, "r")) == NULL) {
			perror(*argv);
			cleanup();
		}
	}

	/* init constants for scaling */
	topx = topy = DevRange;
	botx = boty = 0;
	delx = dely = del = DevRange;
	centx = (DevRange - mapx(topx))/2;
	centy = mapy(topy)/2;
	signal(SIGTERM, cleanup);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, cleanup);
	mktemp(picture);
	do {
		if ((fd = creat(picture, 0666)) < 0) {
			fprintf(stderr, "can't create %s\n", picture);
			cleanup();
		}
#ifndef VAX
		close(fd);
		if ((fd = open(picture, 2)) < 0) {
			fprintf(stderr, "can't reopen %s\n", picture);
			cleanup();
		}
#endif
		i = strlen(picture) + 1;
		if ((arg = malloc(i)) == NULL) {
			fprintf(stderr, "ran out of memory\n");
			cleanup();
		}
		strcpy(arg, picture);
		lpargs[lparg++] = arg;
		picture[run]++;
		for (i=0; i<NB; i++) {
#ifdef VAX
			dirty[i] = 0;
			for (j=0; j<BSIZ; ++j)
				blocks[i][j] = 0;
#else
			bufs[i].bno = -1;
			bufs[i.block = blocks[i];
#endif
		}
#ifdef NOHOLES
		/* clear the entire file */
		for (i=0; i<BSIZ; i++)
			blocks[0][i] = '\0';
		for (i=0; i<1024; i++)
			write(fd, blocks[0], BSIZ);
#endif

		again = getpict();

		for (i=0; i<NB; i++) {
#ifdef VAX
			if (dirty[i]) {		/* write out non-zero blocks */
				zseek(fd, i);
				write(fd, blocks[i], BSIZ);
			}
#else
			if (bufs[i].bno != -1) {
				zseek(fd, bufs[i].bno);
				write(fd, bufs[i].blocks[i], BSIZ);
			}
#endif
		}
		close(fd);
	} while (again);
	lpargs[lparg] = 0;
	execv(LPR, lpargs);
	fprintf(stderr, "can't exec %s\n", LPR);
	cleanup();
}

getpict()
{
	register x1, y1;

	for (;;) switch (x1 = getc(infile)) {

	case '\n':
		continue;

	case 's':
		botx = getinteger(infile);
		boty = getinteger(infile);
		topx = getinteger(infile);
		topy = getinteger(infile);
		delx = topx-botx;
		dely = topy-boty;
		if (dely/delx > 1536./2048.)
			del = dely;
		else
			del = delx;
		centx = 0;
		centx = (DevRange - mapx(topx))/2;
		centy = 0;
		centy = mapy(topy) / 2;
		continue;

	case 'b':
		x1 = getc(infile);
		continue;

	case 'l':
		done1 |= 01;
		x1 = mapx(getinteger(infile));
		y1 = mapy(getinteger(infile));
		lastx = mapx(getinteger(infile));
		lasty = mapy(getinteger(infile));
		line(x1, y1, lastx, lasty);
		continue;

	case 'c':
		x1 = mapx(getinteger(infile));
		y1 = mapy(getinteger(infile));
		radius = mapx(getinteger(infile));
		if (!warned) {
			fprintf(stderr,"Circles are Implemented\n");
			warned++;
		}
		circle(x1, y1, radius);
		continue;
		
	case 'a':
		x1 = mapx(getinteger(infile));
		y1 = mapy(getinteger(infile));
		startx = mapx(getinteger(infile));
		starty = mapy(getinteger(infile));
		endx = mapx(getinteger(infile));
		endy = mapy(getinteger(infile));
		if (!warned) {
			fprintf(stderr,"Circles and Arcs are unimplemented\n");
			warned++;
		}
		continue;

	case 'm':
		lastx = mapx(getinteger(infile));
		lasty = mapy(getinteger(infile));
		continue;

	case 't':
		lastx = lastx - 6;
		lasty = lasty + 6;
		done1 |= 01;
		while ((x1 = getc(infile)) != '\n')
			plotch(x1);
		continue;

	case 'e':
		if (done1)
			return(1);
		continue;

	case 'p':
		done1 |= 01;
		lastx = mapx(getinteger(infile));
		lasty = mapy(getinteger(infile));
		point(lastx, lasty);
		point(lastx+1, lasty);
		point(lastx, lasty+1);
		point(lastx+1, lasty+1);
		continue;

	case 'n':
		done1 |= 01;
		x1 = mapx(getinteger(infile));
		y1 = mapy(getinteger(infile));
		line(lastx, lasty, x1, y1);
		lastx = x1;
		lasty = y1;
		continue;

	case 'f':
		getinteger(infile);
		getc(infile);
		switch (getc(infile)) {
		case 't':
			linmod = DOTTED;
			break;
		default:
		case 'i':
			linmod = SOLID;
			break;
		case 'g':
			linmod = LONGDASHED;
			break;
		case 'r':
			linmod = SHORTDASHED;
			break;
		case 'd':
			linmod = DOTDASHED;
			break;
		}
		while ((x1 = getc(infile)) != '\n')
			if (x1 == EOF)
				return(0);
		continue;

	case 'd':
		getinteger(infile);
		getinteger(infile);
		getinteger(infile);
		x1 = getinteger(infile);
		while (--x1 >= 0)
			getinteger(infile);
		continue;

	case 0:		/* ignore null characters */
		continue;

	case 255:
	case EOF:
		return(0);

	default:
		fprintf(stderr, "Input format error %c(%o)\n",x1,x1);
		cleanup();
	}
}

plotch(ch)
char ch;
{
	register int i,j,k;
	register char *ptr,c;
	int nbytes;

	if (!fontSet)
		InitFont();	/* Read font if not already read */

	ptr = bits + dispatch[ch].addr;

	for (i = dispatch[ch].up; i > -dispatch[ch].down; --i) {
		nbytes = (dispatch[ch].right + dispatch[ch].left + 7)/8;
		for (j = 0; j < nbytes; j++) {
			c = *ptr++;
			for (k = 7; k >= 0; k--)
				if ((c >> k) & 1)
					point(lastx+7-k+j*8-dispatch[ch].left, lasty-i);
		}
	}
	if (ch != ' ')
		lastx += dispatch[ch].width;
	else
		lastx += dispatch['a'].width;
}

InitFont()
{
	char *s;
	int fonts;
	int i;

	fontSet = 1;
	/* Get the font file */
	s = fontFile;
	if ((fonts = open(s, 0)) == -1) {
		perror(s);
		fprintf(stderr, "Can't get font file");
		cleanup();
	}
	/* Get the header and check magic number */
	if (read(fonts, &header, sizeof(header)) != sizeof(header)) {
		perror(s);
		fprintf(stderr, "Bad read in font file");
		cleanup();
	}
	if (header.magic != 0436) {
		fprintf(stderr,"Bad magic numer in font file");
		cleanup();
	}
	/* Get dispatches */
	if (read(fonts, dispatch, sizeof(dispatch)) != sizeof(dispatch)) {
		perror(s);
		fprintf(stderr, "Bad read in font file");
		cleanup();
	}
	/* Allocate space for bit map and read in bits */
	bits = (char *) malloc(header.size);
	if (read(fonts, bits, header.size) != header.size) {
		perror(s);
		fprintf(stderr,"Can't read bit map in font file");
		cleanup();
	}
	/* Close font file */
	if (close(fonts) != 0) {
		perror(s);
		fprintf(stderr,"Can't close font file");
		cleanup();
	}
}

line(x0, y0, x1, y1)
register x0, y0;
{
	int dx, dy;
	int xinc, yinc;
	register res1;
	int res2;
	int slope;

	xinc = 1;
	yinc = 1;
	if ((dx = x1-x0) < 0) {
		xinc = -1;
		dx = -dx;
	}
	if ((dy = y1-y0) < 0) {
		yinc = -1;
		dy = -dy;
	}
	slope = xinc*yinc;
	res1 = 0;
	res2 = 0;
	if (dx >= dy) while (x0 != x1) {
		if ((x0+slope*y0) & linmod)
			point(x0, y0);
		if (res1 > res2) {
			res2 += dx - res1;
			res1 = 0;
			y0 += yinc;
		}
		res1 += dy;
		x0 += xinc;
	} else while (y0 != y1) {
		if ((x0+slope*y0) & linmod)
		point(x0, y0);
		if (res1 > res2) {
			res2 += dy - res1;
			res1 = 0;
			x0 += xinc;
		}
		res1 += dx;
		y0 += yinc;
	}
	if ((x1+slope*y1) & linmod)
		point(x1, y1);
}

#define labs(a) (a >= 0 ? a : -a)

circle(x,y,c)
{
	register dx, dy;
	long ep;
	int de;

	dx = 0;
	ep = 0;
	for (dy=c; dy>=dx; dy--) {
		for (;;) {
			point(x+dx, y+dy);
			point(x-dx, y+dy);
			point(x+dx, y-dy);
			point(x-dx, y-dy);
			point(x+dy, y+dx);
			point(x-dy, y+dx);
			point(x+dy, y-dx);
			point(x-dy, y-dx);
			ep += 2*dx + 1;
			de = -2*dy + 1;
			dx++;
			if (labs(ep) >= labs(ep+de)) {
				ep += de;
				break;
			}
		}
	}
}

/*
 * Points should be in the range 0 <= x (or y) <= DevRange.
 * The origin is the top left-hand corner with increasing x towards the
 * right and increasing y going down.
 */
point(x, y)
register int x, y;
{
	register unsigned bno, byte;

	byte = y * BytesPerLine + (x >> 3);
	bno = byte >> LOGBSIZ;
	byte &= BSIZ - 1;
	if (bno >= 1024)
		return;
#ifndef VAX
	if (bno != bufs[0].bno)
		getblk(bno);
	bufs[0].block[byte] |= 1 << (7 - (x & 07));
#else
	blocks[bno][byte] |= 1 << (7 - (x & 07));
	dirty[bno] = 1;
#endif
}

#ifndef VAX
getblk(b)
register b;
{
	register struct buf *bp1;
	register char *tp;

loop:
	for (bp1 = bufs; bp1 < &bufs[NB]; bp1++) {
		if (bp1->bno == b || bp1->bno == -1) {
			tp = bp1->block;
			while (bp1 > bufs) {
				bp1->bno = (bp1-1)->bno;
				bp1->block = (bp1-1)->block;
				bp1--;
			}
			bp1->bno = b;
			bp1->block = tp;
			return;
		}
	}
	zseek(fd, bufs[NB-1].bno);
	write(fd, bufs[NB-1].block, BSIZ);
	zseek(fd, b);
	read(fd, bufs[NB-1].block, BSIZ);
	bufs[NB-1].bno = b;
	goto loop;
}
#endif

cleanup()
{
	while (picture[run] != 'a') {
		unlink(picture);
		picture[run]--;
	}
	exit(1);
}

zseek(a, b)
{
	return(lseek(a, (long)b*BSIZ, 0));
}

getinteger(f)
FILE *f;
{
	register int low, high, result;

	low = getc(f);
	high = getc(f);
	result = ((high << 8) | low);
	if (high > 127)
		result |= ~0xffff;
	return(result);
}
