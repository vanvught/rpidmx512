/**
 * @file sscan.h
 *
 */
/* Copyright (C) 2016-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
extern int sscan_i2c_address(const char *, const char *, /*@out@*/uint8_t *);
extern int sscan_hexuint16(const char *buf, const char *name, /*@out@*/uint16_t *uint16);
extern int sscan_hex24uint32(const char *buf, const char *name, /*@out@*/uint32_t *uint32);

#ifdef __cplusplus
class Sscan {
public:
	enum ReturnCode {
		OK = 2, NAME_ERROR = 1, VALUE_ERROR = 0	// TODO
	};

	static ReturnCode Uint8(const char *pBuffer, const char *pName, uint8_t &nValue);
	static ReturnCode Uint16(const char *pBuffer, const char *pName, uint16_t &nValue);
	static ReturnCode Uint32(const char *pBuffer, const char *pName, uint32_t &nValue);
	static ReturnCode I2c(const char *pBuffer, char *pName, uint8_t &nLength, uint8_t &nAddress, uint8_t &nChannel);
	static ReturnCode Spi(const char *pBuffer, char& nChipSelect, char *pName, uint8_t& nLength, uint8_t& nAddress, uint16_t& nDmxStartAddress, uint32_t& nSpeedHz);

	static int Uint8(const char *a, const char *b, uint8_t *c) {
		return sscan_uint8_t(a, b, c);
	}

	static int Uint16(const char *a, const char *b, uint16_t *c) {
		return sscan_uint16_t(a, b, c);
	}

	static int Uint32(const char *a, const char *b, uint32_t *c) {
		return sscan_uint32_t(a, b, c);
	}

	static int Float(const char *a, const char *b, float *c) {
		return sscan_float(a, b, c);
	}

	static int Char(const char *a, const char *b, char *c, uint8_t *d) {
		return sscan_char_p(a, b, c, d);
	}

	static int IpAddress(const char *a, const char *b, uint32_t *c) {
		return sscan_ip_address(a, b, c);
	}

	static int Uuid(const char *a, const char *b, char *c, uint8_t *d) {
		return sscan_uuid(a, b, c, d);
	}

	static int I2cAddress(const char *a, const char *b, uint8_t *c) {
		return sscan_i2c_address(a, b, c);
	}

	static int HexUint16(const char *a, const char *b, uint16_t *c) {
		return sscan_hexuint16(a, b, c);
	}

	static int Hex24Uint32(const char *a, const char *b, uint32_t *c) {
		return sscan_hex24uint32(a, b, c);
	}

private:
	static uint8_t fromHex(const char Hex[2]);
	static const char *checkName(const char *pBuffer, const char *pName);
};
}
#endif

#endif /* SSCAN_H_ */
