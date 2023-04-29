/**
 * @file llrpdevice.cpp
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

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "llrpdevice.h"

#include "llrp.h"
#include "llrppacket.h"

#include "rdmmessage.h"

#include "e133.h"
#include "rdm_e120.h"

#include "debug.h"

#if !defined(BARE_METAL)
# define SHOW_LLRP_MESSAGE
# define SHOW_RDM_MESSAGE
#endif

int32_t LLRPDevice::s_nHandleLLRP { -1 };
uint32_t LLRPDevice::s_nIpAddressFrom;
uint8_t *LLRPDevice::s_pLLRP;

void LLRPDevice::HandleRequestMessage() {
	auto *pReply = reinterpret_cast<struct TTProbeReplyPDUPacket *>(s_pLLRP);

	uint8_t DestinationCid[16];
	memcpy(DestinationCid, pReply->Common.RootLayerPDU.SenderCid, 16); // TODO Optimize / cleanup

	// Root Layer PDU
	pReply->Common.RootLayerPDU.FlagsLength[2] = 67;
	CopyCID(pReply->Common.RootLayerPDU.SenderCid);
	// LLRP PDU
	pReply->Common.LlrpPDU.FlagsLength[2] = 44;
	pReply->Common.LlrpPDU.Vector = __builtin_bswap32(VECTOR_LLRP_PROBE_REPLY);
	memcpy(pReply->Common.LlrpPDU.DestinationCid, DestinationCid, 16);
	// Probe Reply PDU
	pReply->ProbeReplyPDU.FlagsLength[2] = 17;
	pReply->ProbeReplyPDU.Vector = VECTOR_PROBE_REPLY_DATA;
	CopyUID(pReply->ProbeReplyPDU.UID);
	Network::Get()->MacAddressCopyTo(pReply->ProbeReplyPDU.HardwareAddress);
#if defined (NODE_RDMNET_LLRP_ONLY)
	pReply->ProbeReplyPDU.ComponentType = LLRP_COMPONENT_TYPE_NON_RDMNET;
#else
	pReply->ProbeReplyPDU.ComponentType = LLRP_COMPONENT_TYPE_RPT_DEVICE;
#endif

	Network::Get()->SendTo(s_nHandleLLRP, pReply, sizeof(struct TTProbeReplyPDUPacket), llrp::device::IP_LLRP_RESPONSE, LLRP_PORT);

#ifndef NDEBUG
	debug_dump(pReply, sizeof(struct TTProbeReplyPDUPacket));
	DumpCommon();
#endif
}

void LLRPDevice::HandleRdmCommand() {
	DEBUG_ENTRY

	auto *pRDMCommand = reinterpret_cast<struct LTRDMCommandPDUPacket *>(s_pLLRP);

#ifdef SHOW_RDM_MESSAGE
	const uint8_t *pRdmDataInNoSc = const_cast<uint8_t *>(pRDMCommand->RDMCommandPDU.RDMData)	;
	RDMMessage::PrintNoSc(pRdmDataInNoSc);
#endif

	const uint8_t *pReply = LLRPHandleRdmCommand(pRDMCommand->RDMCommandPDU.RDMData);

	if ((pReply == nullptr) || (*pReply != E120_SC_RDM)) {
		DEBUG_EXIT
		return;
	}

	uint8_t DestinationCid[16];
	memcpy(DestinationCid, pRDMCommand->Common.RootLayerPDU.SenderCid, 16); // TODO Optimize / cleanup

	const auto nMessageLength = static_cast<uint8_t>(pReply[2] + 1);	// RDM Command length without SC

	// Root Layer PDU
	pRDMCommand->Common.RootLayerPDU.FlagsLength[2] = RDM_ROOT_LAYER_LENGTH(nMessageLength);
	CopyCID(pRDMCommand->Common.RootLayerPDU.SenderCid);
	// LLRP PDU
	pRDMCommand->Common.LlrpPDU.FlagsLength[2] = RDM_LLRP_PDU_LENGHT(nMessageLength);
	memcpy(pRDMCommand->Common.LlrpPDU.DestinationCid, DestinationCid, 16);
	// RDM Command
	pRDMCommand->RDMCommandPDU.FlagsLength[2] = RDM_COMMAND_PDU_LENGTH(nMessageLength);
	assert(E120_SC_RDM == VECTOR_RDM_CMD_RDM_DATA);
	memcpy(pRDMCommand->RDMCommandPDU.RDMData, &pReply[1], nMessageLength);

	const auto nLength = sizeof(struct TRootLayerPreAmble) + RDM_ROOT_LAYER_LENGTH(nMessageLength);

	Network::Get()->SendTo(s_nHandleLLRP, pRDMCommand, static_cast<uint16_t>(nLength) , llrp::device::IP_LLRP_RESPONSE, LLRP_PORT);

#ifdef SHOW_RDM_MESSAGE
	RDMMessage::Print(pReply);
#endif

#ifndef NDEBUG
	debug_dump(pRDMCommand, static_cast<uint16_t>(nLength));
	DumpCommon();
#endif

	DEBUG_EXIT
}

void LLRPDevice::Run() {
	uint16_t nForeignPort;

	const auto nBytesReceived = Network::Get()->RecvFrom(s_nHandleLLRP, const_cast<const void **>(reinterpret_cast<void **>(&s_pLLRP)), &s_nIpAddressFrom, &nForeignPort) ;

	if (__builtin_expect((nBytesReceived < sizeof(struct TLLRPCommonPacket)), 1)) {
		return;
	}

#ifndef NDEBUG
	debug_dump(s_pLLRP, nBytesReceived);
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

void LLRPDevice::Print() {
	printf("LLRP Device configuration\n");
	printf(" Port UDP               : %d\n", LLRP_PORT);
	printf(" Multicast join Request : " IPSTR "\n", IP2STR(llrp::device::IP_LLRP_REQUEST));
	printf(" Multicast Response     : " IPSTR "\n", IP2STR(llrp::device::IP_LLRP_RESPONSE));
}
