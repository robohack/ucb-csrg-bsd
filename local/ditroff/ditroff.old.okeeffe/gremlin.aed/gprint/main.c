/* gprint.c-
 *
 * Copyright -C- 1982 Barry S. Roitblat
 *
 *	This file contains the main and file system dependent routines
 * for producing hard copy from gremlin files.  It is extensively modified
 * from the vplot source.
 */

#include "gprint.h"
#include "grem2.h"
#include <signal.h>
#include <vfont.h>

#define LPR "/usr/ucb/lpr"

extern char *mktemp();
extern char *malloc();
extern char *rindex();

/* imports */
extern HGtline(), HGArc(), HGPutText(), HGMove(), HGSetFont();
extern HGSetBrush(), HGInitFont(), HGPrintElt();
extern int style[], thick[];
extern char *tfont[], *tsize[];
 
/* database imports */

extern ELT *DBInit(), *DBRead();
extern POINT *PTInit(), *PTMakePoint();

int	linethickness = 0;	/* brush styles */
int	linmod = SOLID;
char	chrtab[][16];
char	*obuf;		/* output buffer DevRange x DevRange/8 */
int	bufsize;	/* output buffer size */
int	lastx;
int	lasty;
int	angle, startx, starty, endx, endy;
double	scale = 4.0;	/* Variables used to map gremlin screen */
double	topx;		/* coordinates into output device coordinates */
double	topy;
double	botx;
double	boty;
int	centx = 0;
int	centy = 0;
double	delx;
double	dely;
double	del;

FILE	*pfp = stdout;		/* file descriptor of current picture */
char	picture[] = "/usr/tmp/rastAXXXXXX";
int	run = 13;		/* index of 'a' in picture[] */
int	DevRange = 1536;	/* Bits per line for output array */
int	DevRange8 = 1536/8;	/* Bytes per line for output array */
int	BytesPerLine = 264;	/* Bytes per raster line (different from range
				   due to non-square paper). */
char	device = 'V';		/* default device */
int	lparg = 6;		/* index into lpargs */

char	*lpargs[50] = { "lpr", "-Pvarian", "-v", "-s", "-r", "-J", };

/* variables used to print from font file */
int	Orientation;
int	cfont = 0;
int	csize = 0;
struct	header header;
struct	dispatch dispatch[256];
char	*bits = NULL;
char	*fontdir = "/usr/lib/vfont/";

