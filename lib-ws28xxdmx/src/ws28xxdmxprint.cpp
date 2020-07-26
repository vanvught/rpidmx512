/**
 * @file ws28xxdmxprint.cpp
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdio.h>

#include "ws28xxdmx.h"

#include "ws28xx.h"

void WS28xxDmx::Print() {
	printf("Led parameters\n");
	printf(" Type  : %s [%d]\n", WS28xx::GetLedTypeString(m_tLedType), m_tLedType);
	printf(" Count : %d\n", m_nLedCount);

	if ((m_tLedType == WS2801) || (m_tLedType == APA102) || (m_tLedType == P9813)) {
		if (m_tLedType == P9813) {
			printf(" Clock : %d Hz %s {Default: %d Hz, Maximum %d Hz}\n", m_nClockSpeedHz, (m_nClockSpeedHz == 0 ? "Default" : ""), spi::speed::p9813::default_hz, spi::speed::p9813::max_hz);
		} else {
			printf(" Clock : %d Hz %s {Default: %d Hz, Maximum %d Hz}\n", m_nClockSpeedHz, (m_nClockSpeedHz == 0 ? "Default" : ""), spi::speed::ws2801::default_hz, spi::speed::ws2801::max_hz);
		}
		if (m_tLedType == APA102) {
			printf(" GlbBr : %d\n", m_nGlobalBrightness);
		}
	}
}
