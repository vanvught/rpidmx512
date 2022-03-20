/**
 * @file llrpdevice.cpp
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

#ifndef LLRPDEVICE_H_
#define LLRPDEVICE_H_

#include <cstdint>
#include <cassert>

#include "llrppacket.h"
#include "network.h"

#include "debug.h"

class LLRPDevice {
public:
	LLRPDevice();
	virtual ~LLRPDevice() {}

	void Start() {
		DEBUG_ENTRY
		m_nHandleLLRP = Network::Get()->Begin(LLRP_PORT);
		assert(m_nHandleLLRP != -1);
		Network::Get()->JoinGroup(m_nHandleLLRP, m_nIpAddresLLRPRequest);
		DEBUG_EXIT
	}

	void Stop() {
		DEBUG_ENTRY
		Network::Get()->LeaveGroup(m_nHandleLLRP, m_nIpAddresLLRPRequest);
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
	static int32_t m_nHandleLLRP;
	static uint32_t m_nIpAddresLLRPRequest;
	static uint32_t m_nIpAddressLLRPResponse;
	static uint32_t s_nIpAddressFrom;
	static uint8_t *m_pLLRP;
};

#endif /* LLRPDEVICE_H_ */