main(argc, argv)
int argc;
char *argv[];
{
	FILE *fp, *fopen();
	ELT *PICTURE, *e;
	POINT *p1, pos;
	char *file[50], string[10], *arg;
	char c, string1[50], string2[50], string3[50], string4[50], 
		string5[50], string6[50], string7[50], string8[50]; 
	extern int cleanup();
	float mult;
	int WriteRaster = FALSE;
	register char *cp1, *cp2;
	register int i, k;
	int brsh, gfil = 0;

	/* Parse the command line. */

	argc--;
	argv++;
	while (argc--) {
		if (*(arg = *argv++) != '-')
			file[gfil++] = arg;
		else switch (*++arg) {
		case 'W':	/* Print to wide (versatec) device */
			device = 'W';
			DevRange = 2048;
			DevRange8 = 2048/8;
			BytesPerLine = 880;
			lpargs[1] = "-Pversatec";
			break;
		case 'V':	/* Print to narrow (varian) device */
			device = 'V';
			DevRange = 1536;
			DevRange8 = 1536/8;
			BytesPerLine = 264;
			lpargs[1] = "-Pvarian";
			break;
		case '1':	/* select size 1 */
			if (*++arg == '\0' && argc--)
				arg = *argv++;
			tsize[0] = arg;
			break;
		case '2':	/* select size 2 */
			if (*++arg == '\0' && argc--)
				arg = *argv++;
			tsize[1] = arg;
			break;
		case '3':	/* select size 3 */
			if (*++arg == '\0' && argc--)
				arg = *argv++;
			tsize[2] = arg;
			break;
		case '4':	/* select size 4 */
			if (*++arg == '\0' && argc--)
				arg = *argv++;
			tsize[3] = arg;
			break;
		case 'R':	/* select Roman font */
			if (*++arg == '\0' && argc--)
				arg = *argv++;
			tfont[0] = arg;
			break;
		case 'I':	/* select italics font */
			if (*++arg == '\0' && argc--)
				arg = *argv++;
			tfont[1] = arg;
			break;
		case 'B':	/* select bold font */
			if (*++arg == '\0' && argc--)
				arg = *argv++;
			tfont[2] = arg;
			break;
		case 'S':	/* select special font */
			if (*++arg == '\0' && argc--)
				arg = *argv++;
			tfont[3] = arg;
			break;
		case 'N':	/* select narrow brush width */
			if (*++arg == '\0' && argc--)
				arg = *argv++;
			(void) sscanf(arg, "%d", &brsh);
			thick[0] = thick[1] = thick[3] = thick[4] = brsh;
			break;
		case 'T':	/* select thick brush width */
			if (*++arg == '\0' && argc--)
				arg = *argv++;
			(void) sscanf(arg, "%d", &brsh);
			thick[2] = brsh;
			break;
		case 'M':	/* select medium brush width */
			if (*++arg == '\0' && argc--)
				arg = *argv++;
			(void) sscanf(arg, "%d", &brsh);
			thick[5] = brsh;
			break;
		case 't':	/* send raster to standard output */
			WriteRaster = TRUE;
			break;
		case 'x':	/* select scale */
			if (*++arg == '\0' && argc--)
				arg = *argv++;
			sscanf(arg,"%f", &mult);
			scale *= mult;
			break;
		case 'p':	/* prompt for font and size parameters */
			printf("Roman font name? (%s): ", tfont[0]);
			gets(string1);
			if (*string1 != '\0') tfont[0] = string1;
			printf("Italic font name? (%s): ", tfont[1]);
			gets(string2);
			if (*string2 != '\0') tfont[1] = string2;
			printf("Bold font name? (%s): ", tfont[2]);
			gets(string3);
			if (*string3 != '\0') tfont[2] = string3;
			printf("Special font name? (%s): ", tfont[3]);
			gets(string4);
			if (*string4 != '\0') tfont[3] = string4;
			printf("font size 1? (%s): ", tsize[0]);
			gets(string5);
			if (*string5 != '\0') tsize[0] = string5;
			printf("font size 2? (%s): ", tsize[1]);
			gets(string6);
			if (*string6 != '\0') tsize[1] = string6;
			printf("font size 3? (%s): ", tsize[2]);
			gets(string7);
			if (*string7 != '\0') tsize[2] = string7;
			printf("font size 4? (%s): ", tsize[3]);
			gets(string8);
			if (*string8 != '\0') tsize[3] = string8;
			printf("narrow brush size? (%d): ", thick[0]);
			gets(string);
			if (*string != '\0') {
				sscanf(string, "%d", &brsh);
				thick[0] = thick[1] = thick[3] = thick[4] = brsh;
			}
			printf("medium brush size? (%d): ", thick[5]);
			gets(string);
			if (*string != '\0') {
				sscanf(string, "%d", &brsh);
				thick[5] = brsh;
			}
			printf("thick brush size? (%d): ", thick[2]);
			gets(string);
			if (*string != '\0') {
				sscanf(string, "%d", &brsh);
				thick[2] = brsh;
			}
			break;
		default:
			fprintf(stderr, "unknown switch: %c", *arg);
		}
	}

	/* init constants for scaling */
	topx = topy = DevRange;
	botx = boty = 0;
	delx = dely = del = DevRange;
	centx = (DevRange - mapx(topx/scale))/2;
	centy = mapy(topy/scale)/2;
	signal(SIGTERM, cleanup);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, cleanup);
	mktemp(picture);
	if ((obuf = malloc(bufsize = DevRange * DevRange8)) == NULL) {
		fprintf(stderr, "gprint: ran out of memory for output buffer\n");
		exit(1);
	}
	if (gfil == 0) {	/* no filename, use standard input */
		file[0] = NULL;
		gfil++;
	}
	for (k=0; k<gfil; k++) {
		if (file[k] != NULL) {
			if ((fp = fopen(file[k], "r")) == NULL) {
				fprintf(stderr, "gprint: can't open %s", file[k]);
				continue;
			}
			if (k == 0) {
				if ((arg = rindex(file[k], '/')) == NULL)
					arg = file[k];
				else
					arg++;
				lpargs[lparg++] = arg;
			}
		} else {
			fp = stdin;
			lpargs[lparg++] = "gremlin";
		}
		/* read picture file */
		PICTURE = DBRead(fp, &Orientation, &pos);
		fclose(fp);
		if (DBNullelt(PICTURE))
			continue;

		if (!WriteRaster) {
			umask(0);
			if ((pfp = fopen(picture, "w")) == NULL) {
				fprintf(stderr, "gprint: can't create %s\n", picture);
				cleanup();
			}
		}
		i = strlen(picture) + 1;
		if ((arg = malloc(i)) == NULL) {
			fprintf(stderr, "gprint: ran out of memory\n");
			cleanup();
		}
		strcpy(arg, picture);
		lpargs[lparg++] = arg;
		picture[run]++;
		cp2 = &obuf[bufsize];
		for (cp1 = obuf; cp1 < cp2; )
			*cp1++ = 0;
		e = PICTURE;
		while (!DBNullelt(e)) {
			HGPrintElt(e);	/* traverse picture, printing elements */
			e = DBNextElt(e);
		}
		for (cp1 = obuf; cp1 < cp2; cp1 += DevRange8) {
			fwrite(cp1, sizeof(char), DevRange8, pfp);
			if (fseek(pfp, (long) BytesPerLine - DevRange8, 1) < 0)
				for (i = BytesPerLine - DevRange8; i--; )
					putc('\0', pfp);
		}
		if (!WriteRaster)
			fclose(pfp);
	}
	if (!WriteRaster) {
		lpargs[lparg] = 0;
		execv(LPR, lpargs);
		fprintf(stderr, "gprint: can't exec %s\n", LPR);
		cleanup();
	}
	exit(0);
}

cleanup()
{
	while (picture[run] != 'a') {
		unlink(picture);
		picture[run]--;
	}
	exit(1);
}

/*
 * Points should be in the range 0 <= x (or y) <= DevRange.
 * The origin is the top left-hand corner with increasing x towards the
 * right and increasing y going down.
 * The output array is DevRange x DevRange pixels.
 */
point(x, y)
register int x, y;
{
	register unsigned byte;

	byte = y * DevRange8 + (x >> 3);
	if (byte < bufsize)
		obuf[byte] |= 1 << (7 - (x & 07));
}
