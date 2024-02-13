/**
 * @file rdmnetdevice.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "llrp/llrpdevice.h"

#include "rdmhandler.h"
#include "e131.h"

#include "hardware.h"

#include "debug.h"

#if defined (RDM_RESPONDER)
# error "Cannot be both RDMNet Device and RDM Responder"
#endif

class RDMNetDevice final: public RDMDeviceResponder, public LLRPDevice {
public:
	RDMNetDevice(RDMPersonality **pRDMPersonalities, uint32_t nPersonalityCount) : RDMDeviceResponder(pRDMPersonalities, nPersonalityCount) {
		DEBUG_ENTRY

		DEBUG_EXIT
	}

	~RDMNetDevice() {
		DEBUG_ENTRY

		DEBUG_EXIT
	};

	void Run() {
		LLRPDevice::Run();
	}

	void Print() {
		static constexpr auto UUID_STRING_LENGTH = 36;
		char uuid_str[UUID_STRING_LENGTH + 1];
		uuid_str[UUID_STRING_LENGTH] = '\0';

		uint8_t s_Cid[e131::CID_LENGTH];
		Hardware::Get()->GetUuid(s_Cid);
		uuid_unparse(s_Cid, uuid_str);

		printf("RDMNet\n");
		printf(" CID : %s\n", uuid_str);

		LLRPDevice::Print();
		RDMDeviceResponder::Print();
	}
};

#endif /* RDMNETDEVICE_H_ */
