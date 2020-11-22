/**
 * @file dmx++.cpp
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

#include <stdint.h>
#include <cassert>

#include "dmx.h"
#include "rdm_send.h"

#include "h3_hs_timer.h"

#include "debug.h"

void Dmx::SetPortDirection(__attribute__((unused)) uint8_t nPort, TDmxRdmPortDirection tPortDirection, bool bEnableData) {
	assert(nPort == 0);

	dmx_set_port_direction(static_cast<_dmx_port_direction>(tPortDirection), bEnableData);
}

void Dmx::RdmSendRaw(__attribute__((unused)) uint8_t nPort, const uint8_t *pRdmData, uint16_t nLength) {
	assert(nPort == 0);

	rdm_send_data(pRdmData, nLength);
}

const uint8_t *Dmx::RdmReceive(__attribute__((unused)) uint8_t nPort) {
	assert(nPort == 0);

	const uint8_t *p = rdm_get_available();
	return p;
}

const uint8_t *Dmx::RdmReceiveTimeOut(__attribute__((unused)) uint8_t nPort, uint32_t nTimeOut) {
	assert(nPort == 0);

	uint8_t *p = nullptr;
	const uint32_t nMicros = h3_hs_timer_lo_us() + nTimeOut;

	do {
		if ((p = const_cast<uint8_t *>(rdm_get_available())) != nullptr) {
			return p;
		}
	} while (h3_hs_timer_lo_us() < nMicros);

	return p;
}
