/**
 * @file dmxreceiver.cpp
 *
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "dmxreceiver.h"
#include "dmx.h"

#include "debug.h"

DMXReceiver::DMXReceiver(uint8_t nGpioPin) : Dmx(nGpioPin, false) {
	DEBUG1_ENTRY

	auto *p = reinterpret_cast<uint32_t*>(m_Data);

	for (uint32_t i = 0; i < (sizeof m_Data) / 4; i ++) {
		*p++ = 0;
	}

	DEBUG1_EXIT
}

DMXReceiver::~DMXReceiver() {
	DEBUG1_ENTRY

	Stop();

	m_pLightSet = nullptr;
	m_IsActive = false;

	DEBUG1_EXIT
}

void DMXReceiver::SetOutput(LightSet *pLightSet) {
	DEBUG1_ENTRY

	assert(pLightSet != nullptr);
	m_pLightSet = pLightSet;

	DEBUG1_EXIT
}

void DMXReceiver::Start() {
	DEBUG1_ENTRY

	Init();
	SetPortDirection(0, DMXRDM_PORT_DIRECTION_INP, true);

	DEBUG1_EXIT
}

void DMXReceiver::Stop() {
	DEBUG1_ENTRY

	SetPortDirection(0, DMXRDM_PORT_DIRECTION_INP, false);
	m_pLightSet->Stop(0);

	DEBUG1_EXIT
}

bool DMXReceiver::IsDmxDataChanged(const uint8_t *pData, uint16_t nLength) {
	bool isChanged = false;

	const uint8_t *pSrc = pData;
	auto *pDst = const_cast<uint8_t*>(m_Data);

	if (nLength != m_nLength) {
		m_nLength = nLength;

		for (uint32_t i = 0; i < nLength; i++) {
			*pDst++ = *pSrc++;
		}

		return true;
	}

	for (uint32_t i = 0; i < nLength; i++) {
		if (*pDst != *pSrc) {
			isChanged = true;
		}
		*pDst++ = *pSrc++;
	}

	return isChanged;
}

const uint8_t* DMXReceiver::Run(int16_t &nLength) {
	uint8_t* p = nullptr;

	if (GetUpdatesPerSecond() == 0) {
		if (m_IsActive) {
			m_pLightSet->Stop(0);
			m_IsActive = false;
		}

		nLength = -1;
		return nullptr;
	} else {
		const uint8_t *pDmx = GetDmxAvailable();

		if (pDmx != nullptr) {
			const auto *dmx_statistics = reinterpret_cast<const struct TDmxData*>(pDmx);
			nLength = dmx_statistics->Statistics.SlotsInPacket;

			if (IsDmxDataChanged(++pDmx, static_cast<uint16_t>(nLength))) {  // Skip DMX START CODE

				DEBUG_PRINTF("\tDMX Data Changed", __FILE__, __FUNCTION__, __LINE__);

				m_pLightSet->SetData(0, pDmx, static_cast<uint16_t>(nLength));
				p = const_cast<uint8_t*>(pDmx);
			}

			if (!m_IsActive) {
				m_pLightSet->Start(0);
				m_IsActive = true;
			}

			return p;
		}
	}

	nLength = 0;
	return nullptr;
}

void DMXReceiver::Print() {

}
