/* public domain Simple, Minimalistic, UTF8 decoder and encoder library
 *	©2022 Yuichiro Nakada
 * http://www.ne.jp/asahi/maoyam/hp/UTF-8/
 * */

/***********************************************************************
 * File: utf8.h
 * 	(Extracted from
 *	   "$Id: utf8.h,v 1.2 2009/05/30 15:34:08 maoyam Exp maoyam $")
 *
 * Abstracts:
 *  This C header defines prototyping for the UTF-8 character decoder
 *  and encoder.
 *
 * Copyright (C) 2008 Maoyam Tokyo, Japan (mailto:maoyam@mail.goo.ne.jp)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **********************************************************************/


#ifndef _UTF8_H_ /* { */
#define _UTF8_H_
#ifdef __cplusplus
extern "C" {
#endif

/*
 * including the system definitions
 */
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iconv.h>
#include <errno.h>

typedef enum Sanitize_for_UTF8_e { /* the source is encoded in... */
	  Sanitize_for_UTF8_e_Zero			= 0
	, Sanitize_for_UTF8_e_EUC_JISX0213	/* "EUCJP-JISX0213" */
	, Sanitize_for_UTF8_e_EUCJP_OPEN	/* "EUCJP-OPEN" */
	, Sanitize_for_UTF8_e_EUCJP_WIN		/* "EUCJP-WIN" */
	, Sanitize_for_UTF8_e_EUCJP_MS		/* "EUCJP-MS" */
	, Sanitize_for_UTF8_e_EUCJP			/* "EUCJP" */
	, Sanitize_for_UTF8_e_SJIS_OPEN		/* "SJIS-OPEN" (-> "EUCJP-OPEN") */
	, Sanitize_for_UTF8_e_SJIS_WIN		/* "SJIS-WIN"  (-> "EUCJP_WIN") */
	, Sanitize_for_UTF8_e_SJIS			/* "SJIS"      (-> "SJIS") */
	, Sanitize_for_UTF8_e_N				/* number of the source encoding. */
	} Sanitize_for_UTF8_e;

/* translate `str_encoding' to iconv_open(3)'s encode identifier */
#define Sanitize_for_UTF8_e_To_iconv_open_id(STR_ENCODING)	(			\
			((STR_ENCODING) == Sanitize_for_UTF8_e_EUC_JISX0213)?		\
		  											"EUCJP-JISX0213"	\
			: (((STR_ENCODING) == Sanitize_for_UTF8_e_EUCJP_OPEN)?		\
													"EUCJP-OPEN"		\
			: (((STR_ENCODING) == Sanitize_for_UTF8_e_EUCJP_WIN)?		\
													"EUCJP-WIN"			\
			: (((STR_ENCODING) == Sanitize_for_UTF8_e_EUCJP_MS)?		\
													"EUCJP-MS"			\
			: (((STR_ENCODING) == Sanitize_for_UTF8_e_EUCJP)?			\
													"EUCJP"				\
			: (((STR_ENCODING) == Sanitize_for_UTF8_e_SJIS_OPEN)?		\
													"SJIS-OPEN"			\
			: (((STR_ENCODING) == Sanitize_for_UTF8_e_SJIS_WIN)?		\
													"SJIS-WIN"			\
			: (((STR_ENCODING) == Sanitize_for_UTF8_e_SJIS)?			\
													"SJIS"				\
			:										NULL				\
			))))))))

/* iconv_open(3)'s identifier of UTF-8 */
#define Sanitize_for_UTF8_iconv_open_id	"UTF-8"

/* `how' arguments are the logical-or-ed the followings...
 * (default, to_utf8() converts Yen-sign (\x5c)  to UTF-8 "\xc2\xa5 U+00A5",
 * convert tilde (0x7e) to UTF-8 "\x7e"), replace the hankaku-kana with
 * `replace_code' character, and replace the machine-depended Chinese
 * character with `replace_code'.)
 */
#define Sanitize_for_UTF8_Default_how				(0UL)
#define Sanitize_for_UTF8_Yen_sign_must_be_U005C	(1UL)
#define Sanitize_for_UTF8_Tilde_must_be_U203E		(2UL)
#define Sanitize_for_UTF8_Pass_hankaku_kana			(4UL)
#define Sanitize_for_UTF8_Pass_extra_characters		(8UL)
#define Sanitize_for_UTF8_Set_all					(					\
							  Sanitize_for_UTF8_Default_how				\
							| Sanitize_for_UTF8_Yen_sign_must_be_U005C	\
							| Sanitize_for_UTF8_Tilde_must_be_U203E		\
							| Sanitize_for_UTF8_Pass_hankaku_kana		\
							| Sanitize_for_UTF8_Pass_extra_characters	\
							)

typedef	unsigned char	uint8_t;
/* this function sanitizes a string (safer UTF-8 encoding)
 * if error occurred, this returns NULL and set `errno'.
 *	EINVAL means "illegal arguments".
 */
extern uint8_t /*@null@*/	*sanitize_for_utf8(
										  Sanitize_for_UTF8_e	src_encode
										, size_t				length
										, uint8_t			*str
										, char					replace_code
										, unsigned long			how
										);

