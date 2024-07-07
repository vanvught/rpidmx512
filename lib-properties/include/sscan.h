/**
 * @file sscan.h
 *
 */
/* Copyright (C) 2016-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>

class Sscan {
public:
	enum ReturnCode {
		OK, NAME_ERROR, VALUE_ERROR
	};

	static ReturnCode Char(const char *pBuffer, const char *pName, char *pValue, uint32_t& nLength);

	static ReturnCode Uint8(const char *pBuffer, const char *pName, uint8_t& nValue);
	static ReturnCode Uint16(const char *pBuffer, const char *pName, uint16_t& nValue);
	static ReturnCode Uint32(const char *pBuffer, const char *pName, uint32_t& nValue);

	static ReturnCode Float(const char *pBuffer, const char *pName, float& fValue);

	static ReturnCode IpAddress(const char *pBuffer, const char *pName, uint32_t& nIpAddress);

	static ReturnCode HexUint16(const char *pBuffer, const char *pName, uint16_t& nValue);
	static ReturnCode Hex24Uint32(const char *pBuffer, const char *pName, uint32_t &nValue);

	static ReturnCode I2cAddress(const char *pBuffer, const char *pName, uint8_t& nAddress);
	static ReturnCode I2c(const char *pBuffer, char *pName, uint8_t& nLength, uint8_t& nAddress, uint8_t& nReserved);
	static ReturnCode Spi(const char *pBuffer, char& nChipSelect, char *pName, uint8_t& nLength, uint8_t& nAddress, uint16_t &nDmxStartAddress, uint32_t &nSpeedHz);

	static ReturnCode UtcOffset(const char *pBuffer, const char *pName, int8_t& nHours, uint8_t& nMinutes);
private:
	static uint8_t fromHex(const char Hex[2]);
	static const char *checkName(const char *pBuffer, const char *pName);
};

#endif /* SSCAN_H_ */
