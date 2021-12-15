/**
 * @file rdmnetdevice.h
 *
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef RDMNETDEVICE_H_
#define RDMNETDEVICE_H_

#include <cstdint>
#include <cstring>

#include "rdmdeviceresponder.h"
#include "llrpdevice.h"

#include "rdmhandler.h"

#include "e131.h"

#include "hardware.h"

class RDMNetDevice: public RDMDeviceResponder, public LLRPDevice {
public:
	RDMNetDevice(RDMPersonality **pRDMPersonalities, uint32_t nPersonalityCount) : RDMDeviceResponder(pRDMPersonalities, nPersonalityCount) {
		Hardware::Get()->GetUuid(s_Cid);
	}
	~RDMNetDevice() override {};

	void CopyUID(uint8_t *pUID) override {
		memcpy(pUID, RDMDeviceResponder::GetUID(), RDM_UID_SIZE);
	}
	void CopyCID(uint8_t *pCID) override {
		memcpy(pCID, s_Cid, sizeof(s_Cid));
	}

	uint8_t *LLRPHandleRdmCommand(const uint8_t *pRdmDataNoSC) override {
		m_RDMHandler.HandleData(pRdmDataNoSC, reinterpret_cast<uint8_t*>(&s_RdmCommand));
		return reinterpret_cast<uint8_t*>(&s_RdmCommand);
	}

	void Start() {
		LLRPDevice::Start();
	}
	void Stop() {
		LLRPDevice::Stop();
	}
	void Run() {
		LLRPDevice::Run();
	}

	void Print();

private:
	RDMHandler m_RDMHandler { false };

	static TRdmMessage s_RdmCommand;
	static uint8_t s_Cid[E131::CID_LENGTH];
};

#endif /* RDMNETDEVICE_H_ */
