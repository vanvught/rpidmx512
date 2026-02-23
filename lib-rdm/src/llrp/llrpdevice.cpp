/**
 * @file llrpdevice.cpp
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

#if defined(DEBUG_RDM_LLRPDEVICE)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstring>
#include <cassert>

#include "hal.h"
#include "hal_uuid.h"
#include "llrp/llrpdevice.h"
#include "llrp/llrppacket.h"
#include "rdmdevice.h"
#include "e133.h"
#include "network.h"
#include "rdmconst.h"
#include "rdm_e120.h"
#include "rdm_message_print.h"
#include "firmware/debug/debug_dump.h"
#include "firmware/debug/debug_debug.h"

#if defined(__linux__) || defined(__APPLE__)
#define SHOW_LLRP_MESSAGE
#define DEBUG_RDM_SHOW_MESSAGE
#endif

void LLRPDevice::HandleRequestMessage()
{
    DEBUG_ENTRY();

    const auto* request = reinterpret_cast<struct TProbeRequestPDUPacket*>(llrp);
    const auto* pdu = request->ProbeRequestPDU.flags_length;
    const auto kLength = (static_cast<uint32_t>((pdu[0] & 0x0fu) << 16) | static_cast<uint32_t>(pdu[1] << 8) | pdu[2]);

    uint8_t uid[RDM_UID_SIZE];
    memcpy(uid, RdmDevice::Get().GetUID(), RDM_UID_SIZE);

    if (kLength > 18)
    {
        const auto kKnownUiDs = (kLength - 18) / RDM_UID_SIZE;
        const auto* p = request->ProbeRequestPDU.KnownUUIDs;

        for (uint32_t index = 0; index < kKnownUiDs; index++)
        {
            if (memcmp(p, uid, RDM_UID_SIZE) == 0)
            {
                DEBUG_EXIT();
                return;
            }
            p += RDM_UID_SIZE;
        }
    }

    debug::Dump(request->ProbeRequestPDU.LowerUUID, 2 * RDM_UID_SIZE);

    if (!((memcmp(request->ProbeRequestPDU.LowerUUID, uid, RDM_UID_SIZE) <= 0) && (memcmp(uid, request->ProbeRequestPDU.UpperUUID, RDM_UID_SIZE) <= 0)))
    {
        DEBUG_PUTS("Not for me");
        DEBUG_EXIT();
        return;
    }

    auto* reply = reinterpret_cast<struct TTProbeReplyPDUPacket*>(llrp);

    uint8_t destination_cid[16];
    memcpy(destination_cid, reply->Common.RootLayerPDU.SenderCid, 16);

    // Root Layer PDU
    reply->Common.RootLayerPDU.flags_length[2] = 67;
    hal::UuidCopy(reply->Common.RootLayerPDU.SenderCid);
    // LLRP PDU
    reply->Common.LlrpPDU.flags_length[2] = 44;
    reply->Common.LlrpPDU.vector = __builtin_bswap32(VECTOR_LLRP_PROBE_REPLY);
    memcpy(reply->Common.LlrpPDU.DestinationCid, destination_cid, 16);
    // Probe Reply PDU
    reply->ProbeReplyPDU.flags_length[2] = 17;
    reply->ProbeReplyPDU.vector = VECTOR_PROBE_REPLY_DATA;
    memcpy(reply->ProbeReplyPDU.UID, RdmDevice::Get().GetUID(), RDM_UID_SIZE);
    network::iface::CopyMacAddressTo(reply->ProbeReplyPDU.HardwareAddress);
#if defined(NODE_RDMNET_LLRP_ONLY)
    reply->ProbeReplyPDU.ComponentType = LLRP_COMPONENT_TYPE_NON_RDMNET;
#else
    reply->ProbeReplyPDU.ComponentType = LLRP_COMPONENT_TYPE_RPT_DEVICE;
#endif

    network::udp::Send(handle_llrp, reinterpret_cast<const uint8_t*>(reply), sizeof(struct TTProbeReplyPDUPacket), llrp::device::kIpV4LlrpResponse,
                   llrp::device::kLlrpPort);

#ifndef NDEBUG
    DumpCommon();
#endif
    DEBUG_EXIT();
}

void LLRPDevice::HandleRdmCommand()
{
    DEBUG_ENTRY();

    auto* pdu_packet = reinterpret_cast<struct LTRDMCommandPDUPacket*>(llrp);

#ifdef DEBUG_RDM_SHOW_MESSAGE
    const auto* rdm_data_in_no_sc = const_cast<uint8_t*>(pdu_packet->RDMCommandPDU.RDMData);
    rdm::MessagePrintNoStartcode(rdm_data_in_no_sc);
#endif

    const auto* reply = LLRPHandleRdmCommand(pdu_packet->RDMCommandPDU.RDMData);

    if ((reply == nullptr) || (*reply != E120_SC_RDM))
    {
        DEBUG_EXIT();
        return;
    }

    uint8_t destination_cid[16];
    memcpy(destination_cid, pdu_packet->Common.RootLayerPDU.SenderCid, 16);

    const auto kMessageLength = static_cast<uint8_t>(reply[2] + 1); // RDM Command length without SC

    // Root Layer PDU
    pdu_packet->Common.RootLayerPDU.flags_length[2] = RDM_ROOT_LAYER_LENGTH(kMessageLength);
    hal::UuidCopy(pdu_packet->Common.RootLayerPDU.SenderCid);
    // LLRP PDU
    pdu_packet->Common.LlrpPDU.flags_length[2] = RDM_LLRP_PDU_LENGHT(kMessageLength);
    memcpy(pdu_packet->Common.LlrpPDU.DestinationCid, destination_cid, 16);
    // RDM Command
    pdu_packet->RDMCommandPDU.flags_length[2] = RDM_COMMAND_PDU_LENGTH(kMessageLength);
    assert(E120_SC_RDM == VECTOR_RDM_CMD_RDM_DATA);
    memcpy(pdu_packet->RDMCommandPDU.RDMData, &reply[1], kMessageLength);

    const auto kLength = sizeof(struct TRootLayerPreAmble) + RDM_ROOT_LAYER_LENGTH(kMessageLength);

    network::udp::Send(handle_llrp, reinterpret_cast<const uint8_t*>(pdu_packet), kLength, llrp::device::kIpV4LlrpResponse, llrp::device::kLlrpPort);

#ifdef DEBUG_RDM_SHOW_MESSAGE
    rdm::MessagePrint(reply);
#endif

#ifndef NDEBUG
    DumpCommon();
#endif

    DEBUG_EXIT();
}
