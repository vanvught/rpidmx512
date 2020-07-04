#if defined (BARE_METAL)
/**
 * @file rdmresponder.cpp
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
#include <cassert>

#include "rdmdeviceresponder.h"
#include "dmxreceiver.h"

#include "rdmresponder.h"
#include "rdmsubdevices.h"

#include "lightset.h"

#include "rdm.h"
#include "rdm_e120.h"

#include "debug.h"

RDMResponder::RDMResponder(RDMPersonality *pRDMPersonality, LightSet *pLightSet, uint8_t nGpioPin) :
	DMXReceiver(nGpioPin),
	RDMDeviceResponder(pRDMPersonality, pLightSet),
	m_pRdmCommand(0),
	m_RDMHandler(0),
	m_IsSubDeviceActive(false)
{
	m_pRdmCommand = new struct TRdmMessage;
	assert(m_pRdmCommand != 0);

	m_RDMHandler = new RDMHandler;
	assert(m_RDMHandler != 0);
}

RDMResponder::~RDMResponder(void) {
	delete m_RDMHandler;
	m_RDMHandler = 0;

	delete m_pRdmCommand;
	m_pRdmCommand = 0;
}

void RDMResponder::Init(void) {
	RDMDeviceResponder::Init();
}

int RDMResponder::HandleResponse(uint8_t *pResponse) {
	int nLength = RDM_RESPONDER_INVALID_RESPONSE;

	if (pResponse[0] == E120_SC_RDM) {
		const struct TRdmMessage *p = reinterpret_cast<const struct TRdmMessage*>(pResponse);
		nLength = p->message_length + RDM_MESSAGE_CHECKSUM_SIZE;
		Rdm::SendRawRespondMessage(0, pResponse, nLength);
	} else if (pResponse[0] == 0xFE) {
		nLength = sizeof(struct TRdmDiscoveryMsg);
		Rdm::SendDiscoveryRespondMessage(pResponse, nLength);
	} else {
	}

#ifndef NDEBUG
	RDMMessage::Print(pResponse);
#endif

	return nLength;
}

int RDMResponder::Run(void) {
	int16_t nLength;

	const uint8_t *pDmxDataIn = DMXReceiver::Run(nLength);

	if (RDMSubDevices::Get()->GetCount() != 0) {
		if (nLength == -1) {
			if (m_IsSubDeviceActive) {
				RDMSubDevices::Get()->Stop();
				m_IsSubDeviceActive = false;
			}
		} else if (pDmxDataIn != 0) {
			RDMSubDevices::Get()->SetData(pDmxDataIn, static_cast<uint16_t>(nLength));
			if (!m_IsSubDeviceActive) {
				RDMSubDevices::Get()->Start();
				m_IsSubDeviceActive = true;
			}
		}
	}

	const uint8_t *pRdmDataIn = Rdm::Receive(0);

	if (pRdmDataIn == 0) {
		return RDM_RESPONDER_NO_DATA;
	}

#ifndef NDEBUG
	RDMMessage::Print(pRdmDataIn);
#endif

	if (pRdmDataIn[0] == E120_SC_RDM) {
		const struct TRdmMessage *pRdmCommand = reinterpret_cast<const struct TRdmMessage*>(pRdmDataIn);

		switch (pRdmCommand->command_class) {
		case E120_DISCOVERY_COMMAND:
		case E120_GET_COMMAND:
		case E120_SET_COMMAND:
			m_RDMHandler->HandleData(&pRdmDataIn[1], reinterpret_cast<uint8_t*>(m_pRdmCommand));
			return HandleResponse(reinterpret_cast<uint8_t*>(m_pRdmCommand));
			break;
		default:
			DEBUG_PUTS("RDM_RESPONDER_INVALID_DATA_RECEIVED");
			return RDM_RESPONDER_INVALID_DATA_RECEIVED;
			break;
		}
	}

	DEBUG_PUTS("RDM_RESPONDER_DISCOVERY_RESPONSE");
	return RDM_RESPONDER_DISCOVERY_RESPONSE;
}

void RDMResponder::Print(void) {
	RDMDeviceResponder::Print();
	DMXReceiver::Print();
}
#endif
