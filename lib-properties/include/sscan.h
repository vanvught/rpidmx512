/**
 * @file sscan.h
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef SSCAN_H_
#define SSCAN_H_

#include <stdint.h>

#define SSCAN_NAME_ERROR	0
#define SSCAN_VALUE_ERROR	1
#define SSCAN_OK			2

#ifdef __cplusplus
extern "C" {
#endif

extern int sscan_uint8_t(const char *, const char *, /*@out@*/uint8_t *);
extern int sscan_uint16_t(const char *, const char *, /*@out@*/uint16_t *);
extern int sscan_uint32_t(const char *, const char *, /*@out@*/uint32_t *);
extern int sscan_float(const char *, const char *, /*@out@*/float *);
extern int sscan_char_p(const char *, const char *, /*@out@*/char *, /*@out@*/uint8_t *);
extern int sscan_ip_address(const char *, const char *, /*@out@*/uint32_t *);
extern int sscan_uuid(const char *, const char *, /*@out@*/char *, /*@out@*/uint8_t *);
extern int sscan_i2c(const char *, /*@out@*/char *, /*@out@*/uint8_t *, /*@out@*/uint8_t *, /*@out@*/uint8_t *);
extern int sscan_i2c_address(const char *, const char *, /*@out@*/uint8_t *);
extern int sscan_hexuint16(const char *buf, const char *name, /*@out@*/uint16_t *uint16);
extern int sscan_spi(const char *buf, /*@out@*/char *spi, /*@out@*/char *name, /*@out@*/uint8_t *len, /*@out@*/uint8_t *address, /*@out@*/uint16_t *dmx, /*@out@*/uint32_t *speed);

#ifdef __cplusplus
class Sscan {
public:
	inline static int Uint8(const char *a, const char *b, uint8_t *c) {
		return sscan_uint8_t(a, b, c);
	}

	inline static int Uint16(const char *a, const char *b, uint16_t *c) {
		return sscan_uint16_t(a, b, c);
	}

	inline static int Uint32(const char *a, const char *b, uint32_t *c) {
		return sscan_uint32_t(a, b, c);
	}

	inline static int Float(const char *a, const char *b, float *c) {
		return sscan_float(a, b, c);
	}

	inline static int Char(const char *a, const char *b, char *c, uint8_t *d) {
		return sscan_char_p(a, b, c, d);
	}

	inline static int IpAddress(const char *a, const char *b, uint32_t *c) {
		return sscan_ip_address(a, b, c);
	}

	inline static int Uuid(const char *a, const char *b, char *c, uint8_t *d) {
		return sscan_uuid(a, b, c, d);
	}

	inline static int I2c(const char *a, char *b, uint8_t *c, uint8_t *d, uint8_t *e) {
	 	 return sscan_i2c(a, b, c, d, e);
	}

	inline static int I2cAddress(const char *a, const char *b, uint8_t *c) {
		return sscan_i2c_address(a, b, c);
	}
	
	inline static int HexUint16(const char *a, const char *b, uint16_t *c) {
		return sscan_hexuint16(a, b, c);
	}

	inline static int Spi(const char *a, char *b, char *c, uint8_t *d, uint8_t *e, uint16_t *f, uint32_t *g) {
		return sscan_spi(a, b, c , d, e, f, g);
	}

private:

};
}
#endif

#endif /* SSCAN_H_ */
