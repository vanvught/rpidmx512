/**
 * @file rdm.cpp
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

#include <cstdint>
#include <cassert>

#include "rdm.h"
#include "dmxset.h"

extern "C" {
void udelay(uint32_t);
}

const uint8_t *Rdm::Receive(uint32_t nPort) {
	return DmxSet::Get()->RdmReceive(nPort);
}

const uint8_t *Rdm::ReceiveTimeOut(uint32_t nPort, uint32_t nTimeOut) {
	return DmxSet::Get()->RdmReceiveTimeOut(nPort, nTimeOut);
}

void Rdm::SendRaw(uint32_t nPort, const uint8_t *pRdmData, uint32_t nLength) {
	assert(pRdmData != nullptr);
	assert(nLength != 0);

	DmxSet::Get()->SetPortDirection(nPort, dmx::PortDirection::OUTP, false);

	DmxSet::Get()->RdmSendRaw(nPort, pRdmData, nLength);

	udelay(RDM_RESPONDER_DATA_DIRECTION_DELAY);

	DmxSet::Get()->SetPortDirection(nPort, dmx::PortDirection::INP, true);
}
