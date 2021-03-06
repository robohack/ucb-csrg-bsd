/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)types.h	8.4 (Berkeley) 1/9/95
 */

#ifndef	_MACHTYPES_H_
#define	_MACHTYPES_H_

#if !defined(_ANSI_SOURCE) && !defined(_POSIX_SOURCE)
typedef struct _physadr {
	short r[1];
} *physadr;

typedef struct label_t {			/* Consistent with HP-UX. */
	int val[15];
} label_t;
#endif

typedef	unsigned long	vm_offset_t;
typedef	unsigned long	vm_size_t;

/*
 * Basic integral types.  Omit the typedef if
 * not possible for a machine/compiler combination.
 */
typedef	__signed char		   int8_t;
typedef	unsigned char		 u_int8_t;
typedef	short			  int16_t;
typedef	unsigned short		u_int16_t;
typedef	int			  int32_t;
typedef	unsigned int		u_int32_t;
typedef	long long		  int64_t;
typedef	unsigned long long	u_int64_t;

typedef	int32_t			register_t;

#endif	/* _MACHTYPES_H_ */
