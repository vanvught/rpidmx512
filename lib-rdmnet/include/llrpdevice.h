/**
 * @file llrpdevice.h
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef LLRPDEVICE_H_
#define LLRPDEVICE_H_

#include <cstdint>
#include <cassert>

#include "llrppacket.h"
#include "network.h"

#include "debug.h"

namespace llrp {
namespace device {
static constexpr auto IP_LLRP_REQUEST = network::convert_to_uint(239, 255, 250, 133);
static constexpr auto IP_LLRP_RESPONSE  = network::convert_to_uint(239, 255, 250, 134);
}  // namespace device
}  // namespace llrp

class LLRPDevice {
public:
	LLRPDevice() {
		DEBUG_ENTRY
		s_nHandleLLRP = Network::Get()->Begin(LLRP_PORT);
		assert(s_nHandleLLRP != -1);
		Network::Get()->JoinGroup(s_nHandleLLRP, llrp::device::IP_LLRP_REQUEST);
		DEBUG_EXIT
	}

	virtual ~LLRPDevice() {
		DEBUG_ENTRY
		Network::Get()->LeaveGroup(s_nHandleLLRP, llrp::device::IP_LLRP_REQUEST);
		Network::Get()->End(LLRP_PORT);
		DEBUG_EXIT
	}

	void Run();
	void Print();

protected:
	virtual void CopyUID(__attribute__((unused)) uint8_t *pUID) {
		// Override
	}

	virtual void CopyCID(__attribute__((unused)) uint8_t *pCID) {
		// Override
	}

	virtual uint8_t *LLRPHandleRdmCommand(__attribute__((unused)) const uint8_t *pRDMCommand) {
		// Override
		return nullptr;
	}

private:
	void HandleRequestMessage();
	void HandleRdmCommand();
	// DEBUG subject for deletions
	void DumpCommon();
	void DumpLLRP();
	void DumpRdmMessageInNoSc();

private:
	static int32_t s_nHandleLLRP;
	static uint32_t s_nIpAddressFrom;
	static uint8_t *s_pLLRP;
};

#endif /* LLRPDEVICE_H_ */
