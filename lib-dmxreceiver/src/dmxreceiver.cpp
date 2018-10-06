/**
 * @file dmxreceiver.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEconst uint8_t*MENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>
#include <assert.h>

#include "dmxreceiver.h"
#include "dmx.h"

#include "debug.h"

DMXReceiver::DMXReceiver(uint8_t nGpioPin) :
	Dmx(nGpioPin, false),
	m_pLightSet(0),
	m_IsActive(false),
	m_nLength(0)
{
	DEBUG1_ENTRY

	uint32_t *p = (uint32_t *)m_Data;

	for (unsigned i = 0; i < (sizeof m_Data) / 4; i ++) {
		*p++ = 0;
	}

	DEBUG1_EXIT
}

DMXReceiver::~DMXReceiver(void) {
	DEBUG1_ENTRY

	Stop();

	m_pLightSet = 0;
	m_IsActive = false;

	DEBUG1_EXIT
}

void DMXReceiver::SetOutput(LightSet *pOutput) {
	DEBUG1_ENTRY

	assert(pOutput != 0);
	m_pLightSet = pOutput;

	DEBUG1_EXIT
}

void DMXReceiver::Start(void) {
	DEBUG1_ENTRY

	Init();
	SetPortDirection(0, DMXRDM_PORT_DIRECTION_INP, true);

	DEBUG1_EXIT
}

void DMXReceiver::Stop(void) {
	DEBUG1_ENTRY

	SetPortDirection(0, DMXRDM_PORT_DIRECTION_INP, false);
	m_pLightSet->Stop(0);

	DEBUG1_EXIT
}

bool DMXReceiver::IsDmxDataChanged(const uint8_t *pData, uint16_t nLength) {
	bool isChanged = false;

	const uint8_t *src = (uint8_t *) pData;
	uint8_t *dst = (uint8_t *) m_Data;

	if (nLength != m_nLength) {
		m_nLength = nLength;

		for (unsigned i = 0; i < DMX_DATA_BUFFER_SIZE; i++) {
			*dst++ = *src++;
		}
		return true;
	}

	for (unsigned i = 0; i < nLength; i++) {
		if (*dst != *src) {
			isChanged = true;
		}
		*dst++ = *src++;
	}

	return isChanged;
}

const uint8_t* DMXReceiver::Run(int16_t &nLength) {
	uint8_t* p = 0;

	if (GetUpdatesPerSecond() == 0) {
		if (m_IsActive) {
			m_pLightSet->Stop(0);
			m_IsActive = false;
		}

		nLength = -1;
		return 0;
	} else {
		const uint8_t *pDmx = (const uint8_t *)__builtin_assume_aligned(GetDmxAvailable(), 4);

		if (pDmx != 0) {
			const struct TDmxData *dmx_statistics = (struct TDmxData *) pDmx;
			nLength = (uint16_t) (dmx_statistics->Statistics.SlotsInPacket);

			if (IsDmxDataChanged(pDmx, nLength)) {  // Skip DMX START CODE

				DEBUG_PRINTF("\tDMX Data Changed", __FILE__, __FUNCTION__, __LINE__);

				m_pLightSet->SetData(0, ++pDmx, nLength);
				p = (uint8_t*) pDmx;
			}

			if (!m_IsActive) {
				m_pLightSet->Start(0);
				m_IsActive = true;
			}

			return p;
		}
	}

	nLength = (uint16_t) 0;
	return 0;
}
