/**
 * @file rdmnetdevice.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <stdio.h>
#include <uuid/uuid.h>
#include <assert.h>

#include "rdmnetdevice.h"

#include "llrpdevice.h"
#include "rdmpersonality.h"
#include "lightset.h"
#include "rdmdeviceresponder.h"
#include "rdmhandler.h"
#include "e131uuid.h"

#include "debug.h"

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#define UUID_STRING_LENGTH	36

RDMNetDevice::RDMNetDevice(RDMPersonality *pRDMPersonality) :
	RDMDeviceResponder(pRDMPersonality, LightSet::Get(), false),
	m_RDMHandler(0),
	m_pRdmCommand(0)
{
	DEBUG_ENTRY

	m_E131Uuid.GetHardwareUuid(m_Cid);

	m_pRdmCommand = new struct TRdmMessage;
	assert(m_pRdmCommand != 0);

	m_RDMHandler = new RDMHandler(false);
	assert(m_RDMHandler != 0);

	DEBUG_EXIT
}

RDMNetDevice::~RDMNetDevice(void) {
	DEBUG_ENTRY

	delete m_RDMHandler;
	m_RDMHandler = 0;

	delete m_pRdmCommand;
	m_pRdmCommand = 0;

	DEBUG_EXIT
}

void RDMNetDevice::Start(void) {
	DEBUG_ENTRY

	LLRPDevice::Start();

	DEBUG_EXIT
}

void RDMNetDevice::Stop(void) {
	DEBUG_ENTRY

	LLRPDevice::Stop();

	DEBUG_EXIT
}

void RDMNetDevice::Run(void) {
	LLRPDevice::Run();
}

void RDMNetDevice::Print(void) {
	char uuid_str[UUID_STRING_LENGTH + 1];
	uuid_str[UUID_STRING_LENGTH] = '\0';
	uuid_unparse(m_Cid, uuid_str);

	printf("RDMNet configuration\n");
	printf(" CID : %s\n", uuid_str);

	LLRPDevice::Print();
	RDMDeviceResponder::Print();
}

void RDMNetDevice::CopyUID(uint8_t *pUID) {
	assert(pUID != 0);
	memcpy(pUID, RDMDeviceResponder::GetUID(), RDM_UID_SIZE);
}

void RDMNetDevice::CopyCID(uint8_t *pCID) {
	assert(pCID != 0);
	memcpy(pCID, m_Cid, sizeof(m_Cid));
}

uint8_t* RDMNetDevice::LLRPHandleRdmCommand(const uint8_t *pRdmDataNoSC) {
	m_RDMHandler->HandleData(pRdmDataNoSC, (uint8_t*) m_pRdmCommand);

	return (uint8_t*) m_pRdmCommand;
}