/* convert the EUC-JP or SJIS encoded string to UTF-8 encoded.
 * if error occurred, this returns NULL and set `errno'.
 *	EINVAL means "illegal arguments".
 *	ENOMEM means "insufficient memory".
 *	EPERM means "system not support EUC-JP, SJIS or UTF-8 encode".
 *	EILSEQ means "`inbuf' contains illegal character".
 */
extern uint8_t /*@null@*/	*to_utf8(Sanitize_for_UTF8_e	inbuf_encode
										, const uint8_t	*inbuf
										, size_t				inbuf_length
										, char					replace_code
										, uint8_t			*outbuf
										, size_t				outbuf_size
										, char					padding_code
										, unsigned long			how
										);

/* convert the EUC-JP or SJIS encoded string to UTF-8 encoded.
 * if error occurred, this returns NULL and set `errno'.
 *	EINVAL means "illegal arguments".
 *	ENOMEM means "insufficient memory".
 *	EPERM means "system not support EUC-JP, SJIS or UTF-8 encode".
 *	EILSEQ means "`inbuf' contains illegal character".
 */
extern uint8_t /*@null@*/	*from_utf8(
									  Sanitize_for_UTF8_e	outbuf_encode
									, uint8_t			*inbuf
									, size_t				inbuf_length
									, uint8_t			*outbuf
									, size_t				outbuf_size
									, char					padding_code
									);

#ifdef __cplusplus
}
#endif
#endif /* } (!defined(_UTF8_H_) */


/***********************************************************************
 * File: utf8.c
 * 	(Extracted from
 *	   "$Id: utf8.c,v 1.2 2009/05/30 15:34:08 maoyam Exp maoyam $")
 *
 * Abstracts:
 *  This C source implements a character converter between UTF-8 and
 *  EUC-JP/SJIS.
 *
 * Copyright (C) 2008 Maoyam Tokyo, Japan (mailto:maoyam@mail.goo.ne.jp)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **********************************************************************/


/* include this library definitions */
//#include "utf8.h"

typedef struct garbled_characters_t {
	unsigned long	how;		/* mask of `how' argument (if the bit's off,
								 * replace the character class)
								 */
	uint8_t	*from, *to; /* set range of garbled code region */
} Garbled_characters_t;

/* this is the comparison of `Garbled_characters_t' array, for `bsearch(3).
 */
static int cmp_Garbled_characters_t(const void *ap, const void *bp)
{
	Garbled_characters_t	*key, *gap;
	int						result;

	if (((Garbled_characters_t *)ap)->to == NULL) {
		key = (Garbled_characters_t *)ap; gap = (Garbled_characters_t *)bp;
	}
	else {
		key = (Garbled_characters_t *)bp; gap = (Garbled_characters_t *)ap;
	}

	if ((((key->how & gap->how) != 0UL)
	|| key->from == NULL)
	|| memcmp(key->from, gap->from, 2) < 0
	) {
		result	= -1;
	}
	else if (memcmp(key->from, gap->to, 2) > 0) {
		result	= 1;
	}
	else {
		result	= 0; /* the `str' will be garbled. */
	}

	return	result;
}

/* this function implements how to sanitizes a string (safer UTF-8
 * encoding). if error occurred, this returns NULL and set `errno'.
 *	ENOMEM means "insufficient memory".
 */
