/**
 * @file artnetrdmresponder.cpp
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

#include "artnetrdmresponder.h"
#include "rdmhandler.h"

#include "lightset.h"

#include "rdm_e120.h"

#include "rdmmessage.h"
#include "debug.h"

ArtNetRdmResponder::ArtNetRdmResponder(RDMPersonality *pRDMPersonality, LightSet *pLightSet) :
	m_Responder(pRDMPersonality, pLightSet, false),
	m_pRdmCommand(0),
	m_RDMHandler(0)
{
	m_pRdmCommand = new struct TRdmMessage;
	assert(m_pRdmCommand != 0);

	m_RDMHandler = new RDMHandler(&m_Responder);
	assert(m_RDMHandler != 0);
}

ArtNetRdmResponder::~ArtNetRdmResponder(void) {
	delete m_RDMHandler;
	m_RDMHandler = 0;

	delete m_pRdmCommand;
	m_pRdmCommand = 0;
}

void ArtNetRdmResponder::Full(uint8_t nPort) {
	// We are a Responder - no code needed
}

const uint8_t ArtNetRdmResponder::GetUidCount(uint8_t nPort) {
	return 1; // We are a Responder
}

void ArtNetRdmResponder::Copy(uint8_t nPort, unsigned char *tod) {
	unsigned char *src = (unsigned char *) m_Responder.GetUID();
	unsigned char *dst = tod;

	for (unsigned i = 0; i < RDM_UID_SIZE; i++) {
		*dst=*src;
		dst++;
		src++;
	}
}

const uint8_t *ArtNetRdmResponder::Handler(uint8_t nPort, const uint8_t *pRdmDataNoSC) {
	DEBUG_ENTRY

	if (pRdmDataNoSC == 0) {
		DEBUG_EXIT
		return 0;
	}

	m_RDMHandler->HandleData(pRdmDataNoSC, (uint8_t *)m_pRdmCommand);

	if (m_pRdmCommand->start_code != E120_SC_RDM) {
		DEBUG_EXIT
		return 0;
	}
#ifndef NDEBUG
	RDMMessage::Print((uint8_t *)m_pRdmCommand);
#endif
	DEBUG_EXIT
	return (const uint8_t *)m_pRdmCommand;
}
