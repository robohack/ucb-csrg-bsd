/*
 * Copyright (c) 1991, 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * This software was developed by the Computer Systems Engineering group
 * at Lawrence Berkeley Laboratory under DARPA contract BG 91-66 and
 * contributed to Berkeley.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)bsd_audioio.h	7.1 (Berkeley) 7/13/92
 *
 * from: $Header: bsd_audioio.h,v 1.4 92/07/13 00:31:22 torek Exp $ (LBL)
 */

#ifndef _BSD_AUDIOIO_H_
#define _BSD_AUDIOIO_H_

/*
 * /dev/audio ioctls.  needs comments!
 */
#define AUDIO_MIN_GAIN 0
#define AUDIO_MAX_GAIN 255

#define AUDIO_ENCODING_ULAW 1
#define AUDIO_ENCODING_ALAW 2

struct audio_prinfo {
	u_int	sample_rate;
	u_int	channels;
	u_int	precision;
	u_int	encoding;
	u_int	gain;
	u_int	port;
	u_int	samples;

	u_char	pause;
	u_char	error;
	u_char	waiting;
	u_char	open;

	/* BSD extensions */
	u_long	seek;
};

struct audio_info {
	struct	audio_prinfo play;
	struct	audio_prinfo record;
	u_int	monitor_gain;
	/* BSD extensions */
	u_int	blocksize;	/* input blocking threshold */
	u_int	hiwat;		/* output high water mark */
	u_int	lowat;		/* output low water mark */
};
typedef struct audio_info audio_info_t;

#define AUDIO_INITINFO(p)\
	(void)memset((void *)(p), 0xff, sizeof(struct audio_info))

#if (defined(sun) || defined(ibm032)) && !defined(__GNUC__)
#define AUDIO_GETINFO	_IOR(A, 1, struct audio_info)
#define AUDIO_SETINFO	_IOWR(A, 2, struct audio_info)
#define AUDIO_DRAIN	_IO(A, 3)
#define AUDIO_FLUSH	_IO(A, 4)
#define AUDIO_WSEEK	_IOR(A, 5, u_long)
#define AUDIO_RERROR	_IOR(A, 6, int)
#define AUDIO_GETMAP	_IOR(A, 20, struct mapreg)
#define	AUDIO_SETMAP	_IOW(A, 21, struct mapreg)
#else
#define AUDIO_GETINFO	_IOR('A', 1, struct audio_info)
#define AUDIO_SETINFO	_IOWR('A', 2, struct audio_info)
#define AUDIO_DRAIN	_IO('A', 3)
#define AUDIO_FLUSH	_IO('A', 4)
#define AUDIO_WSEEK	_IOR('A', 5, u_long)
#define AUDIO_RERROR	_IOR('A', 6, int)
#define AUDIO_GETMAP	_IOR('A', 20, struct mapreg)
#define	AUDIO_SETMAP	_IOW('A', 21, struct mapreg)
#endif

#define AUDIO_SPEAKER   	1
#define AUDIO_HEADPHONE		2

/*
 * Low level interface.
 */
struct mapreg {
	u_short	mr_x[8];
	u_short	mr_r[8];
	u_short	mr_gx;
	u_short	mr_gr;
	u_short	mr_ger;
	u_short	mr_stgr;
	u_short	mr_ftgr;
	u_short	mr_atgr;
	u_char	mr_mmr1;
	u_char	mr_mmr2;
};

#endif /* _BSD_AUDIOIO_H_ */
