#if defined (BARE_METAL)
/**
 * @file rdmresponder.cpp
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
//#ifndef NDEBUG
 #include <stdio.h>
//#endif
#include <assert.h>

#include "rdmresponder.h"

#include "lightset.h"

#include "rdm.h"
#include "rdm_e120.h"

RDMResponder::RDMResponder(RDMPersonality *pRDMPersonality, LightSet *pLightSet, uint8_t nGpioPin, bool EnableSubDevices) :
	DMXReceiver(nGpioPin),
	m_Responder(pRDMPersonality, pLightSet, EnableSubDevices),
	m_pRdmCommand(0),
	m_RDMHandler(0),
	m_IsEnableSubDevices(EnableSubDevices),
	m_IsSubDeviceActive(false)
{
	m_pRdmCommand = new struct TRdmMessage;
	assert(m_pRdmCommand !=0);

	m_RDMHandler = new RDMHandler(&m_Responder);
	assert(m_RDMHandler !=0);
}

RDMResponder::~RDMResponder(void) {
	delete m_RDMHandler;
	m_RDMHandler = 0;

	delete m_pRdmCommand;
	m_pRdmCommand = 0;
}

int RDMResponder::HandleResponse(uint8_t *pResponse) {
	uint16_t nLength = RDM_RESPONDER_INVALID_RESPONSE;

	if (pResponse[0] == E120_SC_RDM) {
		const struct TRdmMessage *p = (struct TRdmMessage *) pResponse;
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

	//DMXReceiver::Run(nLength); //TODO Test!

	const uint8_t *pDmxDataIn = DMXReceiver::Run(nLength);

	if (m_IsEnableSubDevices && (nLength == -1)) {
		if (m_IsSubDeviceActive) {
			m_Responder.GetRDMSubDevices()->Stop();
			m_IsSubDeviceActive = false;
		}
	} else if (pDmxDataIn != 0) {
		m_Responder.GetRDMSubDevices()->SetData(pDmxDataIn, (uint16_t) nLength);
		if (!m_IsSubDeviceActive) {
			m_Responder.GetRDMSubDevices()->Start();
			m_IsSubDeviceActive = true;
		}
	}

	const uint8_t *pRdmDataIn = (uint8_t *) Rdm::Receive(0);

	if (pRdmDataIn == NULL) {
		return RDM_RESPONDER_NO_DATA;
	}

#ifndef NDEBUG
	RDMMessage::Print(pRdmDataIn);
#endif

	if (pRdmDataIn[0] == E120_SC_RDM) {
		const struct TRdmMessage *pRdmCommand = (struct TRdmMessage *) pRdmDataIn;

		switch (pRdmCommand->command_class) {
		case E120_DISCOVERY_COMMAND:
		case E120_GET_COMMAND:
		case E120_SET_COMMAND:
			m_RDMHandler->HandleData(&pRdmDataIn[1], (uint8_t *)m_pRdmCommand);
			return HandleResponse((uint8_t *)m_pRdmCommand);
			break;
		default:
#ifndef NDEBUG
			printf("\t%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
#endif
			return RDM_RESPONDER_INVALID_DATA_RECEIVED;
			break;
		}
	}

#ifndef NDEBUG
	printf("\t%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
#endif
	return RDM_RESPONDER_DISCOVERY_RESPONSE;
}
#endif

