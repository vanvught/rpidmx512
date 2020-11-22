/**
 * @file propertiesbuilder.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef PROPERTIESBUILDER_H_
#define PROPERTIESBUILDER_H_

#include <stdint.h>
#include <stdio.h>

class PropertiesBuilder {
public:
	PropertiesBuilder(const char *pFileName, char *pBuffer, uint32_t nLength);

	template<typename T>
	bool Add(const char *pProperty, const T x, bool bIsSet = true, uint32_t nPrecision = 1) {
		if (m_nSize >= m_nLength) {
			return false;
		}

		char *p = &m_pBuffer[m_nSize];
		const uint32_t nSize = m_nLength - m_nSize;

		int i = add_part(p, nSize, pProperty, x, bIsSet, nPrecision);

		if (i > static_cast<int>(nSize)) {
			return false;
		}

		m_nSize += static_cast<uint32_t>(i);

		return true;
	}

	template<typename T> int inline add_part(char *p, uint32_t nSize, const char *pProperty, const T x, bool bIsSet, __attribute__((unused)) uint32_t nPrecision) {
		if (bIsSet) {
			return snprintf(p, nSize, "%s=%d\n", pProperty, static_cast<int>(x));
		}

		return snprintf(p, nSize, "#%s=%d\n", pProperty, static_cast<int>(x));
	}

	bool AddIpAddress(const char *pProperty, uint32_t nValue, bool bIsSet = true);

	bool AddHex8(const char *pProperty, uint8_t nValue, bool bIsSet = true) {
		return AddHex(pProperty, nValue, bIsSet, 2);
	}

	bool AddHex16(const char *pProperty, const uint16_t nValue16, bool bIsSet = true) {
		return AddHex(pProperty, nValue16, bIsSet, 4);
	}

	bool AddHex16(const char *pProperty, const uint8_t nValue[2], bool bIsSet = true) {
		const uint16_t v = (static_cast<uint16_t>(nValue[0]) << 8) | nValue[1];
		return AddHex16(pProperty, v, bIsSet);
	}

	bool AddHex24(const char *pProperty, const uint32_t nValue32, bool bIsSet = true) {
		return AddHex(pProperty, nValue32, bIsSet, 6);
	}

	bool AddComment(const char *pComment);

	uint32_t GetSize() {
		return m_nSize;
	}

private:
	bool AddHex(const char *pProperty, uint32_t nValue, const bool bIsSet, const uint32_t nWidth);

private:
	char *m_pBuffer;
	uint32_t m_nLength;
	uint32_t m_nSize{0};
};

template<> int inline PropertiesBuilder::add_part<float>(char *p, uint32_t nSize, const char *pProperty, const float x, bool bIsSet, uint32_t nPrecision) {
	if (bIsSet) {
		return snprintf(p, nSize, "%s=%.*f\n", pProperty, nPrecision, x);
	}

	return snprintf(p, nSize, "#%s=%.*f\n", pProperty, nPrecision, x);
}

template<> int inline PropertiesBuilder::add_part<char*>(char *p, uint32_t nSize, const char *pProperty, char* x, bool bIsSet, __attribute__((unused)) uint32_t nPrecision) {
	if (bIsSet) {
		return snprintf(p, nSize, "%s=%s\n", pProperty, x);
	}

	return snprintf(p, nSize, "#%s=%s\n", pProperty, x);
}

template<> int inline PropertiesBuilder::add_part<const char*>(char *p, uint32_t nSize, const char *pProperty, const char* x, bool bIsSet, uint32_t nPrecision) {
	return PropertiesBuilder::add_part(p, nSize, pProperty, const_cast<char *>(x), bIsSet, nPrecision);
}

#endif /* PROPERTIESBUILDER_H_ */
