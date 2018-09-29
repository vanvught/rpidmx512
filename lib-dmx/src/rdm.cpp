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
//#include <stdio.h>
#include <assert.h>

#ifdef H3
 #include "h3_hs_timer.h"
#else
 #include "bcm2835.h"
#endif

#include "dmx.h"

#include "rdm.h"
#include "rdm_send.h"

#include "rdmmessage.h"

uint8_t Rdm::m_TransactionNumber = 0;

Rdm::Rdm(void) {

}

Rdm::~Rdm(void) {

}

const uint8_t *Rdm::Receive(void) {
	const uint8_t *p = rdm_get_available();
	return p;
}

const uint8_t *Rdm::ReceiveTimeOut(uint32_t nTimeOut) {
	uint8_t *p = NULL;
#ifdef H3
	uint32_t micros_now = h3_hs_timer_lo_us();
#else
	uint32_t micros_now = BCM2835_ST->CLO;
#endif

	do {
		if ((p = (uint8_t *)rdm_get_available()) != NULL) {
			return (const uint8_t *) p;
		}
#ifdef H3
	} while ( h3_hs_timer_lo_us() - micros_now < nTimeOut);
#else
	} while ( BCM2835_ST->CLO - micros_now < nTimeOut);
#endif

	return (const uint8_t *) p;
}

void Rdm::Send(struct TRdmMessage *pRdmCommand) {
	assert(pRdmCommand != 0);

	uint8_t *rdm_data = (uint8_t *)pRdmCommand;
	uint8_t i;
	uint16_t rdm_checksum = 0;

	pRdmCommand->transaction_number = m_TransactionNumber;

	for (i = 0; i < pRdmCommand->message_length; i++) {
		rdm_checksum += rdm_data[i];
	}

	rdm_data[i++] = rdm_checksum >> 8;
	rdm_data[i] = rdm_checksum & 0XFF;

#ifndef NDEBUG
	RDMMessage::Print((const uint8_t *)pRdmCommand);
#endif

	SendRaw((const uint8_t *)pRdmCommand, pRdmCommand->message_length + RDM_MESSAGE_CHECKSUM_SIZE);

	m_TransactionNumber++;
}

void Rdm::SendRaw(const uint8_t *pRdmData, uint16_t nLength) {
	assert(pRdmData != 0);
	assert(nLength != 0);

	dmx_set_port_direction(DMX_PORT_DIRECTION_OUTP, false);

	rdm_send_data((const uint8_t *) pRdmData, nLength);
	udelay(RDM_RESPONDER_DATA_DIRECTION_DELAY);

	dmx_set_port_direction(DMX_PORT_DIRECTION_INP, true);
}

void Rdm::SendDiscoveryRespondMessage(const uint8_t *data, uint16_t data_length) {
	rdm_send_discovery_respond_message(data, data_length);
}
