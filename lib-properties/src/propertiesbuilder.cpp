/**
 * @file propertiesbuilder.cpp
 *
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(__clang__)	// Needed for compiling on MacOS
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <cstring>
#include <cstdio>
#include <cassert>

#include "propertiesbuilder.h"
#include "propertiesconfig.h"

#include "network.h"

#include "debug.h"

PropertiesBuilder::PropertiesBuilder(const char *pFileName, char *pBuffer, uint32_t nLength):
	m_pBuffer(pBuffer),
	m_nLength(static_cast<uint16_t>(nLength)),
	m_bJson(PropertiesConfig::IsJSON())
{
	assert(pFileName != nullptr);
	assert(pBuffer != nullptr);

	const auto l = strlen(pFileName);

	if (m_bJson) {
		if ((l + 4) <= m_nLength) {
			pBuffer[0] = '{';
			pBuffer[1] = '\"';
			memcpy(&pBuffer[2], pFileName, l);
			pBuffer[l + 2] = '\"';
			pBuffer[l + 3] = ':';
			pBuffer[l + 4] = '{';
			m_nSize = static_cast<uint16_t>(l + 5);
		}
	} else if ((l + 2) <= m_nLength) {
		pBuffer[0] = '#';
		memcpy(&pBuffer[1], pFileName, l);
		pBuffer[l + 1] = '\n';
		m_nSize = static_cast<uint16_t>(l + 2);
	}

	DEBUG_PRINTF("m_nLength=%d, m_nSize=%d", m_nLength, m_nSize);
}

bool PropertiesBuilder::AddIpAddress(const char *pProperty, uint32_t nValue, bool bIsSet) {
	if (m_nSize >= m_nLength) {
		return false;
	}

	auto *p = &m_pBuffer[m_nSize];
	const auto nSize = static_cast<size_t>(m_nLength - m_nSize);

	int i;

	if (bIsSet || m_bJson) {
		if (m_bJson) {
			i = snprintf(p, nSize, "\"%s\":\"" IPSTR "\",", pProperty, IP2STR(nValue));
		} else {
			i = snprintf(p, nSize, "%s=" IPSTR "\n", pProperty, IP2STR(nValue));
		}
	} else {
		i = snprintf(p, nSize, "#%s=" IPSTR "\n", pProperty, IP2STR(nValue));
	}

	if (i > static_cast<int>(nSize)) {
		return false;
	}

	m_nSize = static_cast<uint16_t>(m_nSize + i);

	DEBUG_PRINTF("m_nLength=%d, m_nSize=%d", m_nLength, m_nSize);

	return true;
}

bool PropertiesBuilder::AddUtcOffset(const char *pProperty, const int8_t nHours, const uint8_t nMinutes) {
	if (m_nSize >= m_nLength) {
		return false;
	}

	auto *p = &m_pBuffer[m_nSize];
	const auto nSize = static_cast<size_t>(m_nLength - m_nSize);

	int i;

	if (m_bJson) {
		i = snprintf(p, nSize, "\"%s\":\"%s%.2d:%.2u\",", pProperty, nHours > 0 ? "+" : "", nHours, nMinutes);
	} else {
		i = snprintf(p, nSize, "%s=%s%.2d:%.2u", pProperty, nHours > 0 ? "+" : "", nHours, nMinutes);
	}

	if (i > static_cast<int>(nSize)) {
		return false;
	}

	m_nSize = static_cast<uint16_t>(m_nSize + i);

	DEBUG_PRINTF("m_nLength=%d, m_nSize=%d", m_nLength, m_nSize);
	return true;
}

bool PropertiesBuilder::AddComment(const char *pComment) {
	if (m_bJson) {
		return true;
	}

	if (m_nSize >= m_nLength) {
		return false;
	}

	auto *p = &m_pBuffer[m_nSize];
	const auto nSize = static_cast<size_t>(m_nLength - m_nSize);

	const auto i = snprintf(p, nSize, "# %s #\n", pComment);

	if (i > static_cast<int>(nSize)) {
		return false;
	}

	m_nSize = static_cast<uint16_t>(m_nSize + i);

	DEBUG_PRINTF("pComment=%s, m_nLength=%d, m_nSize=%d", pComment, m_nLength, m_nSize);

	return true;
}

bool PropertiesBuilder::AddRaw(const char *pRaw) {
	if (m_bJson) {
		return true;
	}

	if (m_nSize >= m_nLength) {
		return false;
	}

	auto *p = &m_pBuffer[m_nSize];
	const auto nSize = static_cast<size_t>(m_nLength - m_nSize);

	const auto i = snprintf(p, nSize, "%s\n", pRaw);

	if (i > static_cast<int>(nSize)) {
		return false;
	}

	m_nSize = static_cast<uint16_t>(m_nSize + i);

	DEBUG_PRINTF("pRaw=%s, m_nLength=%d, m_nSize=%d", pRaw, m_nLength, m_nSize);

	return true;
}
