/**
 * @file artnetrdmresponder.cpp
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <string.h>
#include <cassert>

#include "artnetrdmresponder.h"

#include "rdmdeviceresponder.h"
#include "rdmhandler.h"
#include "rdmmessage.h"

#include "lightset.h"

#include "rdm_e120.h"

#include "debug.h"

ArtNetRdmResponder::ArtNetRdmResponder(RDMPersonality *pRDMPersonality, LightSet *pLightSet) :
	RDMDeviceResponder(pRDMPersonality, pLightSet),
	m_pRdmCommand(nullptr),
	m_RDMHandler(nullptr)
{
	DEBUG_ENTRY

	m_pRdmCommand = new struct TRdmMessage;
	assert(m_pRdmCommand != nullptr);

	m_RDMHandler = new RDMHandler;
	assert(m_RDMHandler != nullptr);

	DEBUG_EXIT
}

ArtNetRdmResponder::~ArtNetRdmResponder() {
	DEBUG_ENTRY

	delete m_RDMHandler;
	m_RDMHandler = nullptr;

	delete m_pRdmCommand;
	m_pRdmCommand = nullptr;

	DEBUG_EXIT
}

void ArtNetRdmResponder::Full(__attribute__((unused)) uint8_t nPort) {
	// We are a Responder - no code needed
}

uint8_t ArtNetRdmResponder::GetUidCount(__attribute__((unused)) uint8_t nPort) {
	return 1; // We are a Responder
}

void ArtNetRdmResponder::Copy(__attribute__((unused)) uint8_t nPort, unsigned char *tod) {
	memcpy(tod, RDMDeviceResponder::GetUID(), RDM_UID_SIZE);
}

const uint8_t *ArtNetRdmResponder::Handler(__attribute__((unused)) uint8_t nPort, const uint8_t *pRdmDataNoSC) {
	DEBUG_ENTRY

	if (pRdmDataNoSC == nullptr) {
		DEBUG_EXIT
		return nullptr;
	}

#ifndef NDEBUG
	RDMMessage::PrintNoSc(pRdmDataNoSC);
#endif

	m_RDMHandler->HandleData(pRdmDataNoSC, reinterpret_cast<uint8_t*>(m_pRdmCommand));

	if (m_pRdmCommand->start_code != E120_SC_RDM) {
		DEBUG_EXIT
		return nullptr;
	}

#ifndef NDEBUG
	RDMMessage::Print(reinterpret_cast<uint8_t*>(m_pRdmCommand));
#endif

	DEBUG_EXIT
	return reinterpret_cast<const uint8_t*>(m_pRdmCommand);
}