static uint8_t /*@null@*/  *get_sanitized(
									  Sanitize_for_UTF8_e		str_encoding
									, size_t					length
									, uint8_t				*str
									, size_t					outstr_size
									, /*@null@*/ uint8_t	*outstr
									, char						replace_code
									, unsigned long				how
									)
{
	static Garbled_characters_t
	  eucjp_garbled[] = {
		  { /* JIS X 0201 (Kana) 21 to 5F */
			  Sanitize_for_UTF8_Pass_hankaku_kana
			, (uint8_t *)"\x8e\x21", (uint8_t *)"\x8e\x5f"
			}
		, {	/* JIS X 0208 (level 1) 02-15 to 02-25 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xa2\xaf", (uint8_t *)"\xa2\xb9"
			}
		, {	/* JIS X 0208 (level 1) 02-34 to 02-41 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xa2\xc2", (uint8_t *)"\xa2\xc9"
			}
		, {	/* JIS X 0208 (level 1) 02-49 to 02-59 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xa2\xd1", (uint8_t *)"\xa2\xdb"
			}
		, {	/* JIS X 0208 (level 1) 02-75 to 02-81 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xa2\xeb", (uint8_t *)"\xa2\xf1"
			}
		, {	/* JIS X 0208 (level 1) 02-90 to 02-93 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xa2\xfa", (uint8_t *)"\xa2\xfd"
			}
		, {	/* JIS X 0208 (level 1) 03-01 to 03-15 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xa3\xa1", (uint8_t *)"\xa3\xaf"
			}
		, {	/* JIS X 0208 (level 1) 03-26 to 03-32 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xa3\xba", (uint8_t *)"\xa3\xc0"
			}
		, {	/* JIS X 0208 (level 1) 03-59 to 03-64 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xa3\xdb", (uint8_t *)"\xa3\xe0"
			}
		, {	/* JIS X 0208 (level 1) 03-91 to 03-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xa3\xfb", (uint8_t *)"\xa3\xfe"
			}
		, {	/* JIS X 0208 (level 1) 04-84 to 04-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xa4\xf4", (uint8_t *)"\xa4\xfe"
			}
		, {	/* JIS X 0208 (level 1) 05-87 to 05-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xa5\xf7", (uint8_t *)"\xa5\xfe"
			}
		, {	/* JIS X 0208 (level 1) 06-25 to 06-32 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xa6\xb9", (uint8_t *)"\xa6\xc0"
			}
		, {	/* JIS X 0208 (level 1) 06-57 to 06-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xa6\xd9", (uint8_t *)"\xa6\xfe"
			}
		, {	/* JIS X 0208 (level 1) 07-34 to 07-47 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xa7\xc2", (uint8_t *)"\xa7\xd0"
			}
		, {	/* JIS X 0208 (level 1) 07-82 to 07-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xa7\xf2", (uint8_t *)"\xa7\xfe"
			}
		, {	/* JIS X 0208 (level 1) 08-33 to 08-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xa8\xc1", (uint8_t *)"\xa8\xfe"
			}
		, {	/* JIS X 0208 (level 1) 09-01 to 09-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xa9\xa1", (uint8_t *)"\xa9\xfe"
			}
		, {	/* JIS X 0208 (level 1) 10-01 to 10-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xaa\xa1", (uint8_t *)"\xaa\xfe"
			}
		, {	/* JIS X 0208 (level 1) 11-01 to 11-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xab\xa1", (uint8_t *)"\xab\xfe"
			}
		, {	/* JIS X 0208 (level 1) 12-01 to 12-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xac\xa1", (uint8_t *)"\xac\xfe"
			}
		, {	/* JIS X 0208 (level 1) 13-01 to 13-94 (circled digits, etc) */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xad\xa1", (uint8_t *)"\xad\xfe"
			}
		, {	/* JIS X 0208 (level 1) 14-01 to 14-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xae\xa1", (uint8_t *)"\xae\xfe"
			}
		, {	/* JIS X 0208 (level 1) 15-01 to 15-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xaf\xa1", (uint8_t *)"\xaf\xfe"
			}
		, {	/* JIS X 0208 (level 1) 47-52 to 47-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xcf\xd4", (uint8_t *)"\xcf\xfe"
			}
		, {	/* JIS X 0208 (level 2) 84-07 to 84-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xf4\xa7", (uint8_t *)"\xf4\xfe"
			}
		, {	/* JIS X 0208 (level 2) 85-01 to 85-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xf5\xa1", (uint8_t *)"\xf5\xfe"
			}
		, {	/* JIS X 0208 (level 2) 86-01 to 86-94 (JIS not assigned) */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xf6\xa1", (uint8_t *)"\xf6\xfe"
			}
		, {	/* JIS X 0208 (level 2) 87-01 to 87-94 (JIS not assigned) */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xf7\xa1", (uint8_t *)"\xf7\xfe"
			}
		, {	/* JIS X 0208 (level 2) 88-01 to 88-94 (JIS not assigned) */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xf8\xa1", (uint8_t *)"\xf8\xfe"
			}
		, {	/* JIS X 0208 (level 2) 89-01 to 89-94 (Debian garbled) */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xf9\xa1", (uint8_t *)"\xf9\xfe"
			}
		, {	/* JIS X 0208 (level 2) 90-01 to 90-94 (Debian garbled) */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xfa\xa1", (uint8_t *)"\xfa\xfe"
			}
		, {	/* JIS X 0208 (level 2) 91-01 to 91-94 (Debian garbled) */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xfb\xa1", (uint8_t *)"\xfb\xfe"
			}
		, {	/* JIS X 0208 (level 2) 92-01 to 92-94 (Debian garbled) */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xfc\xa1", (uint8_t *)"\xfc\xfe"
			}
		, {	/* JIS X 0208 (level 2) 93-01 to 93-94 (JIS not assigned) */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xfd\xa1", (uint8_t *)"\xfd\xfe"
			}
		, {	/* JIS X 0208 (level 2) 94-01 to 94-94 (JIS not assigned) */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xfe\xa1", (uint8_t *)"\xfe\xfe"
			}
		}
	, sjis_garbled[] = {
		  {	/* JIS X 0208 (level 1) 02-15 to 02-25 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x81\xad", (uint8_t *)"\x81\xb7"
			}
		, {	/* JIS X 0208 (level 1) 02-34 to 02-41 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x81\xc0", (uint8_t *)"\x81\xc7"
			}
		, {	/* JIS X 0208 (level 1) 02-49 to 02-59 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x81\xcf", (uint8_t *)"\x81\xd9"
			}
		, {	/* JIS X 0208 (level 1) 02-75 to 02-81 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x81\xe9", (uint8_t *)"\x81\xef"
			}
		, {	/* JIS X 0208 (level 1) 02-90 to 02-93 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x81\xf8", (uint8_t *)"\x81\xfb"
			}
		, {	/* JIS X 0208 (level 1) 03-01 to 03-15 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x82\x40", (uint8_t *)"\x82\x4e"
			}
		, {	/* JIS X 0208 (level 1) 03-26 to 03-32 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x82\x59", (uint8_t *)"\x82\x5f"
			}
		, {	/* JIS X 0208 (level 1) 03-59 to 03-64 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x82\x7a", (uint8_t *)"\x82\x80"
			}
		, {	/* JIS X 0208 (level 1) 03-91 to 03-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x82\x9b", (uint8_t *)"\x82\x9e"
			}
		, {	/* JIS X 0208 (level 1) 04-84 to 04-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x82\xf2", (uint8_t *)"\x82\xfc"
			}
		, {	/* JIS X 0208 (level 1) 05-87 to 05-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x83\x97", (uint8_t *)"\x83\x9e"
			}
		, {	/* JIS X 0208 (level 1) 06-25 to 06-32 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x83\xb7", (uint8_t *)"\x83\xbe"
			}
		, {	/* JIS X 0208 (level 1) 06-57 to 06-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x83\xd7", (uint8_t *)"\x83\xfc"
			}
		, {	/* JIS X 0208 (level 1) 07-34 to 07-48 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x84\x61", (uint8_t *)"\x84\x6f"
			}
		, {	/* JIS X 0208 (level 1) 07-82 to 07-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x84\x92", (uint8_t *)"\x84\x9e"
			}
		, {	/* JIS X 0208 (level 1) 08-33 to 08-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x84\xbf", (uint8_t *)"\x84\xfc"
			}
		, {	/* JIS X 0208 (level 1) 09-01 to 09-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x85\x40", (uint8_t *)"\x85\x9e"
			}
		, {	/* JIS X 0208 (level 1) 10-01 to 10-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x85\x9f", (uint8_t *)"\x85\xfc"
			}
		, {	/* JIS X 0208 (level 1) 11-01 to 11-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x86\x40", (uint8_t *)"\x86\x9e"
			}
		, {	/* JIS X 0208 (level 1) 12-01 to 12-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x86\x9f", (uint8_t *)"\x86\xfc"
			}
		, {	/* JIS X 0208 (level 1) 13-01 to 13-94 (circled digits, etc) */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x87\x40", (uint8_t *)"\x87\x9e"
			}
		, {	/* JIS X 0208 (level 1) 14-01 to 14-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x87\x9f", (uint8_t *)"\x87\xfc"
			}
		, {	/* JIS X 0208 (level 1) 15-01 to 15-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x88\x40", (uint8_t *)"\x88\x9e"
			}
		, {	/* JIS X 0208 (level 1) 47-52 to 47-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\x98\x73", (uint8_t *)"\x98\x9e"
			}
		, {	/* JIS X 0208 (level 2) 84-07 to 84-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xea\xa5", (uint8_t *)"\xea\xfc"
			}
		, {	/* JIS X 0208 (level 2) 85-01 to 85-94 */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xeb\x40", (uint8_t *)"\xeb\x9e"
			}
		, {	/* JIS X 0208 (level 2) 86-01 to 86-94 (JIS not assigned) */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xeb\x9f", (uint8_t *)"\xeb\xfc"
			}
		, {	/* JIS X 0208 (level 2) 87-01 to 87-94 (JIS not assigned) */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xec\x40", (uint8_t *)"\xec\x9e"
			}
		, {	/* JIS X 0208 (level 2) 88-01 to 88-94 (JIS not assigned) */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xec\x9f", (uint8_t *)"\xec\xfc"
			}
		, {	/* JIS X 0208 (level 2) 89-01 to 89-94 (Debian garbled) */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xed\x40", (uint8_t *)"\xed\x9e"
			}
		, {	/* JIS X 0208 (level 2) 90-01 to 90-94 (Debian garbled) */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xed\x9f", (uint8_t *)"\xed\xfc"
			}
		, {	/* JIS X 0208 (level 2) 91-01 to 91-94 (Debian garbled) */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xee\x40", (uint8_t *)"\xee\x9e"
			}
		, {	/* JIS X 0208 (level 2) 92-01 to 92-94 (Debian garbled) */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xee\x9f", (uint8_t *)"\xee\xfc"
			}
		, {	/* JIS X 0208 (level 2) 93-01 to 93-94 (JIS not assigned) */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xef\x40", (uint8_t *)"\xef\x9e"
			}
		, {	/* JIS X 0208 (level 2) 94-01 to 94-94 (JIS not assigned) */
			  Sanitize_for_UTF8_Pass_extra_characters
			, (uint8_t *)"\xef\x9f", (uint8_t *)"\xef\xfc"
			}
		};
	struct garbled_characters_t	*garbled, *gap;
	Garbled_characters_t		key;
	uint8_t				*result;
	size_t						size, nmemb;

	/* invoker needs the allocated memory of sanitized string? */
	if (outstr == NULL) {
		if ((outstr = malloc((length + 1))) == NULL) {
			/* Error: can't allocate sanitized string. */
			errno = ENOMEM; return NULL; /* insufficient memory */
		}
		memset(outstr, (int)'\0', (length + 1));
	}
	result	= outstr;

	/* set table of `garbled' code */
	if (str_encoding <= Sanitize_for_UTF8_e_EUCJP) {
		garbled	= eucjp_garbled
		; nmemb	= (sizeof(eucjp_garbled)/ sizeof(eucjp_garbled[0]))
		;
	}
	else {
		garbled	= sjis_garbled
		; nmemb	= (sizeof(sjis_garbled)/ sizeof(sjis_garbled[0]))
		;
	}

	/* process the source string... */
	while (((outstr_size > 0) && length > 0) && *str != (uint8_t)'\0')
	{
		if (*str <= (uint8_t)0x7f) {
			/* leave the ASCII (0x01 to 0x7d, 0x7f) and C0-control codes */
			*outstr	= *str
			; ++outstr; --outstr_size; ++str; --length
			; continue
			;
		}

		/* process extra codes (according to the each source encoding) */
		if (str_encoding <= Sanitize_for_UTF8_e_EUCJP) {
			if (*str == (uint8_t)0x8f) {
				/* replace EUC-JP extra code (three bytes are started
				 * '\x8f') using `replace_code (ASCII code)', or pass
				 * the EUC-JP extra code (3 bytes).
				 */
				size	= ((outstr_size < 3)? outstr_size: 3);
				if (((how & Sanitize_for_UTF8_Pass_extra_characters) == 0UL)
				|| size < 3
				) {
					memset(outstr, (int)replace_code, size);
				}
				else {
					if (outstr != str) { memcpy(outstr, str, size);	}
				}
				outstr += size; outstr_size -= size
				; str += size; length -= size
				; continue
				;
			}
		}
		else if (str_encoding <= Sanitize_for_UTF8_e_SJIS) {
			if ((*str >= (uint8_t)0xa1)
			&& *str <= (uint8_t)0xdf
			) {	/* replace the SJIS hankaku-kana code (single byte) */
				*outstr	= (((how & Sanitize_for_UTF8_Pass_hankaku_kana)
																== 0UL)?
												(uint8_t)replace_code
							:					*str
							);
				; ++outstr; --outstr_size; ++str; --length; continue
				;
			}
		}

		/* check left length */
		if (outstr_size < 2) {
			/* process the last one byte
			 * replace the non-ASCII code, or pass the ASCII code.
			 */
			*outstr	= ((*str > (uint8_t)0x7f)?
										(uint8_t)replace_code
						:				*str
						);
			++outstr; --outstr_size; ++str; --length
			; continue
			;
		}

		/* check and process the two bytes... */
		size = 2
		; key.how = how; key.from = str; key.to = NULL
		; gap	= (Garbled_characters_t *)bsearch((const void *)(&key)
												, garbled
												, nmemb
												, sizeof(*garbled)
												, cmp_Garbled_characters_t
												);
		if (gap != NULL) {
			/* replace garbled code, using `replace_code (ASCII code)' */
			memset(outstr, (int)replace_code, size);
		}
		else {
			/* pass the wide character */
			if (outstr != str) { memcpy(outstr, str, size);	}
		}
		outstr += size; outstr_size -= size; str += size; length -= size;
	}

	/* terminate the `outstr' string */
	if (outstr_size > 0) {
		*outstr	= (uint8_t)'\0';
	}

	return	result;	/* processed normally */
}

