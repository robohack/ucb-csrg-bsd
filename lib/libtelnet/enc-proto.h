/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)enc-proto.h	5.3 (Berkeley) 12/18/92
 */

/*
 * Copyright (C) 1990 by the Massachusetts Institute of Technology
 *
 * Export of this software from the United States of America is assumed
 * to require a specific license from the United States Government.
 * It is the responsibility of any person or organization contemplating
 * export to obtain such a license before exporting.
 *
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 */
#if	!defined(P)
#ifdef	__STDC__
#define	P(x)	x
#else
#define	P(x)	()
#endif
#endif

#if	defined(ENCRYPTION)
void encrypt_init P((char *, int));
Encryptions *findencryption P((int));
void encrypt_send_supprt P((void));
void encrypt_auto P((int));
void decrypt_auto P((int));
void encrypt_is P((unsigned char *, int));
void encrypt_reply P((unsigned char *, int));
void encrypt_start_input P((int));
void encrypt_session_key P((Session_Key *, int));
void encrypt_end_input P((void));
void encrypt_start_output P((int));
void encrypt_end_output P((void));
void encrypt_send_request_start P((void));
void encrypt_send_request_end P((void));
void encrypt_send_end P((void));
void encrypt_wait P((void));
void encrypt_send_support P((void));
void encrypt_send_keyid P((int, unsigned char *, int, int));
int net_write P((unsigned char *, int));

#ifdef	TELENTD
void encrypt_wait P((void));
#else
int encrypt_cmd P((int, char **));
void encrypt_display P((void));
#endif

void krbdes_encrypt P((unsigned char *, int));
int krbdes_decrypt P((int));
int krbdes_is P((unsigned char *, int));
int krbdes_reply P((unsigned char *, int));
void krbdes_init P((int));
int krbdes_start P((int, int));
void krbdes_session P((Session_Key *, int));
void krbdes_printsub P((unsigned char *, int, unsigned char *, int));

void cfb64_encrypt P((unsigned char *, int));
int cfb64_decrypt P((int));
void cfb64_init P((int));
int cfb64_start P((int, int));
int cfb64_is P((unsigned char *, int));
int cfb64_reply P((unsigned char *, int));
void cfb64_session P((Session_Key *, int));
int cfb64_keyid P((int, unsigned char *, int *));
void cfb64_printsub P((unsigned char *, int, unsigned char *, int));

void ofb64_encrypt P((unsigned char *, int));
int ofb64_decrypt P((int));
void ofb64_init P((int));
int ofb64_start P((int, int));
int ofb64_is P((unsigned char *, int));
int ofb64_reply P((unsigned char *, int));
void ofb64_session P((Session_Key *, int));
int ofb64_keyid P((int, unsigned char *, int *));
void ofb64_printsub P((unsigned char *, int, unsigned char *, int));

int  des_new_random_key P((Block));
void des_set_random_generator_seed P((Block));
void des_key_sched P((Block, Schedule));
void des_ecb_encrypt P((Block, Block, Schedule, int));
int  des_string_to_key P((char *, Block));
#endif
