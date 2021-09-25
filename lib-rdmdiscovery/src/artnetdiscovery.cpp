/**
 * @file artnetdiscovery.cpp
 *
 */
/* Copyright (C) 2017-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstring>
#include <cassert>

#include "hardware.h"

#include "artnetnode.h"
#include "artnetdiscovery.h"

#include "rdm.h"
#include "rdm_e120.h"
#include "rdmdevicecontroller.h"

#include "rdmdiscovery.h"

#include "debug.h"

ArtNetRdmController::ArtNetRdmController()  {
	for (uint8_t i = 0 ; i < artnetnode::MAX_PORTS; i++) {
		m_Discovery[i] = new RDMDiscovery(i);
		assert(m_Discovery[i] != nullptr);
		m_Discovery[i]->SetUid(GetUID());
	}

	m_pRdmCommand = new struct TRdmMessage;
	assert(m_pRdmCommand != nullptr);

	m_pRdmCommand->start_code = E120_SC_RDM;
}

ArtNetRdmController::~ArtNetRdmController() {
	for (uint32_t i = 0; i < artnetnode::MAX_PORTS; i++) {
		if (m_Discovery[i] != nullptr) {
			delete m_Discovery[i];
			m_Discovery[i] = nullptr;
		}
	}
}

void ArtNetRdmController::Print() {
	RDMDeviceController::Print();
}

void ArtNetRdmController::Full(uint32_t nPortIndex) {
	DEBUG_ENTRY
	assert(nPortIndex < artnetnode::MAX_PORTS);

	m_Discovery[nPortIndex]->Full();

	DEBUG_PRINTF("nPortIndex=%d", nPortIndex);
	DEBUG_EXIT
}

uint8_t ArtNetRdmController::GetUidCount(uint32_t nPortIndex) {
	assert(nPortIndex < artnetnode::MAX_PORTS);

	DEBUG_PRINTF("nPortIndex=%d", nPortIndex);

	return m_Discovery[nPortIndex]->GetUidCount();
}

void ArtNetRdmController::Copy(uint32_t nPortIndex, uint8_t *pTod) {
	DEBUG_ENTRY
	assert(nPortIndex < artnetnode::MAX_PORTS);

	m_Discovery[nPortIndex]->Copy(pTod);

	DEBUG_PRINTF("nPortIndex=%d", nPortIndex);
	DEBUG_EXIT
}

void ArtNetRdmController::DumpTod(uint32_t nPortIndex) {
	DEBUG_ENTRY
	assert(nPortIndex < artnetnode::MAX_PORTS);

	m_Discovery[nPortIndex]->Dump();

	DEBUG_PRINTF("nPortIndex=%d", nPortIndex);
	DEBUG_EXIT
}

const uint8_t *ArtNetRdmController::Handler(uint32_t nPortIndex, const uint8_t *pRdmData) {
	assert(nPortIndex < artnetnode::MAX_PORTS);

	if (pRdmData == nullptr) {
		return nullptr;
	}

	Hardware::Get()->WatchdogFeed();

	while (nullptr != RDMMessage::Receive(nPortIndex)) {
		// Discard late responses
		Hardware::Get()->WatchdogFeed();
	}

	const auto *pRdmMessageNoSc = reinterpret_cast<const TRdmMessageNoSc*>(const_cast<uint8_t*>(pRdmData));
	auto *pRdmCommand = reinterpret_cast<uint8_t*>(m_pRdmCommand);

	memcpy(&pRdmCommand[1], pRdmData, static_cast<size_t>(pRdmMessageNoSc->message_length + 2));

#ifndef NDEBUG
	RDMMessage::Print(pRdmCommand);
#endif

	RDMMessage::SendRaw(nPortIndex, pRdmCommand, pRdmMessageNoSc->message_length + 2U);

	const auto *pResponse = RDMMessage::ReceiveTimeOut(nPortIndex, 60000);

#ifndef NDEBUG
	RDMMessage::Print(pResponse);
#endif
	return pResponse;
}
