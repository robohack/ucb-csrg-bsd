/*	tp.h	1.2	82/05/15	*/

/*
 * Constants and structures for DECnet Transport layer,
 * per Transport Funtional Specification, Version 1.3.0, October 1980.
 */

#define	TP_VERSION	'1'
#define	TP_ECO		'3'
#define	TP_USERECO	'0'

/*
 * Transport layer message types,
 * low 3 bits of first byte of message.
 */
#define	TP_INIT		1	/* initialization message */
#define	TP_RH		2	/* route header */
#define	TP_VERIF	3	/* verification message */
#define	TP_TEST		5	/* hello and test message */
#define	TP_ROUTE	7	/* routing message */

/*
 * Initialization Message
 */
struct tpin {
	char	tpin_ctlflg;		/* control flag (message type) */
	d_short	tpin_srcnode;		/* source node */
	char	tpin_tiinfo;		/* see below */
	d_short	tpin_blksize;		/* max data link layer block size */
	char	tpin_ver[3];		/* version number (1.3.0) */
	char	tpin_res;		/* reserved, must contain 0 */
};

/* tiinfo */
#define	TPIN_NTYPE	03	/* node type */
#define	TPINNT_RT	02	/* routing node */
#define	TPINNT_NRT	03	/* non-routing node */
#define	TPIN_VERIF	04	/* transport verification message required */

/*
 * Route Header
 *
 * A route header is attached to all NSP segments.
 */
struct tprh {
	char	tprh_rtflg;		/* flags, see below */
	d_short	tprh_dstnode;		/* destination node */
	d_short	tprh_srcnode;		/* source node */
	char	tprh_forward;		/* visit count (low 6 bits) */
};

/* route flags */
#define	TPRF_EV		0100	/* routing evolution: 0 = P3, 1 = P2 ASCII */
#define	TPRF_RTS	0020	/* return to sender: 1 = being returned */
#define	TPRF_RQR	0010	/* return to sender request: 1 = please do */

/*
 * Verification Message
 */
struct tpvf {
	char	tpvf_ctlflg;		/* control flag (message type) */
	d_short	tpvf_srcnode;		/* source node */
	char	tpvf_cnt;		/* size of next field */
	char	tpvf_fcnval[64];	/* function value (???) */
};

/*
 * Hello and Test Message
 */
struct tpht {
	char	tpht_ctlflg;		/* control flag (message type) */
	d_short	tpht_srcnode;		/* source node */
	char	tpht_cnt;		/* size of data field */
	char	tpht_data[1];		/* test data, each byte must be 0252 */
};

/*
 * Routing Message
 */
struct tprt {
	char	tprt_ctlflg;		/* control flag (message type) */
	d_short	tprt_srcnode;		/* source node */
	d_short	tprt_rtginfo[1];	/* routing information: */
					/* bits 14-10, hops; bits 9-0, cost */
					/* this field is variable length */
	d_short	tprt_checksum;		/* checksum; last field of message */
};
