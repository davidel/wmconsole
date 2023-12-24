/*
 *  WmConsole by Davide Libenzi (Windows Mobile console server)
 *  Copyright (C) 2006  Davide Libenzi
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Davide Libenzi <davidel@xmailserver.org>
 *
 */

#if !defined(_WMC_SHA1_H)
#define _WMC_SHA1_H


#if !defined(SHA1_INT32TYPE)
#define SHA1_INT32TYPE int
#endif

#define SHA1_DIGEST_SIZE 20



typedef unsigned SHA1_INT32TYPE sha1_int32;

typedef struct {
	sha1_int32 state[5];
	sha1_int32 count[2];
	unsigned char buffer[64];
} sha1_ctx_t;


void sha1_init(sha1_ctx_t *context);
void sha1_update(sha1_ctx_t *context, unsigned char const *data, unsigned int len);
void sha1_final(unsigned char digest[SHA1_DIGEST_SIZE], sha1_ctx_t *context);

#endif

