/**
 * @file dmx++.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "rdm.h"

#include "arm/pl011.h"
#include "bcm2835.h"

const uint8_t *Dmx::RdmReceive(__attribute__((unused)) uint8_t nPort) {
	assert(nPort == 0);

	const auto *p = rdm_get_available();
	return p;
}

const uint8_t* Dmx::RdmReceiveTimeOut(uint8_t nPort, uint32_t nTimeOut) {
	assert(nPort == 0);

	uint8_t *p = nullptr;
	const auto nMicros = BCM2835_ST->CLO;

	do {
		if ((p = const_cast<uint8_t*>(RdmReceive(nPort))) != 0) {
			return reinterpret_cast<const uint8_t*>(p);
		}
	} while ((BCM2835_ST->CLO - nMicros) < nTimeOut);

	return p;
}

void Dmx::SetPortDirection(__attribute__((unused)) uint8_t nPort, TDmxRdmPortDirection tPortDirection, bool bEnableData) {
	assert(nPort == 0);

	dmx_set_port_direction(static_cast<_dmx_port_direction>(tPortDirection), bEnableData);
}

void Dmx::RdmSendRaw(__attribute__((unused)) uint8_t nPort, const uint8_t *pRdmData, uint16_t nLength) {
	assert(nPort == 0);

	BCM2835_PL011->LCRH &= ~PL011_LCRH_FEN;
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2 | PL011_LCRH_BRK;
	udelay(RDM_TRANSMIT_BREAK_TIME);	// Break Time

	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2;
	udelay(RDM_TRANSMIT_MAB_TIME);		// Mark After Break

	for (uint16_t i = 0; i < nLength; i++) {
		while ((BCM2835_PL011->FR & PL011_FR_TXFF) == PL011_FR_TXFF)
			;
		BCM2835_PL011->DR = pRdmData[i];
	}

	while ((BCM2835_PL011->FR & PL011_FR_BUSY) != 0)
		;
}
