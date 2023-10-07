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

#include "llrppacket.h"

#include "rdmmessage.h"

#include "e133.h"
#include "rdm_e120.h"

#include "debug.h"

#if !defined(BARE_METAL)
# define SHOW_LLRP_MESSAGE
# define SHOW_RDM_MESSAGE
#endif

int32_t LLRPDevice::s_nHandleLLRP ;
uint32_t LLRPDevice::s_nIpAddressFrom;
uint8_t *LLRPDevice::s_pLLRP;

void LLRPDevice::HandleRequestMessage() {
	DEBUG_ENTRY

	const auto *pRequest = reinterpret_cast<struct TProbeRequestPDUPacket *>(s_pLLRP);
	const auto *pPdu = pRequest->ProbeRequestPDU.FlagsLength;
	const auto nLength = (static_cast<uint32_t>((pPdu[0] & 0x0fu) << 16) | static_cast<uint32_t>(pPdu[1] << 8) | pPdu[2]);

	uint8_t uid[RDM_UID_SIZE];
	CopyUID(uid);

	if (nLength > 18) {
		const auto nKnownUIDs = (nLength - 18) / RDM_UID_SIZE;
		const auto *p = pRequest->ProbeRequestPDU.KnownUUIDs;

		for (uint32_t nIndex = 0; nIndex < nKnownUIDs; nIndex++) {
			if (memcmp(p, uid, RDM_UID_SIZE) == 0) {
				DEBUG_EXIT
				return;
			}
			p += RDM_UID_SIZE;
		}
	}

	debug_dump(pRequest->ProbeRequestPDU.LowerUUID, 2 * RDM_UID_SIZE);

	if (!((memcmp(pRequest->ProbeRequestPDU.LowerUUID, uid, RDM_UID_SIZE) <= 0) && (memcmp(uid, pRequest->ProbeRequestPDU.UpperUUID, RDM_UID_SIZE) <= 0))) {
		DEBUG_PUTS("Not for me");
		DEBUG_EXIT
		return;
	}

	auto *pReply = reinterpret_cast<struct TTProbeReplyPDUPacket *>(s_pLLRP);

	uint8_t DestinationCid[16];
	memcpy(DestinationCid, pReply->Common.RootLayerPDU.SenderCid, 16);

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

	Network::Get()->SendTo(s_nHandleLLRP, pReply, sizeof(struct TTProbeReplyPDUPacket), llrp::device::IPV4_LLRP_RESPONSE, llrp::device::LLRP_PORT);

#ifndef NDEBUG
	DumpCommon();
#endif
	DEBUG_EXIT
}

void LLRPDevice::HandleRdmCommand() {
	DEBUG_ENTRY

	auto *pRDMCommand = reinterpret_cast<struct LTRDMCommandPDUPacket *>(s_pLLRP);

#ifdef SHOW_RDM_MESSAGE
	const auto *pRdmDataInNoSc = const_cast<uint8_t *>(pRDMCommand->RDMCommandPDU.RDMData);
 	RDMMessage::PrintNoSc(pRdmDataInNoSc);
#endif

	const auto *pReply = LLRPHandleRdmCommand(pRDMCommand->RDMCommandPDU.RDMData);

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

	Network::Get()->SendTo(s_nHandleLLRP, pRDMCommand, static_cast<uint16_t>(nLength) , llrp::device::IPV4_LLRP_RESPONSE, llrp::device::LLRP_PORT);

#ifdef SHOW_RDM_MESSAGE
	RDMMessage::Print(pReply);
#endif

#ifndef NDEBUG
	DumpCommon();
#endif

	DEBUG_EXIT
}