/* this function sanitizes a string (safer UTF-8 encoding)
 * if error occurred, this returns NULL and set `errno'.
 *	EINVAL means "illegal arguments".
 */
uint8_t	*sanitize_for_utf8(Sanitize_for_UTF8_e	str_encoding
									, size_t			length
									, uint8_t		*str
									, char				replace_code
									, unsigned long		how
									)
{
	if (((((str_encoding <= Sanitize_for_UTF8_e_Zero
	|| str_encoding >= Sanitize_for_UTF8_e_N)
	|| str == NULL)
	|| replace_code <= (char)'\0')
	|| (how & ~Sanitize_for_UTF8_Set_all) != 0UL)
	) {	/* ERROR: illegal argument */
		/* invalid arguments */
		errno = EINVAL; return (uint8_t *)NULL;
	}

	return	get_sanitized(str_encoding
						, length
						, str
						, length
						, str
						, replace_code
						, how
						);
}

/* convert the EUC-JP or SJIS encoded string to UTF-8 encoded.
 * if error occurred, this returns NULL and set `errno'.
 *	EINVAL means "illegal arguments".
 *	ENOMEM means "insufficient memory".
 *	EPERM means "system not support EUC-JP, SJIS or UTF-8 encode".
 *	EILSEQ means "`inbuf' contains illegal character".
 */
