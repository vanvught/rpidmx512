/**
 * @file artnetdiscovery.cpp
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

//#define DEBUG

#include <stdint.h>

#include "rdm_e120.h"

#include "artnetnode.h"
#include "artnetdiscovery.h"

#include "rdmmessage.h"
#include "rdmdiscovery.h"
#include "rdmdevicecontroller.h"

#include "util.h"

ArtNetDiscovery::ArtNetDiscovery(void) : m_pRdmCommand(0){
	m_Controller.Load();
	m_Discovery.SetUid(m_Controller.GetUID());

	m_pRdmCommand = new struct _rdm_command;

	if (m_pRdmCommand != 0) {
		m_pRdmCommand->start_code = E120_SC_RDM;
	}
}

/**
 *
 */
ArtNetDiscovery::~ArtNetDiscovery(void) {
	m_Discovery.Reset();
}

/**
 *
 */
void ArtNetDiscovery::Full(void) {
	m_Discovery.Full();
}

const uint8_t ArtNetDiscovery::GetUidCount(void) {
	return m_Discovery.GetUidCount();
}

void ArtNetDiscovery::Copy(unsigned char *tod) {
	m_Discovery.Copy(tod);
}

void ArtNetDiscovery::DumpTod(void) {
	m_Discovery.Dump();
}

const uint8_t *ArtNetDiscovery::Handler(const uint8_t *rdm_data) {

	if (rdm_data == 0) {
		return 0;
	}

	if (m_pRdmCommand != 0) {

		while (0 != RDMMessage::Receive()) {
			// Discard late responses
		}

		TRdmMessage *p = (TRdmMessage *) (rdm_data);
		uint8_t *c = (uint8_t *) m_pRdmCommand;

		memcpy(&c[1], rdm_data, p->message_length + 2);
#ifdef DEBUG
		RDMMessage::Print((const uint8_t *) c);
#endif
		RDMMessage::SendRaw(c, p->message_length + 2);

		return RDMMessage::ReceiveTimeOut(20000);
	}

	return 0;
}
