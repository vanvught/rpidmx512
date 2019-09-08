/**
 * @file llrpdevice.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>

#include "llrpdevice.h"

#include "llrp.h"
#include "llrppacket.h"

#include "rdmmessage.h"

#include "e133.h"
#include "rdm_e120.h"

#include "network.h"

#include "debug.h"

#define SHOW_LLRP_MESSAGE
#define SHOW_RDM_MESSAGE

LLRPDevice::LLRPDevice(void):
	m_nHandleLLRP(-1),
	m_nIpAddresLLRPRequest(0),
	m_nIpAddressLLRPResponse(0)
{
	DEBUG_ENTRY

	struct in_addr addr;

	(void) inet_aton(LLRP_MULTICAST_IPV4_ADDRESS_REQUEST, &addr);
	m_nIpAddresLLRPRequest = addr.s_addr;

	(void) inet_aton(LLRP_MULTICAST_IPV4_ADDRESS_RESPONSE, &addr);
	m_nIpAddressLLRPResponse = addr.s_addr;

	DEBUG_PRINTF("sizeof(m_tLLRP.LLRPPacket)=%d", (int) sizeof(m_tLLRP.LLRPPacket));
	DEBUG_EXIT
}

LLRPDevice::~LLRPDevice(void) {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void LLRPDevice::Start(void) {
	DEBUG_ENTRY

	m_nHandleLLRP = Network::Get()->Begin(LLRP_PORT);
	assert(m_nHandleLLRP != -1);

	Network::Get()->JoinGroup(m_nHandleLLRP, m_nIpAddresLLRPRequest);

	DEBUG_EXIT
}

void LLRPDevice::Stop(void) {
	DEBUG_ENTRY

	Network::Get()->LeaveGroup(m_nHandleLLRP, m_nIpAddresLLRPRequest);
	Network::Get()->End(LLRP_PORT);

	DEBUG_EXIT
}

void LLRPDevice::HandleRequestMessage(void) {
	struct TTProbeReplyPDUPacket *pReply = (struct TTProbeReplyPDUPacket *) &(m_tLLRP.LLRPPacket.Reply);

	// Root Layer PDU
	CopyCID(pReply->Common.RootLayerPDU.SenderCid);
	// LLRP PDU
	pReply->Common.LlrpPDU.Vector = __builtin_bswap32(VECTOR_LLRP_PROBE_REPLY);
	memcpy(pReply->Common.LlrpPDU.DestinationCid, (const void *)pReply->Common.RootLayerPDU.SenderCid, 16);
	// Probe Reply PDU
	pReply->ProbeReplyPDU.Vector = VECTOR_PROBE_REPLY_DATA;
	CopyUID(pReply->ProbeReplyPDU.UID);
	Network::Get()->MacAddressCopyTo(pReply->ProbeReplyPDU.HardwareAddress);
	pReply->ProbeReplyPDU.ComponentType = LLRP_COMPONENT_TYPE_RPT_DEVICE;

	Network::Get()->SendTo(m_nHandleLLRP, (const uint8_t *)pReply, sizeof(struct TTProbeReplyPDUPacket), m_nIpAddressLLRPResponse, LLRP_PORT);
}

void LLRPDevice::HandleRdmCommand(void) {
	DEBUG_ENTRY

	struct LTRDMCommandPDUPacket *pRDMCommand = (struct LTRDMCommandPDUPacket *) &(m_tLLRP.LLRPPacket.Request);

#ifdef SHOW_RDM_MESSAGE
	DumpRdmMessageIn();
#endif

	const uint8_t *pReply = LLRPHandleRdmCommand(pRDMCommand->RDMCommandPDU.RDMData);

	if (*pReply != E120_SC_RDM) {
		DEBUG_EXIT
		return;
	}

#ifdef SHOW_RDM_MESSAGE
	RDMMessage::Print((uint8_t *)pReply);
#endif

	// Root Layer PDU
	CopyCID(pRDMCommand->Common.RootLayerPDU.SenderCid);
	// LLRP PDU
	memcpy(pRDMCommand->Common.LlrpPDU.DestinationCid, (const void *)pRDMCommand->Common.RootLayerPDU.SenderCid, 16);

	const uint8_t nMessageLength = pReply[2] + 1;
	memcpy((uint8_t *) pRDMCommand->RDMCommandPDU.RDMData, &pReply[1], nMessageLength);

	const uint16_t nLength = (uint16_t) sizeof(struct LTRDMCommandPDUPacket) - (uint16_t) sizeof(pRDMCommand->RDMCommandPDU.RDMData) + nMessageLength;

	Network::Get()->SendTo(m_nHandleLLRP, (const uint8_t *)pRDMCommand, nLength	, m_nIpAddressLLRPResponse, LLRP_PORT);

	debug_dump((void *)pRDMCommand, nLength);

	DEBUG_EXIT
}

void LLRPDevice::Run(void) {
	uint8_t *packet = (uint8_t *) &(m_tLLRP.LLRPPacket);
	uint16_t nForeignPort;

	const int nBytesReceived = Network::Get()->RecvFrom(m_nHandleLLRP, packet, (uint16_t)sizeof(m_tLLRP.LLRPPacket), &m_tLLRP.nIPAddressFrom, &nForeignPort) ;

	if (__builtin_expect((nBytesReceived < (int) sizeof(struct TLLRPCommonPacket)), 1)) {
		return;
	}

#ifndef NDEBUG
	debug_dump(packet, nBytesReceived);
	DumpCommon();
#endif

	const struct TLLRPCommonPacket *pCommon = (struct TLLRPCommonPacket *) &(m_tLLRP.LLRPPacket.Common);

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

void LLRPDevice::Print(void) {

}

void LLRPDevice::CopyUID(uint8_t *pUID) {
	// Override
}

void LLRPDevice::CopyCID(uint8_t *pCID) {
	// Override
}

uint8_t *LLRPDevice::LLRPHandleRdmCommand(const uint8_t *pRDMCommand) {
	// Override
	return 0;
}
