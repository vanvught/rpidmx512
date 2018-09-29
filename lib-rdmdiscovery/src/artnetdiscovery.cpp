/**
 * @file artnetdiscovery.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "artnetnode.h"
#include "artnetdiscovery.h"

#include "rdm.h"
#include "rdm_e120.h"
#include "rdmdevicecontroller.h"

#include "rdmdiscovery.h"

#include "util.h"

#include "debug.h"

ArtNetRdmController::ArtNetRdmController(void) : m_pRdmCommand(0){
	m_Controller.Load();

	for (unsigned i = 0 ; i < DMX_MAX_UARTS; i++) {
		m_Discovery[i] = new RDMDiscovery(i);
		assert(m_Discovery[i] != 0);
		m_Discovery[i]->SetUid(m_Controller.GetUID());
	}

	m_pRdmCommand = new struct TRdmMessage;

	assert(m_pRdmCommand != 0);

	m_pRdmCommand->start_code = E120_SC_RDM;
}

ArtNetRdmController::~ArtNetRdmController(void) {
	for (unsigned i = 0; i < DMX_MAX_UARTS; i++) {
		if (m_Discovery[i] != 0) {
			delete m_Discovery[i];
			m_Discovery[i] = 0;
		}
	}
}

void ArtNetRdmController::Full(uint8_t nPort) {
	assert(nPort < DMX_MAX_UARTS);

	DEBUG_PRINTF("nPort=%d", nPort);

	m_Discovery[nPort]->Full();
}

const uint8_t ArtNetRdmController::GetUidCount(uint8_t nPort) {
	assert(nPort < DMX_MAX_UARTS);

	DEBUG_PRINTF("nPort=%d", nPort);

	return m_Discovery[nPort]->GetUidCount();
}

void ArtNetRdmController::Copy(uint8_t nPort, unsigned char *tod) {
	assert(nPort < DMX_MAX_UARTS);

	DEBUG_PRINTF("nPort=%d", nPort);

	m_Discovery[nPort]->Copy(tod);
}

void ArtNetRdmController::DumpTod(uint8_t nPort) {
	assert(nPort < DMX_MAX_UARTS);

	DEBUG_PRINTF("nPort=%d", nPort);

	m_Discovery[nPort]->Dump();
}

const uint8_t *ArtNetRdmController::Handler(uint8_t nPort, const uint8_t *rdm_data) {
	assert(nPort < DMX_MAX_UARTS);

	if (rdm_data == 0) {
		return 0;
	}

	while (0 != RDMMessage::Receive(nPort)) {
		// Discard late responses
	}

	TRdmMessageNoSc *p = (TRdmMessageNoSc *) (rdm_data);
	uint8_t *c = (uint8_t *) m_pRdmCommand;

	memcpy(&c[1], rdm_data, p->message_length + 2);

#ifndef NDEBUG
	RDMMessage::Print((const uint8_t *) c);
#endif

	RDMMessage::SendRaw(nPort, c, p->message_length + 2);

	const uint8_t *pResponse = RDMMessage::ReceiveTimeOut(nPort, 20000);

#ifndef NDEBUG
	RDMMessage::Print(pResponse);
#endif
	return pResponse;

}
