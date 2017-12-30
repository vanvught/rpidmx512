/**
 * @file dmxreceiver.cpp
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>
#include <assert.h>

#include "dmxreceiver.h"

#include "dmxrdm.h"

DMXReceiver::DMXReceiver(uint8_t nGpioPin) : m_pLightSet(0), m_IsActive(false), m_nLength(0) {
}

DMXReceiver::~DMXReceiver(void) {
	Stop();
	m_pLightSet = 0;
}

void DMXReceiver::SetOutput(LightSet *pOutput) {
	assert(pOutput != 0);

	m_pLightSet = pOutput;
}

void DMXReceiver::Start(void) {
	SetPortDirection(DMXRDM_PORT_DIRECTION_INP, true);
}

void DMXReceiver::Stop(void) {
	SetPortDirection(DMXRDM_PORT_DIRECTION_INP, false);
	m_pLightSet->Stop();
}

bool DMXReceiver::IsDmxDataChanged(const uint8_t *pData, uint16_t nLength) {
	bool isChanged = false;

	uint8_t *src = (uint8_t *) pData;
	uint8_t *dst = (uint8_t *) m_Data;

	if (nLength != m_nLength) {
		m_nLength = nLength;

		for (unsigned i = 0 ; i < DMX_UNIVERSE_SIZE; i++) {
			*dst++ = *src++;
		}
		return true;
	}

	for (unsigned i = 0; i < DMX_UNIVERSE_SIZE; i++) {
		if (*dst != *src) {
			*dst = *src;
			isChanged = true;
		}
		dst++;
		src++;
	}

	return isChanged;

}

int DMXReceiver::Run(void) {
	if (GetUpdatesPerSecond() == 0) {
		if (m_IsActive) {
			m_pLightSet->Stop();
			m_IsActive = false;
		}
		return -1;
	} else {
		const uint8_t *p = GetDmxAvailable();

		if (p != 0) {
			const struct TDmxData *dmx_statistics = (struct TDmxData *) p;
			const uint16_t length = (uint16_t) (dmx_statistics->Statistics.SlotsInPacket);

			if (IsDmxDataChanged(++p, length)) {  // Skip DMX START CODE
				m_pLightSet->SetData(0, p, length);
			}

			if (!m_IsActive) {
				m_pLightSet->Start();
				m_IsActive = true;
			}

			return (int) length;

		}
	}

	return 0;
}
