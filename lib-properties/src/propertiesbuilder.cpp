/**
 * @file propertiesbuilder.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "propertiesbuilder.h"

#include "network.h"

PropertiesBuilder::PropertiesBuilder(const char *pFileName, uint8_t *pBuffer, uint32_t nLength):
	m_pBuffer(pBuffer),
	m_nLength(nLength),
	m_nSize(0)
{
	assert(pFileName != 0);
	assert(pBuffer != 0);

	const uint32_t l = strlen(pFileName);

	if ((l + 2) <= m_nLength) {
		*pBuffer = '#';
		memcpy(&pBuffer[1], pFileName, l);
		pBuffer[l + 1] = '\n';
		m_nSize = l + 2;
	}
}

PropertiesBuilder::~PropertiesBuilder(void) {
}

bool PropertiesBuilder::Add(const char *pProperty, uint32_t nValue, bool bIsSet) {
#if !defined(BUILDER_NOT_SET)
	if (!bIsSet) {
		return false;
	}
#endif

	if (m_nSize >= m_nLength) {
		return false;
	}

	char *p = reinterpret_cast<char *>(&m_pBuffer[m_nSize]);
	const uint32_t nSize = m_nLength - m_nSize;

	int i;

	if (bIsSet) {
		i = snprintf(p, nSize, "%s=%d\n", pProperty, nValue);
	} else {
		i = snprintf(p, nSize, "#%s=%d\n", pProperty, nValue);
	}

	if (i > static_cast<int>(nSize)) {
		return false;
	}

	m_nSize += i;

#ifndef NDEBUG
	printf("m_nLength=%d, m_nSize=%d\n", m_nLength, m_nSize);
#endif

	return true;
}

bool PropertiesBuilder::Add(const char* pProperty, float fValue, bool bIsSet, uint32_t nPrecision) {
#if !defined(BUILDER_NOT_SET)
	if (!bIsSet) {
		return false;
	}
#endif

	if (m_nSize >= m_nLength) {
		return false;
	}

	char *p = reinterpret_cast<char *>(&m_pBuffer[m_nSize]);
	const uint32_t nSize = m_nLength - m_nSize;

	int i;

	if (bIsSet) {
		i = snprintf(p, nSize, "%s=%.*f\n", pProperty, nPrecision, fValue);
	} else {
		i = snprintf(p, nSize, "#%s=%.*f\n", pProperty, nPrecision, fValue);
	}

	if (i > static_cast<int>(nSize)) {
		return false;
	}

	m_nSize += i;

#ifndef NDEBUG
	printf("m_nLength=%d, m_nSize=%d\n", m_nLength, m_nSize);
#endif

	return true;
}

bool PropertiesBuilder::Add(const char *pProperty, const char *pValue, bool bIsSet) {
#if !defined(BUILDER_NOT_SET)
	if (!bIsSet) {
		return false;
	}
#endif

	if (m_nSize >= m_nLength) {
		return false;
	}

	char *p = reinterpret_cast<char *>(&m_pBuffer[m_nSize]);
	const uint32_t nSize = m_nLength - m_nSize;

	int i;

	if (bIsSet) {
		i = snprintf(p, nSize, "%s=%s\n", pProperty, pValue);
	} else {
		i = snprintf(p, nSize, "#%s=%s\n", pProperty, pValue);
	}

	if (i > static_cast<int>(nSize)) {
		return false;
	}

	m_nSize += i;

#ifndef NDEBUG
	printf("m_nLength=%d, m_nSize=%d\n", m_nLength, m_nSize);
#endif

	return true;
}

bool PropertiesBuilder::AddIpAddress(const char *pProperty, uint32_t nValue, bool bIsSet) {
#if !defined(BUILDER_NOT_SET)
	if (!bIsSet) {
		return false;
	}
#endif

	if (m_nSize >= m_nLength) {
		return false;
	}

	char *p = reinterpret_cast<char *>(&m_pBuffer[m_nSize]);
	const uint32_t nSize = m_nLength - m_nSize;

	int i;

	if (bIsSet) {
		i = snprintf(p, nSize, "%s=" IPSTR "\n", pProperty, IP2STR(nValue));
	} else {
		i = snprintf(p, nSize, "#%s=" IPSTR "\n", pProperty, IP2STR(nValue));
	}

	if (i > static_cast<int>(nSize)) {
		return false;
	}

	m_nSize += i;

#ifndef NDEBUG
	printf("m_nLength=%d, m_nSize=%d\n", m_nLength, m_nSize);
#endif

	return true;
}

bool PropertiesBuilder::AddHex16(const char *pProperty, const uint8_t nValue[2], bool bIsSet) {
#if !defined(BUILDER_NOT_SET)
	if (!bIsSet) {
		return false;
	}
#endif

	if (m_nSize >= m_nLength) {
		return false;
	}

	char *p = reinterpret_cast<char *>(&m_pBuffer[m_nSize]);
	const uint32_t nSize = m_nLength - m_nSize;

	int i;

	if (bIsSet) {
		i = snprintf(p, nSize, "%s=%.2x%.2x\n", pProperty, nValue[0], nValue[1]);
	} else {
		i = snprintf(p, nSize, "#%s=%.2x%.2x\n", pProperty, nValue[0], nValue[1]);
	}

	if (i > static_cast<int>(nSize)) {
		return false;
	}

	m_nSize += i;

#ifndef NDEBUG
	printf("m_nLength=%d, m_nSize=%d\n", m_nLength, m_nSize);
#endif

	return true;
}

bool PropertiesBuilder::AddHex24(const char *pProperty, const uint32_t nValue, bool bIsSet) {
#if !defined(BUILDER_NOT_SET)
	if (!bIsSet) {
		return false;
	}
#endif

	if (m_nSize >= m_nLength) {
		return false;
	}

	char *p = reinterpret_cast<char *>(&m_pBuffer[m_nSize]);
	const uint32_t nSize = m_nLength - m_nSize;

	int i;

	if (bIsSet) {
		i = snprintf(p, nSize, "%s=%.6x\n", pProperty, nValue);
	} else {
		i = snprintf(p, nSize, "#%s=%.6x\n", pProperty, nValue);
	}

	if (i > static_cast<int>(nSize)) {
		return false;
	}

	m_nSize += i;

#ifndef NDEBUG
	printf("m_nLength=%d, m_nSize=%d\n", m_nLength, m_nSize);
#endif

	return true;
}
