/**
 * @file uuid_unparse.c
 *
 */
/**
 * This code is inspired by:
 * http://code.metager.de/source/xref/linux/utils/util-linux/libuuid/src/
 *
 * Copyright (C) 1996, 1997 Theodore Ts'o.
 *
 * %Begin-Header%
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, and the entire permission notice in its entirety,
 *    including the disclaimer of warranties.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ALL OF
 * WHICH ARE HEREBY DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF NOT ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * %End-Header%
 */
/* Copyright (C) 2016-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <uuid/uuid.h>
#include <assert.h>

#include "uuid_internal.h"

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

static const char *fmt_lower ALIGNED = "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x";
static const char *fmt_upper ALIGNED = "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X";

#ifdef UUID_UNPARSE_DEFAULT_UPPER
#define FMT_DEFAULT fmt_upper
#else
#define FMT_DEFAULT fmt_lower
#endif

static void uuid_unpack(const uuid_t in, struct uuid *uu) {
	const uint8_t *ptr = in;
	uint32_t tmp;

	assert(uu != NULL);

	tmp = *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	uu->time_low = tmp;

	tmp = *ptr++;
	tmp = (tmp << 8) | *ptr++;
	uu->time_mid = (uint16_t)(tmp);

	tmp = *ptr++;
	tmp = (tmp << 8) | *ptr++;
	uu->time_hi_and_version = (uint16_t)(tmp);

	tmp = *ptr++;
	tmp = (tmp << 8) | *ptr++;
	uu->clock_seq = (uint16_t)(tmp);

	memcpy(uu->node, ptr, 6);
}

static void uuid_unparse_x(const uuid_t uu, char *out, const char *fmt) {
	struct uuid uuid;

	assert(out != NULL);
	assert(fmt != NULL);

	uuid_unpack(uu, &uuid);

	sprintf(out, fmt, uuid.time_low, uuid.time_mid, uuid.time_hi_and_version,
			uuid.clock_seq >> 8, uuid.clock_seq & 0xFF, uuid.node[0],
			uuid.node[1], uuid.node[2], uuid.node[3], uuid.node[4],
			uuid.node[5]);
}

void uuid_unparse_lower(const uuid_t uu, char *out) {
	uuid_unparse_x(uu, out, fmt_lower);
}

void uuid_unparse_upper(const uuid_t uu, char *out) {
	uuid_unparse_x(uu, out, fmt_upper);
}

void uuid_unparse(const uuid_t uu, char *out) {
	uuid_unparse_x(uu, out, FMT_DEFAULT);
}
