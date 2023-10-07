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

#include "dmxconst.h"
#include "dmx_internal.h"

#include "h3_board.h"
#include "h3_timer.h"
#include "h3_hs_timer.h"
#include "h3_uart.h"

uint8_t s_TransactionNumber[dmx::config::max::OUT];
uint32_t s_nLastSendMicros[dmx::config::max::OUT];

void Rdm::Send(uint32_t nPortIndex, struct TRdmMessage *pRdmCommand, uint32_t nSpacingMicros) {
	assert(nPortIndex < max::OUT);
	assert(pRdmCommand != nullptr);

	if (nSpacingMicros != 0) {
		const auto nMicros = H3_TIMER->AVS_CNT1;
		const auto nDeltaMicros = nMicros - s_nLastSendMicros[nPortIndex];
		if (nDeltaMicros < nSpacingMicros) {
			const auto nWait = nSpacingMicros - nDeltaMicros;
			do {
			} while ((H3_TIMER->AVS_CNT1 - nMicros) < nWait);
		}
	}

	s_nLastSendMicros[nPortIndex] = H3_TIMER->AVS_CNT1;

	auto *rdm_data = reinterpret_cast<uint8_t*>(pRdmCommand);
	uint32_t i;
	uint16_t rdm_checksum = 0;

	pRdmCommand->transaction_number = s_TransactionNumber[nPortIndex];

	for (i = 0; i < pRdmCommand->message_length; i++) {
		rdm_checksum = static_cast<uint16_t>(rdm_checksum + rdm_data[i]);
	}

	rdm_data[i++] = static_cast<uint8_t>(rdm_checksum >> 8);
	rdm_data[i] = static_cast<uint8_t>(rdm_checksum & 0XFF);

	SendRaw(nPortIndex, reinterpret_cast<const uint8_t*>(pRdmCommand), pRdmCommand->message_length + RDM_MESSAGE_CHECKSUM_SIZE);

	s_TransactionNumber[nPortIndex]++;
}

void Rdm::SendRawRespondMessage(uint32_t nPortIndex, const uint8_t *pRdmData, uint32_t nLength) {
	assert(nPortIndex < max::OUT);
	assert(pRdmData != nullptr);
	assert(nLength != 0);

	const auto nDelay = h3_hs_timer_lo_us() - Dmx::Get()->RdmGetDateReceivedEnd();

	// 3.2.2 Responder Packet spacing
	if (nDelay < RDM_RESPONDER_PACKET_SPACING) {
		udelay(RDM_RESPONDER_PACKET_SPACING - nDelay);
	}

	SendRaw(nPortIndex, pRdmData, nLength);
}

void Rdm::SendDiscoveryRespondMessage(uint32_t nPortIndex, const uint8_t *pRdmData, uint32_t nLength) {
	assert(nPortIndex < max::OUT);
	assert(pRdmData != nullptr);
	assert(nLength != 0);

	const auto nDelay = h3_hs_timer_lo_us() - Dmx::Get()->RdmGetDateReceivedEnd();

	// 3.2.2 Responder Packet spacing
	if (nDelay < RDM_RESPONDER_PACKET_SPACING) {
		udelay(RDM_RESPONDER_PACKET_SPACING - nDelay);
	}

	Dmx::Get()->SetPortDirection(nPortIndex, dmx::PortDirection::OUTP, false);

	auto *p = _get_uart(_port_to_uart(nPortIndex));
	p->LCR = UART_LCR_8_N_2;

	for (uint32_t i = 0; i < nLength; i++) {
		while (!(p->LSR & UART_LSR_THRE))
				;
		EXT_UART->O00.THR = pRdmData[i];
	}

	while (!((p->LSR & UART_LSR_TEMT) == UART_LSR_TEMT))
		;

	udelay(RDM_RESPONDER_DATA_DIRECTION_DELAY);

	Dmx::Get()->SetPortDirection(nPortIndex, dmx::PortDirection::INP, true);
}