uint8_t *to_utf8(Sanitize_for_UTF8_e		inbuf_encode
						, const uint8_t	*inbuf
						, size_t				inbuf_length
						, char					replace_code
						, uint8_t			*outbuf
						, size_t				outbuf_size
						, char					padding_code
						, unsigned long			how
						)
{
	uint8_t	*dup_inbuf = NULL, *inbufp = NULL
					, *iconved_buf = NULL, *outbufp = NULL
					, *result = NULL
					;
	iconv_t			cd =(iconv_t)(-1);
	size_t			inbytesleft = inbuf_length, outbytesleft = outbuf_size;
	char			*tocode = NULL, *fromcode = NULL;
	int				sv_errno = 0;

	if ((((((inbuf_encode <= Sanitize_for_UTF8_e_Zero)
	|| inbuf_encode >= Sanitize_for_UTF8_e_N)
	|| inbuf == NULL)
	|| outbuf == NULL)
	|| (how & ~Sanitize_for_UTF8_Set_all) != 0UL)
	) {	/* Error: argument is illegal. */
		errno = EINVAL; return NULL; /* invalid arguments */
	}

	if (replace_code == '\0') {
		replace_code	= ' ';	/* default replace_code is space (0x20) */
	}

	/* create convert-descriptor (EUC-JP or SJIS to UTF-8) */
	tocode		= Sanitize_for_UTF8_iconv_open_id
	; fromcode	= Sanitize_for_UTF8_e_To_iconv_open_id(inbuf_encode)
	;
	if ((cd = iconv_open((const char *)tocode, (const char *)fromcode))
														== (iconv_t)(-1)
	) {	/* Error: system doesn't support EUC-JP, SJIS or UTF-8 encode. */
		errno	= EPERM; return	NULL; /* system not supported */
	}

	/* allocate and initialize the iconv-ed buffer */
	if ((iconved_buf = malloc(outbuf_size + 1)) == NULL) {
		/* Error: can't allocate iconv-ed string. */
		(void)iconv_close(cd); cd = (iconv_t)(-1);
		errno = ENOMEM; return NULL; /* insufficient memory */
	}
	outbufp	= memset(iconved_buf, (int)'\0', (outbuf_size + 1));

	/* sanitize duplicated inbuf (for UTF-8) */
	if ((dup_inbuf = get_sanitized(inbuf_encode
								, inbuf_length
								, (uint8_t *)inbuf
								, inbuf_length
								, NULL
								, replace_code
								, how
								)) == NULL
	) {	/* Error: maybe, insufficient memory (errno == ENOMEM) */
		(void)iconv_close(cd); cd = (iconv_t)(-1)
		; free(iconved_buf); iconved_buf = NULL
		; return	NULL
		;
	}
	inbufp	= dup_inbuf;

	/* (if dup_inbuf isn't empty) convert dup_inbuf to UTF-8 encode. */
	sv_errno	= errno; errno = 0 /* initialize `errno' */
	;
	if (((*inbufp != (uint8_t)'\0')
	&& iconv(cd,	(char **)(&inbufp), &inbytesleft
			,		(char **)(&outbufp), &outbytesleft
			) == (size_t)(-1))
	&& (errno == EILSEQ || errno == EINVAL)
	) {	/* Error: `inbuf' contains illegal character (EUC-JP or SJIS) */
		errno = EILSEQ;	/* found illegal sequence in `inbuf' */
	}
	else { /* process the UTF-8 encode string (iconved_buf) */
		uint8_t	*inbp = iconved_buf, *outbp = outbuf;

		for (result = outbuf, outbytesleft = outbuf_size
		; (*inbp != (uint8_t)'\0') && outbytesleft > 0
		;
		) {
			size_t	move_size;

			/* replace Yen-sign to UTF-8's 'Yen' code ("\xc2\xa5"),
			 * or replace Yen-sign to UTF-8's `backslash' code ("\x5c")
			 */
			if (((how & Sanitize_for_UTF8_Yen_sign_must_be_U005C) == 0)
			&& *inbp == (uint8_t)'\\'
			) {
				if (outbytesleft < 2) {
					break; /* outbuf is insufficient left size */
				}
				memcpy(outbp, "\xc2\xa5", 2)
				; ++inbp; outbp += 2; outbytesleft -= 2
				; continue
				;
			}
			if (((how & Sanitize_for_UTF8_Yen_sign_must_be_U005C) != 0)
			&& memcmp(inbp, "\xc2\xa5", 2) == 0
			) {
				*outbp	= (uint8_t)'\\'
				; inbp += 2; ++outbp; --outbytesleft
				; continue
				;
			}

			/* replace UTF-8's tilde (3 bytes code) to ASCII's ('\x7e'),
			 * or replace ASCII tilde to UTF-8's (wide-character)
			 */
			if (((how & Sanitize_for_UTF8_Tilde_must_be_U203E) == 0)
			&& memcmp(inbp, "\xe2\x80\xbe", 3) == 0
			) {
				*outbp	= '\x7e'
				; inbp += 3; ++outbp; --outbytesleft
				; continue
				;
			}
			if (((how & Sanitize_for_UTF8_Tilde_must_be_U203E) != 0)
			&& *inbp == (uint8_t)'\x7e'
			) {
				if (outbytesleft < 3) {
					break; /* outbuf is insufficient left size */
				}
				memcpy(outbp, "\xe2\x80\xbe", 3)
				; ++inbp ; outbp += 3; outbytesleft -= 3
				; continue
				;
			}

			/* get the each size of UTF-8 characters (from 1 to 6 bytes) */
			if ((*inbp & (uint8_t)0x80) == (uint8_t)0)
			{	move_size	= 1;	}
			else if ((*inbp & (uint8_t)0xe0) == (uint8_t)0xc0)
			{	move_size	= 2;	}
			else if ((*inbp & (uint8_t)0xf0) == (uint8_t)0xe0)
			{	move_size	= 3;	}
			else if ((*inbp & (uint8_t)0xf8) == (uint8_t)0xf0)
			{	move_size	= 4;	}
			else if ((*inbp & (uint8_t)0xfc) == (uint8_t)0xf8)
			{	move_size	= 5;	}
			else if ((*inbp & (uint8_t)0xfe) == (uint8_t)0xfc)
			{	move_size	= 6;	}
			else { /* Error: found illegal sequence in UTF-8 string */
				errno = EILSEQ; result = (uint8_t *)NULL
				; break	/* illegal sequence */
				;
			}
			
			/* transfer a UTF-8 character to outbuf */
			if (outbytesleft < move_size) {
				break; /* insufficient left size of outbuf */
			}
			memcpy(outbp, inbp, move_size)
			; inbp += move_size
			; outbp += move_size; outbytesleft -= move_size
			;
		}

		/* (if outbuf has left space) fill padding_code to outbuf */
		if (outbytesleft > 0) {
			memset(outbp, (int)padding_code, outbytesleft);
		}
	}

	/* release convert-descriptor and allocated memories */
	if (result == NULL) {	
		/* Error: occurred an error */
		sv_errno	= errno;
	}
	; (void)iconv_close(cd); cd = (iconv_t)(-1)
	; free(dup_inbuf); dup_inbuf = NULL
	; free(iconved_buf); iconved_buf = NULL
	; errno	= sv_errno /* restore `error' */
	;

	/* if error occurred, returns NULL. otherwise, returns address of outbuf
	 */
	return	result;
}

