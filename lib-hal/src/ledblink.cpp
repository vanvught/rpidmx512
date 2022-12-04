/**
 * @file ledblink.cpp
 *
 */
/* Copyright (C) 2017-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cassert>

#include "ledblink.h"
#include "hardware.h"

#include "debug.h"

enum class FreqMode {
	OFF_OFF = 0, NORMAL = 1, DATA = 3, FAST = 5, OFF_ON = 255
};

LedBlink *LedBlink::s_pThis = nullptr;

LedBlink::LedBlink() {
	assert(s_pThis == nullptr);
	s_pThis = this;

	PlatformInit();
}

void LedBlink::SetMode(ledblink::Mode mode) {
	if (m_Mode == mode) {
		return;
	}

	m_Mode = mode;

	switch (m_Mode) {
	case ledblink::Mode::OFF_OFF:
		SetFrequency(static_cast<uint32_t>(FreqMode::OFF_OFF));
		break;
	case ledblink::Mode::OFF_ON:
		SetFrequency(static_cast<uint32_t>(FreqMode::OFF_ON));
		break;
	case ledblink::Mode::NORMAL:
		SetFrequency(static_cast<uint32_t>(FreqMode::NORMAL));
		break;
	case ledblink::Mode::DATA:
		SetFrequency(static_cast<uint32_t>(FreqMode::DATA));
		break;
	case ledblink::Mode::FAST:
		SetFrequency(static_cast<uint32_t>(FreqMode::FAST));
		break;
	default:
		SetFrequency(static_cast<uint32_t>(FreqMode::OFF_OFF));
		break;
	}

	if (m_pLedBlinkDisplay != nullptr) {
		m_pLedBlinkDisplay->Print(static_cast<uint32_t>(m_Mode));
	}

	DEBUG_PRINTF("Mode=%d", static_cast<int>(m_Mode));
}
