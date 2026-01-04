/**
 * @file llrpdevicedump.cpp
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstdio>

#include "llrp/llrpdevice.h"
#include "e133.h"
#include "rdm_e120.h"

void LLRPDevice::DumpCommon()
{
#ifndef NDEBUG
    auto* common = reinterpret_cast<struct TLLRPCommonPacket*>(llrp);

    printf("RootLayerPreAmble.pre_amble_size=0x%.04x\n", __builtin_bswap16(common->RootLayerPreAmble.pre_amble_size));
    printf("RootLayerPreAmble.post_amble_size=0x%.04x\n", __builtin_bswap16(common->RootLayerPreAmble.post_amble_size));
    printf("RootLayerPreAmble.acn_packet_identifier=[%s]\n", common->RootLayerPreAmble.acn_packet_identifier);

    auto* pdu = reinterpret_cast<uint8_t*>(common->RootLayerPDU.flags_length);
    auto length = (((pdu[0] & 0x0fu) << 16) | static_cast<uint32_t>((pdu[1] << 8)) | static_cast<uint32_t>(pdu[2]));

    printf("RootLayerPDU PDU length=%d, High 4 bits=0x%.1x\n", length, static_cast<int>(common->RootLayerPDU.flags_length[0]) >> 4);
    printf("RootLayerPDU.vector=0x%.8x\n", __builtin_bswap32(common->RootLayerPDU.vector));
    printf("RootLayerPDU.SenderCid=");

    for (uint32_t i = 0; i < sizeof(common->RootLayerPDU.SenderCid); i++)
    {
        printf("%.02X", common->RootLayerPDU.SenderCid[i]);
    }
    puts("");

    pdu = reinterpret_cast<uint8_t*>(common->LlrpPDU.flags_length);
    length = (((pdu[0] & 0x0fu) << 16) | static_cast<uint32_t>((pdu[1] << 8)) | static_cast<uint32_t>(pdu[2]));

    printf("LlrpPDU PDU length=%d, High 4 bits=0x%.1x\n", length, static_cast<int>(common->LlrpPDU.flags_length[0]) >> 4);
    printf("LlrpPDU.vector=0x%.8x\n", __builtin_bswap32(common->LlrpPDU.vector));
    printf("LlrpPDU.DestinationCid=");

    for (uint32_t i = 0; i < sizeof(common->LlrpPDU.DestinationCid); i++)
    {
        printf("%.02X", common->LlrpPDU.DestinationCid[i]);
    }
    puts("");

    printf("LlrpPDU.TransactionNumber=0x%.4x\n", __builtin_bswap32(common->LlrpPDU.TransactionNumber));

    switch (__builtin_bswap32(common->LlrpPDU.vector))
    {
        case VECTOR_LLRP_PROBE_REQUEST:
        {
            auto* request = reinterpret_cast<struct TProbeRequestPDUPacket*>(llrp);

            pdu = reinterpret_cast<uint8_t*>(request->ProbeRequestPDU.flags_length);
            length = (((pdu[0] & 0x0fu) << 16) | static_cast<uint32_t>((pdu[1] << 8)) | static_cast<uint32_t>(pdu[2]));

            printf("Probe Request PDU length=%d, High 4 bits=%.1x\n", length, static_cast<int>(request->ProbeRequestPDU.flags_length[0]) >> 4);
            printf("ProbeRequestPDU.vector=0x%.2x\n", static_cast<int>(request->ProbeRequestPDU.vector));
            printf("ProbeRequestPDU.Filter=0x%.4x\n", __builtin_bswap16(request->ProbeRequestPDU.Filter));
        }
        break;
        case VECTOR_LLRP_PROBE_REPLY:
        {
            auto* reply = reinterpret_cast<struct TTProbeReplyPDUPacket*>(llrp);

            pdu = reinterpret_cast<uint8_t*>(reply->ProbeReplyPDU.flags_length);
            length = (((pdu[0] & 0x0fu) << 16) | static_cast<uint32_t>((pdu[1] << 8)) | static_cast<uint32_t>(pdu[2]));

            printf("Probe Request PDU length=%d, High 4 bits=%.1x\n", length, static_cast<int>(reply->ProbeReplyPDU.flags_length[0]) >> 4);
            printf("ProbeRequestPDU.vector=0x%.2x\n", static_cast<int>(reply->ProbeReplyPDU.vector));
            printf("ProbeReplyPDU.UID=");

            for (uint32_t i = 0; i < sizeof(reply->ProbeReplyPDU.UID); i++)
            {
                printf("%.02X", reply->ProbeReplyPDU.UID[i]);
            }
            puts("");

            printf("ProbeReplyPDU.HardwareAddress=");

            for (uint32_t i = 0; i < sizeof(reply->ProbeReplyPDU.HardwareAddress); i++)
            {
                printf("%.02X", reply->ProbeReplyPDU.HardwareAddress[i]);
            }
            puts("");
        }
        break;
        case VECTOR_LLRP_RDM_CMD:
        {
            auto* rdm_command = reinterpret_cast<struct LTRDMCommandPDUPacket*>(llrp);

            pdu = reinterpret_cast<uint8_t*>(rdm_command->RDMCommandPDU.flags_length);
            length = (((pdu[0] & 0x0fu) << 16) | static_cast<uint32_t>((pdu[1] << 8)) | static_cast<uint32_t>(pdu[2]));

            printf("RDM Command PDU length=%d, High 4 bits=0x%.1x\n", length, rdm_command->RDMCommandPDU.flags_length[0] >> 4);
            printf("RDMCommandPDU.vector=0x%.2x\n", rdm_command->RDMCommandPDU.vector);
        }
        break;
        default:
            break;
    }
#endif
}

void LLRPDevice::DumpLLRP()
{
    const auto* common = reinterpret_cast<struct TLLRPCommonPacket*>(llrp);

    printf("SenderCID: ");

    for (uint32_t i = 0; i < sizeof(common->RootLayerPDU.SenderCid); i++)
    {
        printf("%.02X", common->RootLayerPDU.SenderCid[i]);
    }

    printf(" DestinationCID: ");

    for (uint32_t i = 0; i < sizeof(common->LlrpPDU.DestinationCid); i++)
    {
        printf("%.02X", common->LlrpPDU.DestinationCid[i]);
    }

    puts("");
}
