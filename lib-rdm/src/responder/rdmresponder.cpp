/**
 * @file rdmresponder.cpp
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

#include "rdmresponder.h"
#include "rdmdeviceresponder.h"
#include "rdmsubdevices.h"

#include "dmxreceiver.h"

#include "lightset.h"

#include "rdm.h"
#include "rdm_e120.h"

#include "debug.h"

namespace configstore {
void delay();
}  // namespace configstore

RDMResponder *RDMResponder::s_pThis;
TRdmMessage RDMResponder::s_RdmCommand;
bool RDMResponder::m_IsSubDeviceActive;

using namespace rdm::responder;

int RDMResponder::HandleResponse(uint8_t *pResponse) {
	auto nLength = INVALID_RESPONSE;

	if (pResponse[0] == E120_SC_RDM) {
		const auto *p = reinterpret_cast<const struct TRdmMessage*>(pResponse);
		nLength = static_cast<int>(p->message_length + RDM_MESSAGE_CHECKSUM_SIZE);
		Rdm::SendRawRespondMessage(0, pResponse, static_cast<uint16_t>(nLength));
	} else if (pResponse[0] == 0xFE) {
		nLength = sizeof(struct TRdmDiscoveryMsg);
		Rdm::SendDiscoveryRespondMessage(0, pResponse, static_cast<uint16_t>(nLength));
	}

#ifndef NDEBUG
	if (nLength != INVALID_RESPONSE) {
		RDMMessage::Print(pResponse);
	}
#endif

	configstore::delay();
	return nLength;
}

int RDMResponder::Run() {
	int16_t nLength;

	const auto *pDmxDataIn = DMXReceiver::Run(nLength);

	if (RDMSubDevices::Get()->GetCount() != 0) {
		if (nLength == -1) {
			if (m_IsSubDeviceActive) {
				RDMSubDevices::Get()->Stop();
				m_IsSubDeviceActive = false;
			}
		} else if (pDmxDataIn != nullptr) {
			RDMSubDevices::Get()->SetData(pDmxDataIn, static_cast<uint16_t>(nLength));
			if (!m_IsSubDeviceActive) {
				RDMSubDevices::Get()->Start();
				m_IsSubDeviceActive = true;
			}
		}
	}

	const auto *pRdmDataIn = Rdm::Receive(0);

	if (pRdmDataIn == nullptr) {
		return NO_DATA;
	}

#ifndef NDEBUG
	RDMMessage::Print(pRdmDataIn);
#endif

	if (pRdmDataIn[0] == E120_SC_RDM) {
		const auto *pRdmCommand = reinterpret_cast<const struct TRdmMessage*>(pRdmDataIn);

		switch (pRdmCommand->command_class) {
		case E120_DISCOVERY_COMMAND:
		case E120_GET_COMMAND:
		case E120_SET_COMMAND:
			HandleData(&pRdmDataIn[1], reinterpret_cast<uint8_t*>(&s_RdmCommand));
			return HandleResponse(reinterpret_cast<uint8_t*>(&s_RdmCommand));
			break;
		default:
			DEBUG_PUTS("RDM_RESPONDER_INVALID_DATA_RECEIVED");
			return INVALID_DATA_RECEIVED;
			break;
		}
	}

	DEBUG_PUTS("RDM_RESPONDER_DISCOVERY_RESPONSE");
	return DISCOVERY_RESPONSE;
}

void __attribute__((weak)) RDMResponder::PersonalityUpdate(__attribute__((unused)) uint32_t nPersonality)  {
}

void __attribute__((weak))DmxStartAddressUpdate(__attribute__((unused)) uint16_t nDmxStartAddress) {
}
