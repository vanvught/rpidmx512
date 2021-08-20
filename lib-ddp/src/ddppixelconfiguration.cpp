/**
 * @file ddppixelconfiguration.cpp
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <algorithm>
#include <cstdint>
#ifndef NDEBUG
# include <cstdio>
#endif
#include <cassert>

#include "ddpdisplaypixelconfiguration.h"

void DdpDisplayPixelConfiguration::Validate(uint32_t& nLedsPerPixel) {
	auto nCountMax = m_nCount[0];

	for (uint32_t nPortIndex = 1; nPortIndex < ddpdisplay::configuration::pixel::MAX_PORTS; nPortIndex++) {
		nCountMax = std::max(nCountMax, m_nCount[nPortIndex]);
	}

	PixelConfiguration::SetCount(nCountMax);
	PixelConfiguration::Validate(nLedsPerPixel);

	for (uint32_t nPortIndex = 0; nPortIndex < ddpdisplay::configuration::pixel::MAX_PORTS; nPortIndex++) {
		m_nCount[nPortIndex] = std::min(PixelConfiguration::GetCount(), m_nCount[nPortIndex]);
	}
}

void DdpDisplayPixelConfiguration::Dump() {
#ifndef NDEBUG
	PixelConfiguration::Dump();

	for (uint32_t nPortIndex = 0; nPortIndex < ddpdisplay::configuration::pixel::MAX_PORTS; nPortIndex++) {
		printf("m_nCount[%u]=%u\n", nPortIndex, m_nCount[nPortIndex]);
	}
#endif
}