/* convert the UTF-8 encoded string to EUC-JP or SJIS encoded.
 * if error occurred, this returns NULL and set `errno'.
 *	EINVAL means "illegal arguments".
 *	ENOMEM means "insufficient memory".
 *	EPERM means "system not support EUC-JP, SJIS or UTF-8 encode".
 *	EILSEQ means "`inbuf' contains illegal character".
 */
uint8_t *from_utf8(Sanitize_for_UTF8_e	outbuf_encode
						, uint8_t			*inbuf
						, size_t				inbuf_length
						, uint8_t			*outbuf
						, size_t				outbuf_size
						, char					padding_code
						)
{
	uint8_t	*result = NULL, *reduced_inbuf = NULL, *inbufp = NULL
					, *outbufp = NULL
					;
	iconv_t			cd = (iconv_t)(-1);
	size_t			inbytesleft = inbuf_length, outbytesleft = outbuf_size;
	char			*fromcode = NULL, *tocode = NULL;
	int				sv_errno = errno;

	if (((((outbuf_encode <= Sanitize_for_UTF8_e_Zero)
	|| outbuf_encode >= Sanitize_for_UTF8_e_N)
	|| inbuf == NULL)
	|| outbuf == NULL)
	) {	/* Error: argument is illegal. */
		errno	= EINVAL;	return	NULL; /* invalid arguments */
	}

	/* create convert-descriptor (EUC-JP or SJIS to UTF-8) */
	fromcode	= Sanitize_for_UTF8_iconv_open_id
	; tocode	= Sanitize_for_UTF8_e_To_iconv_open_id(outbuf_encode)
	;
	if ((cd = iconv_open((const char *)tocode
						, (const char *)fromcode
						)) == (iconv_t)(-1)
	) {	/* Error: system doesn't support EUC-JP, SJIS or UTF-8 encode. */
		errno = EPERM; return NULL; /* system not supported */
	}

	/* allocate the buffer of reduced `inbuf' */
	if ((reduced_inbuf = malloc((inbuf_length + 1))) == NULL) {
		/* Error: can't allocate duplicated buffer of `inbuf'. */
		(void)iconv_close(cd); cd = (iconv_t)(-1);
		errno = ENOMEM; return NULL; /* insufficient memory */
	}
	outbufp	= memset(reduced_inbuf, (int)'\0', (inbuf_length + 1));

	/* reduce redundant UTF-8 string. */
	for (inbufp = inbuf
	; (*inbuf != (uint8_t)'\0') && inbytesleft > 0
	;
	) {
		if (*inbufp < (uint8_t)0x80) {
			*outbufp	= *inbufp
			; ++outbufp; ++inbufp; --inbytesleft
			; continue
			;
		}
		else if (*inbufp < (uint8_t)0xc2) {
			if (inbytesleft < 2) {
				goto Illegal_sequence;
			}
Reduce_1_byte:
			*outbufp = ((0x40 & (*inbufp << 6)) | (*(inbufp + 1) & 0x3f))
			; ++outbufp; inbufp += 2; inbytesleft -= 2
			; continue
			;
		}
		else if ((*inbufp & (uint8_t)0xe0) == (uint8_t)0xc0) {
			if (inbytesleft < 2) {
				goto Illegal_sequence;
			}
			memcpy((char *)outbufp, inbufp, 2)
			; outbufp += 2; inbufp += 2; inbytesleft -= 2
			; continue
			;
		}
		else if (*inbufp == (uint8_t)0xe0) {
			if (inbytesleft < 3) {
				goto Illegal_sequence;
			}
			if (*(inbufp + 1) >= (uint8_t)0xa0) {
Transfer_3_bytes:
				memcpy((char *)outbufp, inbufp, 3)
				; outbufp += 3; inbufp += 3; inbytesleft -= 3
				; continue
				;
			}

Reduce_2_bytes:
			if (*(inbufp + 1) >= (uint8_t)0x82) {
				*outbufp	= (0xc0 | (*(inbufp + 1) & 0x1f))
				; *(outbufp + 1)	= *(inbufp + 2)
				; outbufp += 2; inbufp += 3; inbytesleft -= 3
				; continue
				;
			}
			else {
				++inbufp; --inbytesleft; goto Reduce_1_byte;
			}
		}
		else if ((*inbufp & 0xf0) == (uint8_t)0xe0) {
			if (inbytesleft < 3) {
				goto Illegal_sequence;
			}
			goto Transfer_3_bytes;
		}
		else if (*inbufp == (uint8_t)0xf0) {
			if (inbytesleft < 4) {
				goto Illegal_sequence;
			}
			if (*(inbufp + 1) >= (uint8_t)0x90) {
Transfer_4_bytes:
				memcpy((char *)outbufp, inbufp, 4)
				; outbufp += 4; inbufp += 4; inbytesleft -= 4
				; continue
				;
			}

Reduce_3_bytes:
			if ((*(inbufp + 1) >= (uint8_t)0x81)
			|| ((*(inbufp + 1) == (uint8_t)0x80)
				&& *(inbufp + 2) >= (uint8_t)0xa0
				)
			) {
				*outbufp	= (0xe0 | (*(inbufp + 1) & 0x0f))
				; memcpy((char *)(outbufp + 1), (inbufp + 2), 2)
				; outbufp += 3; inbufp += 4; inbytesleft -= 4
				; continue
				;
			}
			else {
				++inbufp; --inbytesleft; goto Reduce_2_bytes;
			}
		}
		else if ((*inbufp & 0xf8) == (uint8_t)0xf0) {
			if (inbytesleft < 4) {
				goto Illegal_sequence;
			}
			goto Transfer_4_bytes;
		}
		else if (*inbufp == (uint8_t)0xf8) {
			if (inbytesleft < 5) {
				goto Illegal_sequence;
			}
			if (*(inbufp + 1) >= (uint8_t)0x88) {
Transfer_5_bytes:
				memcpy((char *)outbuf, inbufp, 5)
				; outbufp += 5; inbufp += 5; inbytesleft -= 5
				; continue
				;
			}

Reduce_4_bytes:
			if (*(inbufp + 1) >= (uint8_t)0x81) {
				*outbufp	= (0xf0 | (*(inbufp + 1) & 0x07))
				; memcpy((char *)(outbufp + 1), (inbufp + 2), 3)
				; outbufp += 4; inbufp += 5; inbytesleft -= 5
				; continue
				;
			}
			else {
				++inbufp; --inbytesleft; goto Reduce_3_bytes;
			}
		}
		else if ((*inbufp & 0xfc) == (uint8_t)0xf8) {
			if (inbytesleft < 5) {
				goto Illegal_sequence;
			}
			goto Transfer_5_bytes;
		}
		else if (*inbufp == (uint8_t)0xfc) {
			if (inbytesleft < 6) {
				goto Illegal_sequence;
			}
			if (*(inbufp + 1) >= (uint8_t)0x84) {
Transfer_6_bytes:
				memcpy((char *)outbufp, inbufp, 6)
				; outbufp += 6; inbufp += 6; inbytesleft -= 6
				; continue
				;
			}

			if (*(inbufp + 1) >= (uint8_t)0x81) {
				*outbufp	= (0xf8 | (*(inbufp + 1) & 0x03))
				; memcpy((char *)(outbufp + 1), (inbufp + 2), 4)
				; outbufp += 5; inbufp += 6; inbytesleft -= 6
				; continue
				;
			}
			else {
				++inbufp; --inbytesleft; goto Reduce_4_bytes;
			}
		}
		else if ((*inbufp & 0xfe) == (uint8_t)0xfc) {
			if (inbytesleft < 6) {
				goto Illegal_sequence;
			}
			goto Transfer_6_bytes;
		}
		else {
Illegal_sequence: /* Error: inbuf contains illegal sequence */
			(void)iconv_close(cd); cd = (iconv_t)(-1)
			; free(reduced_inbuf); reduced_inbuf = NULL
			;
			errno	= EILSEQ; return NULL; /* illegal sequence */
		}
	}

	/* get length of the reduced UTF-8 string (inbytesleft), inbufp,
	 * outbytesleft and outbufp
	 */
	inbytesleft		= (size_t)(outbufp - reduced_inbuf)
	; inbufp		= reduced_inbuf
	; outbytesleft	= outbuf_size
	; outbufp		= outbuf
	;

	/* (if dup_inbuf isn't empty) convert dup_inbuf to UTF-8 encode. */
	errno	= 0
	;
	if (((*inbufp != (uint8_t)'\0')
	&& iconv(cd,	(char **)(&inbufp), &inbytesleft
			,		(char **)(&outbufp), &outbytesleft
			) == (size_t)(-1))
	&& (errno == EILSEQ || errno == EINVAL)
	) {	/* Error: `inbuf' contains illegal character (EUC-JP or SJIS) */
		errno = EILSEQ; /* found illegal sequence in `inbuf' */
	}
	else {
		/* (if outbuf has left space) fill padding_code to outbuf */
		if (outbytesleft > 0) {
			memset(outbufp, (int)padding_code, outbytesleft);
		}

		result	= outbuf;
	}

	/* release convert-descriptor and allocated buffer */
	if (result == NULL) {
		sv_errno	= errno;
	}
	(void)iconv_close(cd); cd = (iconv_t)(-1)
	; free(reduced_inbuf); reduced_inbuf = NULL
	; errno	= sv_errno
	;

	/* if error occurred, returns NULL. otherwise, returns address of outbuf
	 */
	return	result;
}
