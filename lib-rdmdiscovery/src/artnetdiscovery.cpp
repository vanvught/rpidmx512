/**
 * @file artnetdiscovery.cpp
 *
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "hardware.h"

#include "artnetnode.h"
#include "artnetdiscovery.h"

#include "dmx_uarts.h"

#include "rdm.h"
#include "rdm_e120.h"
#include "rdmdevicecontroller.h"

#include "rdmdiscovery.h"

#include "debug.h"

ArtNetRdmController::ArtNetRdmController()  {
	for (unsigned i = 0 ; i < DMX_MAX_UARTS; i++) {
		m_Discovery[i] = new RDMDiscovery(i);
		assert(m_Discovery[i] != nullptr);
		m_Discovery[i]->SetUid(GetUID());
	}

	m_pRdmCommand = new struct TRdmMessage;
	assert(m_pRdmCommand != nullptr);

	m_pRdmCommand->start_code = E120_SC_RDM;
}

ArtNetRdmController::~ArtNetRdmController() {
	for (unsigned i = 0; i < DMX_MAX_UARTS; i++) {
		if (m_Discovery[i] != nullptr) {
			delete m_Discovery[i];
			m_Discovery[i] = nullptr;
		}
	}
}

void ArtNetRdmController::Print() {
	RDMDeviceController::Print();
}

void ArtNetRdmController::Full(uint8_t nPort) {
	assert(nPort < DMX_MAX_UARTS);

	DEBUG_PRINTF("nPort=%d", nPort);

	m_Discovery[nPort]->Full();
}

uint8_t ArtNetRdmController::GetUidCount(uint8_t nPort) {
	assert(nPort < DMX_MAX_UARTS);

	DEBUG_PRINTF("nPort=%d", nPort);

	return m_Discovery[nPort]->GetUidCount();
}

void ArtNetRdmController::Copy(uint8_t nPort, uint8_t *pTod) {
	assert(nPort < DMX_MAX_UARTS);

	DEBUG_PRINTF("nPort=%d", nPort);

	m_Discovery[nPort]->Copy(pTod);
}

void ArtNetRdmController::DumpTod(uint8_t nPort) {
	assert(nPort < DMX_MAX_UARTS);

	DEBUG_PRINTF("nPort=%d", nPort);

	m_Discovery[nPort]->Dump();
}

const uint8_t *ArtNetRdmController::Handler(uint8_t nPort, const uint8_t *pRdmData) {
	assert(nPort < DMX_MAX_UARTS);

	if (pRdmData == nullptr) {
		return nullptr;
	}

	Hardware::Get()->WatchdogFeed();

	while (nullptr != RDMMessage::Receive(nPort)) {
		// Discard late responses
	}

	const auto *pRdmMessageNoSc = reinterpret_cast<const TRdmMessageNoSc*>(const_cast<uint8_t*>(pRdmData));
	auto *pRdmCommand = reinterpret_cast<uint8_t*>(m_pRdmCommand);

	memcpy(&pRdmCommand[1], pRdmData, static_cast<size_t>(pRdmMessageNoSc->message_length + 2));

#ifndef NDEBUG
	RDMMessage::Print(pRdmCommand);
#endif

	RDMMessage::SendRaw(nPort, pRdmCommand, pRdmMessageNoSc->message_length + 2);

	const auto *pResponse = RDMMessage::ReceiveTimeOut(nPort, 20000);

#ifndef NDEBUG
	RDMMessage::Print(pResponse);
#endif
	return pResponse;
}
