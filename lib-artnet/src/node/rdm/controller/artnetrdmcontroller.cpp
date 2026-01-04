/**
 * @file artnetrdmcontroller.cpp
 *
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#pragma GCC optimize("-funroll-loops")
#pragma GCC optimize("-fprefetch-loop-arrays")
#endif

#include <cstdint>
#include <cstring>
#include <cassert>

#include "artnetrdmcontroller.h"
#include "rdm.h"
#include "rdmconst.h"
#include "e120.h"
#include "rdm_e120.h"

static void RespondMessageAck(uint32_t port_index, struct TRdmMessage* message)
{
    assert(message->start_code == E120_SC_RDM);

    message->message_count = 0;
    message->command_class = static_cast<uint8_t>(message->command_class + 1U);
    message->message_length = static_cast<uint8_t>(RDM_MESSAGE_MINIMUM_SIZE + message->param_data_length);
    message->slot16.response_type = E120_RESPONSE_TYPE_ACK;

    for (uint32_t i = 0; i < RDM_UID_SIZE; i++)
    {
        auto uid = message->destination_uid[i];
        message->destination_uid[i] = message->source_uid[i];
        message->source_uid[i] = uid;
    }

    uint16_t checksum = 0;
    uint32_t i;

    auto* data = reinterpret_cast<uint8_t*>(message);

    for (i = 0; i < message->message_length; i++)
    {
        checksum = static_cast<uint16_t>(checksum + data[i]);
    }

    data[i++] = static_cast<uint8_t>(checksum >> 8);
    data[i++] = static_cast<uint8_t>(checksum & 0XFF);

    Rdm::SendRawRespondMessage(port_index, reinterpret_cast<uint8_t*>(message), i);
}

bool ArtNetRdmController::RdmReceive(uint32_t port_index, const uint8_t* data)
{
    assert(port_index < dmxnode::kMaxPorts);
    assert(data != nullptr);

    auto* rdm_message = reinterpret_cast<const struct TRdmMessage*>(data);
    auto* uid = rdm_message->destination_uid;

    auto is_rdm_packet_for_me = false;
    auto is_rdm_packet_broadcast = (memcmp(uid, UID_ALL, RDM_UID_SIZE) == 0);

    if (!is_rdm_packet_broadcast)
    {
        is_rdm_packet_broadcast = (memcmp(&rdm_message->destination_uid[2], UID_ALL, 4) == 0);

        if (!is_rdm_packet_broadcast)
        {
            is_rdm_packet_for_me = s_tod[port_index].Exist(uid);
        }
    }

    if ((!is_rdm_packet_for_me) && (!is_rdm_packet_broadcast))
    {
        return false;
    }

    if ((rdm_message->command_class == E120_GET_COMMAND) || (rdm_message->command_class == E120_SET_COMMAND))
    {
        return true;
    }

    if (is_rdm_packet_broadcast)
    {
        uid = s_tod[port_index].Next();
    }

    if (rdm_message->command_class == E120_DISCOVERY_COMMAND)
    {
        const auto kParamId = static_cast<uint16_t>((rdm_message->param_id[0] << 8) + rdm_message->param_id[1]);

        if (kParamId == E120_DISC_UNIQUE_BRANCH)
        {
            if (!s_tod[port_index].IsMuted())
            {
                if ((memcmp(rdm_message->param_data, uid, RDM_UID_SIZE) <= 0) && (memcmp(uid, rdm_message->param_data + 6, RDM_UID_SIZE) <= 0))
                {
                    auto* response = reinterpret_cast<struct TRdmDiscoveryMsg*>(const_cast<uint8_t*>(data));

                    uint16_t checksum = 6 * 0xFF;

                    for (uint32_t i = 0; i < 7; i++)
                    {
                        response->header_FE[i] = 0xFE;
                    }

                    response->header_AA = 0xAA;

                    for (uint32_t i = 0; i < 6; i++)
                    {
                        response->masked_device_id[i + i] = uid[i] | 0xAA;
                        response->masked_device_id[i + i + 1] = uid[i] | 0x55;
                        checksum = static_cast<uint16_t>(checksum + uid[i]);
                    }

                    response->checksum[0] = static_cast<uint8_t>((checksum >> 8) | 0xAA);
                    response->checksum[1] = static_cast<uint8_t>((checksum >> 8) | 0x55);
                    response->checksum[2] = static_cast<uint8_t>((checksum & 0xFF) | 0xAA);
                    response->checksum[3] = static_cast<uint8_t>((checksum & 0xFF) | 0x55);

                    Rdm::SendDiscoveryRespondMessage(port_index, reinterpret_cast<uint8_t*>(response), sizeof(struct TRdmDiscoveryMsg));
                    return false;
                }
            }
        }
        else if (kParamId == E120_DISC_UN_MUTE)
        {
            if (rdm_message->param_data_length != 0)
            {
                /* The response RESPONSE_TYPE_NACK_REASON shall only be used in conjunction
                 * with the Command Classes GET_COMMAND_RESPONSE & SET_COMMAND_RESPONSE.
                 */
                return false;
            }

            if (!is_rdm_packet_broadcast && is_rdm_packet_for_me)
            {
                s_tod[port_index].UnMute();

                auto* message_ack = reinterpret_cast<struct TRdmMessage*>(const_cast<uint8_t*>(data));

                message_ack->param_data_length = 2;
                message_ack->param_data[0] = 0x00; // Control Field
                message_ack->param_data[1] = 0x00; // Control Field

                RespondMessageAck(port_index, message_ack);
            }
            else
            {
                s_tod[port_index].UnMuteAll();
            }

            return false;
        }
        else if (kParamId == E120_DISC_MUTE)
        {
            if (rdm_message->param_data_length != 0)
            {
                /* The response RESPONSE_TYPE_NACK_REASON shall only be used in conjunction
                 * with the Command Classes GET_COMMAND_RESPONSE & SET_COMMAND_RESPONSE.
                 */
                return false;
            }

            if (is_rdm_packet_for_me)
            {
                s_tod[port_index].Mute();

                auto* message_ack = reinterpret_cast<struct TRdmMessage*>(const_cast<uint8_t*>(data));

                message_ack->param_data_length = 2;
                message_ack->param_data[0] = 0x00; // Control Field
                message_ack->param_data[1] = 0x00; // Control Field

                RespondMessageAck(port_index, message_ack);
                ;
                return false;
            }
        }
    }

    return false;
}
