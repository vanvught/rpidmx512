/**
 * @file uuid.c
 *
 */
/**
 * This code is inspired by:
 * http://code.metager.de/source/xref/linux/utils/util-linux/libuuid/src/
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdio.h>

#include "uuid.h"
#include "bcm2835_rng.h"
#include "util.h"
#include "hex_uint32.h"

static const char *fmt_lower =
		"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x";

static const char *fmt_upper =
		"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X";

#ifdef UUID_UNPARSE_DEFAULT_UPPER
#define FMT_DEFAULT fmt_upper
#else
#define FMT_DEFAULT fmt_lower
#endif

typedef union pcast32 {
	uuid_t uuid;
	uint32_t u32[4];
} _pcast32;

struct uuid {
	uint32_t time_low;
	uint16_t time_mid;
	uint16_t time_hi_and_version;
	uint16_t clock_seq;
	uint8_t node[6];
};

/**
 *
 * @param uu
 * @param ptr
 */
static void uuid_pack(const struct uuid *uu, uuid_t ptr) {
	uint32_t tmp;
	unsigned char *out = ptr;

	tmp = uu->time_low;
	out[3] = (unsigned char) tmp;
	tmp >>= 8;
	out[2] = (unsigned char) tmp;
	tmp >>= 8;
	out[1] = (unsigned char) tmp;
	tmp >>= 8;
	out[0] = (unsigned char) tmp;

	tmp = uu->time_mid;
	out[5] = (unsigned char) tmp;
	tmp >>= 8;
	out[4] = (unsigned char) tmp;

	tmp = uu->time_hi_and_version;
	out[7] = (unsigned char) tmp;
	tmp >>= 8;
	out[6] = (unsigned char) tmp;

	tmp = uu->clock_seq;
	out[9] = (unsigned char) tmp;
	tmp >>= 8;
	out[8] = (unsigned char) tmp;

	memcpy(out + 10, uu->node, 6);
}

/**
 *
 * @param in
 * @param uu
 */
static void uuid_unpack(const uuid_t in, struct uuid *uu) {
	const uint8_t *ptr = in;
	uint32_t tmp;

	tmp = *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	uu->time_low = tmp;

	tmp = *ptr++;
	tmp = (tmp << 8) | *ptr++;
	uu->time_mid = tmp;

	tmp = *ptr++;
	tmp = (tmp << 8) | *ptr++;
	uu->time_hi_and_version = tmp;

	tmp = *ptr++;
	tmp = (tmp << 8) | *ptr++;
	uu->clock_seq = tmp;

	memcpy(uu->node, ptr, 6);
}

/**
 *
 * @param uu
 * @param out
 * @param fmt
 */
static void uuid_unparse_x(const uuid_t uu, char *out, const char *fmt) {
	struct uuid uuid;

	uuid_unpack(uu, &uuid);
	sprintf(out, fmt, uuid.time_low, uuid.time_mid, uuid.time_hi_and_version,
			uuid.clock_seq >> 8, uuid.clock_seq & 0xFF, uuid.node[0],
			uuid.node[1], uuid.node[2], uuid.node[3], uuid.node[4],
			uuid.node[5]);
}

/**
 *
 * @param uu
 * @param out
 */
void uuid_unparse_lower(const uuid_t uu, char *out) {
	uuid_unparse_x(uu, out, fmt_lower);
}

/**
 *
 * @param uu
 * @param out
 */
void uuid_unparse_upper(const uuid_t uu, char *out) {
	uuid_unparse_x(uu, out, fmt_upper);
}

/**
 *
 * @param uu
 * @param out
 */
void uuid_unparse(const uuid_t uu, char *out) {
	uuid_unparse_x(uu, out, FMT_DEFAULT);
}

/**
 *
 * @param out
 */
void uuid_generate_random(uuid_t out) {
	_pcast32 cast;

	cast.u32[0] = bcm2835_rng_get_number();
	cast.u32[1] = bcm2835_rng_get_number();
	cast.u32[2] = bcm2835_rng_get_number();
	cast.u32[3] = bcm2835_rng_get_number();

	cast.uuid[6] = 0x40 | (cast.uuid[6] & 0xf);
	cast.uuid[8] = 0x80 | (cast.uuid[8] & 0x3f);

	memcpy(out, cast.uuid, sizeof(uuid_t));
}

/**
 *
 * @param in
 * @param uu
 * @return
 */
int uuid_parse(const char *in, uuid_t uu) {
	struct uuid uuid;
	int i;
	const char *cp;
	char buf[3];

	if (strlen(in) != 36)
		return -1;
	for (i = 0, cp = in; i <= 36; i++, cp++) {
		if ((i == 8) || (i == 13) || (i == 18) || (i == 23)) {
			if (*cp == '-')
				continue;
			else
				return -1;
		}
		if (i == 36)
			if (*cp == 0)
				continue;
		if (!isxdigit(*cp))
			return -1;
	}
	uuid.time_low = hex_uint32(in);
	uuid.time_mid = hex_uint32(in + 9);
	uuid.time_hi_and_version = hex_uint32(in + 14);
	uuid.clock_seq = hex_uint32(in + 19);
	cp = in + 24;
	buf[2] = 0;
	for (i = 0; i < 6; i++) {
		buf[0] = *cp++;
		buf[1] = *cp++;
		uuid.node[i] = hex_uint32(buf);
	}

	uuid_pack(&uuid, uu);
	return 0;
}
