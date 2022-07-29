/**
 * @file artnetdiscovery.cpp
 *
 */
/* Copyright (C) 2017-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

RDMTod *ArtNetRdmController::m_pRDMTod[artnetnode::MAX_PORTS];
TRdmMessage ArtNetRdmController::s_rdmMessage;
uint32_t ArtNetRdmController::s_nPorts;

ArtNetRdmController::ArtNetRdmController(uint32_t nPorts): RDMDiscovery(RDMDeviceController::GetUID()) {
	DEBUG_ENTRY
	assert(nPorts <= artnetnode::MAX_PORTS);

	s_nPorts = nPorts;
	uint32_t nPortIndex;

	for (nPortIndex = 0; nPortIndex < nPorts; nPortIndex++) {
		m_pRDMTod[nPortIndex] = new RDMTod;
		assert(m_pRDMTod[nPortIndex] != nullptr);
	}

	for (; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		m_pRDMTod[nPortIndex] = nullptr;
	}

	s_rdmMessage.start_code = E120_SC_RDM;
	DEBUG_EXIT
}

const uint8_t *ArtNetRdmController::Handler(uint32_t nPortIndex, const uint8_t *pRdmData) {
	assert(nPortIndex < s_nPorts);

	if (pRdmData == nullptr) {
		return nullptr;
	}

	Hardware::Get()->WatchdogFeed();

	while (nullptr != RDMMessage::Receive(nPortIndex)) {
		// Discard late responses
		Hardware::Get()->WatchdogFeed();
	}

	const auto *pRdmMessageNoSc = reinterpret_cast<const TRdmMessageNoSc*>(const_cast<uint8_t*>(pRdmData));
	auto *pRdmCommand = reinterpret_cast<uint8_t*>(&s_rdmMessage);

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
