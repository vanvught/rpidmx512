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
#include "dmx.h"

#include "debug.h"

extern "C" {
void udelay(uint32_t);
}

using namespace dmx;

uint8_t Rdm::m_TransactionNumber[max::OUT] = {0, };

void Rdm::Send(uint32_t nPort, struct TRdmMessage *pRdmCommand, __attribute__((unused)) uint32_t nSpacingMicros) {
	assert(nPort < max::OUT);
	assert(pRdmCommand != nullptr);


	auto *rdm_data = reinterpret_cast<uint8_t*>(pRdmCommand);
	uint32_t i;
	uint16_t rdm_checksum = 0;

	pRdmCommand->transaction_number = m_TransactionNumber[nPort];

	for (i = 0; i < pRdmCommand->message_length; i++) {
		rdm_checksum += rdm_data[i];
	}

	rdm_data[i++] = static_cast<uint8_t>(rdm_checksum >> 8);
	rdm_data[i] = static_cast<uint8_t>(rdm_checksum & 0XFF);

	SendRaw(nPort, reinterpret_cast<const uint8_t*>(pRdmCommand), pRdmCommand->message_length + RDM_MESSAGE_CHECKSUM_SIZE);

	m_TransactionNumber[nPort]++;
}

void Rdm::SendRawRespondMessage(uint32_t nPort, const uint8_t *pRdmData, uint32_t nLength) {
	DEBUG_ENTRY

	assert(nPort < max::OUT);
	assert(pRdmData != nullptr);
	assert(nLength != 0);

	SendRaw(nPort, pRdmData, nLength);

	DEBUG_EXIT
}

void Rdm::SendDiscoveryRespondMessage(uint32_t nPort, const uint8_t *pRdmData, uint32_t nLength) {
	DEBUG_ENTRY

	assert(nPort < max::OUT);
	assert(pRdmData != nullptr);
	assert(nLength != 0);

	DmxSet::Get()->SetPortDirection(nPort, PortDirection::OUTP, false);

	SendRaw(nPort, pRdmData, nLength);

	udelay(RDM_RESPONDER_DATA_DIRECTION_DELAY);

	DmxSet::Get()->SetPortDirection(nPort, PortDirection::INP, true);

	DEBUG_EXIT
}
