/*	vcmd.h	3.3	6/7/80	*/

#define	VPRINT		0100
#define	VPLOT		0200
#define	VPRINTPLOT	0400

#define	VGETSTATE	(('v'<<8)|0)
#define	VSETSTATE	(('v'<<8)|1)
