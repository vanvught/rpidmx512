/**
 * @file rdm.cpp
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "bcm2835.h"
#include "arm/pl011.h"

using namespace dmx;

uint8_t m_TransactionNumber = 0;
uint32_t m_nLastSendMicros = 0;

void Rdm::Send(uint32_t nPortIndex, struct TRdmMessage *pRdmCommand, uint32_t nSpacingMicros) {
	assert(nPort < max::OUT);
	assert(pRdmCommand != nullptr);

	if (nSpacingMicros != 0) {
		const auto nMicros = BCM2835_ST->CLO;
		const auto nDeltaMicros = nMicros - m_nLastSendMicros;
		if (nDeltaMicros < nSpacingMicros) {
			const auto nWait = nSpacingMicros - nDeltaMicros;
			do {
			} while ((BCM2835_ST->CLO - nMicros) < nWait);
		}
	}

	m_nLastSendMicros = BCM2835_ST->CLO;

	auto *rdm_data = reinterpret_cast<uint8_t*>(pRdmCommand);
	uint32_t i;
	uint16_t rdm_checksum = 0;

	pRdmCommand->transaction_number = m_TransactionNumber;

	for (i = 0; i < pRdmCommand->message_length; i++) {
		rdm_checksum += rdm_data[i];
	}

	rdm_data[i++] = static_cast<uint8_t>(rdm_checksum >> 8);
	rdm_data[i] = static_cast<uint8_t>(rdm_checksum & 0XFF);

	SendRaw(0, reinterpret_cast<const uint8_t*>(pRdmCommand), pRdmCommand->message_length + RDM_MESSAGE_CHECKSUM_SIZE);

	m_TransactionNumber++;
}

void Rdm::SendRawRespondMessage(uint32_t nPortIndex, const uint8_t *pRdmData, uint32_t nLength) {
	assert(nPort < max::OUT);
	assert(pRdmData != nullptr);
	assert(nLength != 0);

	const auto nDelay = BCM2835_ST->CLO - Dmx::Get()->RdmGetDateReceivedEnd();

	// 3.2.2 Responder Packet spacing
	if (nDelay < RDM_RESPONDER_PACKET_SPACING) {
		udelay(RDM_RESPONDER_PACKET_SPACING - nDelay);
	}

	SendRaw(nPortIndex, pRdmData, nLength);
}

void Rdm::SendDiscoveryRespondMessage(uint32_t nPortIndex, const uint8_t *pRdmData, uint32_t nLength) {
	assert(nPort < max::OUT);
	assert(pRdmData != nullptr);
	assert(nLength != 0);

	const auto nDelay = BCM2835_ST->CLO - Dmx::Get()->RdmGetDateReceivedEnd();

	// 3.2.2 Responder Packet spacing
	if (nDelay < RDM_RESPONDER_PACKET_SPACING) {
		udelay(RDM_RESPONDER_PACKET_SPACING - nDelay);
	}

	Dmx::Get()->SetPortDirection(nPortIndex, PortDirection::OUTP, false);

	BCM2835_PL011->LCRH &= ~PL011_LCRH_FEN;
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2;

	for (uint32_t i = 0; i < nLength; i++) {
		while ((BCM2835_PL011->FR & PL011_FR_TXFF) != 0)
			;
		BCM2835_PL011->DR = pRdmData[i];
	}

	while ((BCM2835_PL011->FR & PL011_FR_BUSY) != 0)
		;

	udelay(RDM_RESPONDER_DATA_DIRECTION_DELAY);

	Dmx::Get()->SetPortDirection(nPortIndex, PortDirection::INP, true);
}
