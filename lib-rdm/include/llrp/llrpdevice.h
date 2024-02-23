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
#include <cstdio>
#include <cassert>

#include "llrp/llrppacket.h"
#include "e133.h"
#include "rdmconst.h"
#include "rdmhandler.h"

#include "network.h"

#include "debug.h"

namespace llrp {
namespace device {
static constexpr auto IPV4_LLRP_REQUEST = network::convert_to_uint(239, 255, 250, 133);
static constexpr auto IPV4_LLRP_RESPONSE = network::convert_to_uint(239, 255, 250, 134);
static constexpr uint16_t LLRP_PORT = 5569;
}  // namespace device
}  // namespace llrp

class LLRPDevice {
public:
	LLRPDevice() {
		DEBUG_ENTRY

		s_nHandleLLRP = Network::Get()->Begin(llrp::device::LLRP_PORT);
		assert(s_nHandleLLRP != -1);
		Network::Get()->JoinGroup(s_nHandleLLRP, llrp::device::IPV4_LLRP_REQUEST);

		DEBUG_EXIT
	}

	~LLRPDevice() {
		DEBUG_ENTRY

		Network::Get()->LeaveGroup(s_nHandleLLRP, llrp::device::IPV4_LLRP_REQUEST);
		Network::Get()->End(llrp::device::LLRP_PORT);

		DEBUG_EXIT
	}

	void Run() {
		uint16_t nForeignPort;

		const auto nBytesReceived = Network::Get()->RecvFrom(s_nHandleLLRP, const_cast<const void **>(reinterpret_cast<void **>(&s_pLLRP)), &s_nIpAddressFrom, &nForeignPort) ;

		if (__builtin_expect((nBytesReceived < sizeof(struct TLLRPCommonPacket)), 1)) {
			return;
		}

#ifndef NDEBUG
		DumpCommon();
#endif

		const auto *pCommon = reinterpret_cast<struct TLLRPCommonPacket *>(s_pLLRP);

		switch (__builtin_bswap32(pCommon->LlrpPDU.Vector)) {
		case VECTOR_LLRP_PROBE_REQUEST:
#ifdef SHOW_LLRP_MESSAGE
			printf("> VECTOR_LLRP_PROBE_REQUEST\n");
			DumpLLRP();
#endif
			HandleRequestMessage();
			break;
		case VECTOR_LLRP_PROBE_REPLY:
			// Nothing to do here
			DEBUG_PUTS("VECTOR_LLRP_PROBE_REPLY");
			break;
		case VECTOR_LLRP_RDM_CMD:
#ifdef SHOW_LLRP_MESSAGE
			printf("> VECTOR_LLRP_RDM_CMD\n");
			DumpLLRP();
#endif
			HandleRdmCommand();
			break;
		default:
			break;
		}
	}

	void Print() {
		printf("LLRP Device\n");
		printf(" Port UDP           : %d\n", llrp::device::LLRP_PORT);
		printf(" Join Request       : " IPSTR "\n", IP2STR(llrp::device::IPV4_LLRP_REQUEST));
		printf(" Multicast Response : " IPSTR "\n", IP2STR(llrp::device::IPV4_LLRP_RESPONSE));
	}

private:
	uint8_t *LLRPHandleRdmCommand(const uint8_t *pRdmDataNoSC) {
		m_RDMHandler.HandleData(pRdmDataNoSC, reinterpret_cast<uint8_t*>(&s_RdmCommand));
		return reinterpret_cast<uint8_t*>(&s_RdmCommand);
	}

	void HandleRequestMessage();
	void HandleRdmCommand();
	// DEBUG subject for deletions
	void DumpCommon();
	void DumpLLRP();
	void DumpRdmMessageInNoSc();

private:
	RDMHandler m_RDMHandler { false };

	static int32_t s_nHandleLLRP;
	static uint32_t s_nIpAddressFrom;
	static uint8_t *s_pLLRP;
	static TRdmMessage s_RdmCommand;
};

#endif /* LLRPDEVICE_H_ */
