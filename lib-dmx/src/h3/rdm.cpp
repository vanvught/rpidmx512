/**
 * @file rdm.cpp
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "dmx.h"

#include "rdm.h"
#include "rdm_send.h"

#include "h3_hs_timer.h"

#include "debug.h"

uint8_t Rdm::m_TransactionNumber[DMX_MAX_OUT] = {0, };

Rdm::Rdm(void) {
	assert(DmxSet::Get() != 0);
}

Rdm::~Rdm(void) {

}

const uint8_t *Rdm::Receive(uint8_t nPort) {
	return DmxSet::Get()->RdmReceive(nPort);
}

const uint8_t *Rdm::ReceiveTimeOut(uint8_t nPort, uint32_t nTimeOut) {
	return DmxSet::Get()->RdmReceiveTimeOut(nPort, nTimeOut);
}

void Rdm::Send(uint8_t nPort, struct TRdmMessage *pRdmCommand) {
	DEBUG_ENTRY

	assert(nPort < DMX_MAX_OUT);
	assert(pRdmCommand != 0);

	uint8_t *rdm_data = (uint8_t *)pRdmCommand;
	uint8_t i;
	uint16_t rdm_checksum = 0;

	pRdmCommand->transaction_number = m_TransactionNumber[nPort];

	for (i = 0; i < pRdmCommand->message_length; i++) {
		rdm_checksum += rdm_data[i];
	}

	rdm_data[i++] = rdm_checksum >> 8;
	rdm_data[i] = rdm_checksum & 0XFF;

	SendRaw(nPort, (const uint8_t *)pRdmCommand, pRdmCommand->message_length + RDM_MESSAGE_CHECKSUM_SIZE);

	m_TransactionNumber[nPort]++;

	DEBUG_EXIT
}

void Rdm::SendRaw(uint8_t nPort, const uint8_t *pRdmData, uint16_t nLength) {
	DEBUG_ENTRY

	assert(nPort < DMX_MAX_OUT);
	assert(pRdmData != 0);
	assert(nLength != 0);

	DmxSet::Get()->SetPortDirection(nPort, DMXRDM_PORT_DIRECTION_OUTP, false);

	DmxSet::Get()->RdmSendRaw(nPort, (const uint8_t *) pRdmData, nLength);

	udelay(RDM_RESPONDER_DATA_DIRECTION_DELAY);

	DmxSet::Get()->SetPortDirection(nPort, DMXRDM_PORT_DIRECTION_INP, true);

	DEBUG_EXIT
}

void Rdm::SendRawRespondMessage(uint8_t nPort, const uint8_t *pRdmData, uint16_t nLength) {
	assert(pRdmData != 0);
	assert(nLength != 0);

	const uint32_t delay = h3_hs_timer_lo_us() - rdm_get_data_receive_end();

	// 3.2.2 Responder Packet spacing
	if (delay < RDM_RESPONDER_PACKET_SPACING) {
		udelay(RDM_RESPONDER_PACKET_SPACING - delay);
	}

	SendRaw(nPort, pRdmData, nLength);
}

void Rdm::SendDiscoveryRespondMessage(const uint8_t *data, uint16_t data_length) {
	DEBUG_ENTRY

	rdm_send_discovery_respond_message(data, data_length);

	DEBUG_EXIT
}
