/**
 * @file rdm.cpp
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "dmx.h"
#include "dmx_multi_internal.h"
#include "rdm.h"

#include "h3_timer.h"
#include "h3_hs_timer.h"

#include "uart.h"

#include "debug.h"

uint8_t Rdm::m_TransactionNumber[DMX_MAX_OUT] = {0, };
uint32_t Rdm::m_nLastSendMicros[DMX_MAX_OUT] = {0, };

const uint8_t *Rdm::Receive(uint32_t nPort) {
	return DmxSet::Get()->RdmReceive(nPort);
}

const uint8_t *Rdm::ReceiveTimeOut(uint32_t nPort, uint32_t nTimeOut) {
	return DmxSet::Get()->RdmReceiveTimeOut(nPort, nTimeOut);
}

void Rdm::Send(uint32_t nPort, struct TRdmMessage *pRdmCommand, uint32_t nSpacingMicros) {
	assert(nPort < DMX_MAX_OUT);
	assert(pRdmCommand != nullptr);

	if (nSpacingMicros != 0) {
		const auto nMicros = H3_TIMER->AVS_CNT1;
		const auto nDeltaMicros = nMicros - m_nLastSendMicros[nPort];
		if (nDeltaMicros < nSpacingMicros) {
			const auto nWait = nSpacingMicros - nDeltaMicros;
			do {
			} while ((H3_TIMER->AVS_CNT1 - nMicros) < nWait);
		}
	}

	m_nLastSendMicros[nPort] = H3_TIMER->AVS_CNT1;

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

void Rdm::SendRaw(uint32_t nPort, const uint8_t *pRdmData, uint16_t nLength) {
	assert(nPort < DMX_MAX_OUT);
	assert(pRdmData != nullptr);
	assert(nLength != 0);

	DmxSet::Get()->SetPortDirection(nPort, DMXRDM_PORT_DIRECTION_OUTP, false);

	DmxSet::Get()->RdmSendRaw(nPort, pRdmData, nLength);

	udelay(RDM_RESPONDER_DATA_DIRECTION_DELAY);

	DmxSet::Get()->SetPortDirection(nPort, DMXRDM_PORT_DIRECTION_INP, true);
}

void Rdm::SendRawRespondMessage(uint32_t nPort, const uint8_t *pRdmData, uint16_t nLength) {
	assert(pRdmData != nullptr);
	assert(nLength != 0);

	const auto nDelay = h3_hs_timer_lo_us() - rdm_get_data_receive_end();

	// 3.2.2 Responder Packet spacing
	if (nDelay < RDM_RESPONDER_PACKET_SPACING) {
		udelay(RDM_RESPONDER_PACKET_SPACING - nDelay);
	}

	SendRaw(nPort, pRdmData, nLength);
}

void Rdm::SendDiscoveryRespondMessage(uint32_t nPort, const uint8_t *pRdmData, uint16_t nLength) {
	const auto nDelay = h3_hs_timer_lo_us() - rdm_get_data_receive_end();

	// 3.2.2 Responder Packet spacing
	if (nDelay < RDM_RESPONDER_PACKET_SPACING) {
		udelay(RDM_RESPONDER_PACKET_SPACING - nDelay);
	}

	DmxSet::Get()->SetPortDirection(nPort, DMXRDM_PORT_DIRECTION_OUTP, false);

	auto *p = _get_uart(_port_to_uart(nPort));
	p->LCR = UART_LCR_8_N_2;

	for (uint32_t i = 0; i < nLength; i++) {
		while (!(p->LSR & UART_LSR_THRE))
				;
		EXT_UART->O00.THR = pRdmData[i];
	}

	while (!((p->LSR & UART_LSR_TEMT) == UART_LSR_TEMT))
		;

	udelay(RDM_RESPONDER_DATA_DIRECTION_DELAY);

	DmxSet::Get()->SetPortDirection(nPort, DMXRDM_PORT_DIRECTION_INP, true);
}
