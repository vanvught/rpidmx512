/**
 * @file dump.cpp
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "llrp/llrpdevice.h"

#include "e133.h"

#include "rdm_e120.h"
#include "rdm_message_print.h"

#include "debug.h"

void LLRPDevice::DumpCommon() {
#ifndef NDEBUG
	auto *pCommon = reinterpret_cast<struct TLLRPCommonPacket *>(s_pLLRP);

	printf("RootLayerPreAmble.PreAmbleSize=0x%.04x\n", __builtin_bswap16(pCommon->RootLayerPreAmble.PreAmbleSize));
	printf("RootLayerPreAmble.PostAmbleSize=0x%.04x\n",  __builtin_bswap16(pCommon->RootLayerPreAmble.PostAmbleSize));
	printf("RootLayerPreAmble.ACNPacketIdentifier=[%s]\n", pCommon->RootLayerPreAmble.ACNPacketIdentifier);

	auto *pPdu = reinterpret_cast<uint8_t*>(pCommon->RootLayerPDU.FlagsLength);
	auto nLength = (((pPdu[0] & 0x0fu) << 16) | static_cast<uint32_t>((pPdu[1] << 8)) | static_cast<uint32_t>(pPdu[2]));

	printf("RootLayerPDU PDU length=%d, High 4 bits=0x%.1x\n", nLength, static_cast<int>(pCommon->RootLayerPDU.FlagsLength[0]) >> 4);
	printf("RootLayerPDU.Vector=0x%.8x\n", __builtin_bswap32(pCommon->RootLayerPDU.Vector));
	printf("RootLayerPDU.SenderCid=");

	for (uint32_t i = 0; i < sizeof(pCommon->RootLayerPDU.SenderCid); i++) {
		printf("%.02X", pCommon->RootLayerPDU.SenderCid[i]);
	}
	puts("");

	pPdu = reinterpret_cast<uint8_t *>(pCommon->LlrpPDU.FlagsLength);
	nLength = (((pPdu[0] & 0x0fu) << 16) | static_cast<uint32_t>((pPdu[1] << 8)) | static_cast<uint32_t>(pPdu[2]));

	printf("LlrpPDU PDU length=%d, High 4 bits=0x%.1x\n", nLength, static_cast<int>(pCommon->LlrpPDU.FlagsLength[0]) >> 4);
	printf("LlrpPDU.Vector=0x%.8x\n", __builtin_bswap32(pCommon->LlrpPDU.Vector));
	printf("LlrpPDU.DestinationCid=");

	for (uint32_t i = 0; i < sizeof(pCommon->LlrpPDU.DestinationCid); i++) {
		printf("%.02X", pCommon->LlrpPDU.DestinationCid[i]);
	}
	puts("");

	printf("LlrpPDU.TransactionNumber=0x%.4x\n", __builtin_bswap32(pCommon->LlrpPDU.TransactionNumber));

	switch (__builtin_bswap32(pCommon->LlrpPDU.Vector)) {
	case VECTOR_LLRP_PROBE_REQUEST: {
		auto *pRequest = reinterpret_cast<struct TProbeRequestPDUPacket *>(s_pLLRP);

		pPdu = reinterpret_cast<uint8_t*>(pRequest->ProbeRequestPDU.FlagsLength);
		nLength = (((pPdu[0] & 0x0fu) << 16) | static_cast<uint32_t>((pPdu[1] << 8)) | static_cast<uint32_t>(pPdu[2]));

		printf("Probe Request PDU length=%d, High 4 bits=%.1x\n", nLength, static_cast<int>(pRequest->ProbeRequestPDU.FlagsLength[0]) >> 4);
		printf("ProbeRequestPDU.Vector=0x%.2x\n", static_cast<int>(pRequest->ProbeRequestPDU.Vector));
		printf("ProbeRequestPDU.Filter=0x%.4x\n", __builtin_bswap16(pRequest->ProbeRequestPDU.Filter));
	}
	break;
	case VECTOR_LLRP_PROBE_REPLY: {
		auto *pReply = reinterpret_cast<struct TTProbeReplyPDUPacket *>(s_pLLRP);

		pPdu = reinterpret_cast<uint8_t*>(pReply->ProbeReplyPDU.FlagsLength);
		nLength = (((pPdu[0] & 0x0fu) << 16) | static_cast<uint32_t>((pPdu[1] << 8)) | static_cast<uint32_t>(pPdu[2]));

		printf("Probe Request PDU length=%d, High 4 bits=%.1x\n", nLength, static_cast<int>(pReply->ProbeReplyPDU.FlagsLength[0]) >> 4);
		printf("ProbeRequestPDU.Vector=0x%.2x\n", static_cast<int>(pReply->ProbeReplyPDU.Vector));
		printf("ProbeReplyPDU.UID=");

		for (uint32_t i = 0; i < sizeof(pReply->ProbeReplyPDU.UID); i++) {
			printf("%.02X", pReply->ProbeReplyPDU.UID[i]);
		}
		puts("");

		printf("ProbeReplyPDU.HardwareAddress=");

		for (uint32_t i = 0; i < sizeof(pReply->ProbeReplyPDU.HardwareAddress); i++) {
			printf("%.02X", pReply->ProbeReplyPDU.HardwareAddress[i]);
		}
		puts("");
	}
	break;
	case VECTOR_LLRP_RDM_CMD: {
		auto *pRDMCommand = reinterpret_cast<struct LTRDMCommandPDUPacket *>(s_pLLRP);

		pPdu = reinterpret_cast<uint8_t*>(pRDMCommand->RDMCommandPDU.FlagsLength);
		nLength = (((pPdu[0] & 0x0fu) << 16) | static_cast<uint32_t>((pPdu[1] << 8)) | static_cast<uint32_t>(pPdu[2]));

		printf("RDM Command PDU length=%d, High 4 bits=0x%.1x\n", nLength, pRDMCommand->RDMCommandPDU.FlagsLength[0] >> 4);
		printf("RDMCommandPDU.Vector=0x%.2x\n", pRDMCommand->RDMCommandPDU.Vector);
	}
	break;
	default:
		break;
	}
#endif
}

void LLRPDevice::DumpLLRP() {
	const auto *pCommon = reinterpret_cast<struct TLLRPCommonPacket *>(s_pLLRP);

	printf("SenderCID: ");

	for (uint32_t i = 0; i < sizeof(pCommon->RootLayerPDU.SenderCid); i++) {
		printf("%.02X", pCommon->RootLayerPDU.SenderCid[i]);
	}

	printf(" DestinationCID: ");

	for (uint32_t i = 0; i < sizeof(pCommon->LlrpPDU.DestinationCid); i++) {
		printf("%.02X", pCommon->LlrpPDU.DestinationCid[i]);
	}

	puts("");
}
