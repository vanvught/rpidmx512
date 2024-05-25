/**
 * @file propertiesbuilderaddhex.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstddef>
#include <cassert>

#include "propertiesbuilder.h"

#include "debug.h"

bool PropertiesBuilder::AddHex(const char *pProperty, uint32_t nValue, const bool bIsSet, const int nWidth) {
	if (m_nSize >= m_nLength) {
		return false;
	}

	auto *p = &m_pBuffer[m_nSize];
	const auto nSize = static_cast<size_t>(m_nLength - m_nSize);

	int i;

	if (bIsSet || m_bJson) {
		if (m_bJson) {
			i = snprintf(p, nSize, "\"%s\":\"%.*x\",", pProperty, static_cast<int>(nWidth), static_cast<unsigned int>(nValue));
		} else {
			i = snprintf(p, nSize, "%s=%.*x\n", pProperty, static_cast<int>(nWidth), static_cast<unsigned int>(nValue));
		}
	} else {
		i = snprintf(p, nSize, "#%s=%.*x\n", pProperty, static_cast<int>(nWidth), static_cast<unsigned int>(nValue));
	}

	if (i > static_cast<int>(nSize)) {
		return false;
	}

	m_nSize = static_cast<uint16_t>(m_nSize + i);

	DEBUG_PRINTF("m_nLength=%d, m_nSize=%d", m_nLength, m_nSize);

	return true;
}
