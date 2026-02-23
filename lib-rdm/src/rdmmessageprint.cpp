/**
 * @file rdmmessageprint.cpp
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

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "e120.h"
#include "rdm_e120.h"

 #include "firmware/debug/debug_debug.h"

namespace rdm
{
void MessagePrint(const uint8_t* rdm_data)
{
    if (rdm_data == nullptr)
    {
        DEBUG_PUTS("No RDM data");
        return;
    }

    const auto* rdm_message = reinterpret_cast<const struct TRdmMessage*>(rdm_data);

    if (rdm_data[0] == E120_SC_RDM)
    {
        const uint8_t* src_uid = rdm_message->source_uid;
        printf("%.2x%.2x:%.2x%.2x%.2x%.2x -> ", src_uid[0], src_uid[1], src_uid[2], src_uid[3], src_uid[4], src_uid[5]);

        const uint8_t* dst_uid = rdm_message->destination_uid;
        printf("%.2x%.2x:%.2x%.2x%.2x%.2x ", dst_uid[0], dst_uid[1], dst_uid[2], dst_uid[3], dst_uid[4], dst_uid[5]);

        switch (rdm_message->command_class)
        {
            case E120_DISCOVERY_COMMAND:
                printf("DISCOVERY_COMMAND");
                break;
            case E120_DISCOVERY_COMMAND_RESPONSE:
                printf("DISCOVERY_COMMAND_RESPONSE");
                break;
            case E120_GET_COMMAND:
                printf("GET_COMMAND");
                break;
            case E120_GET_COMMAND_RESPONSE:
                printf("GET_COMMAND_RESPONSE %u", rdm_message->slot16.response_type);
                break;
            case E120_SET_COMMAND:
                printf("SET_COMMAND");
                break;
            case E120_SET_COMMAND_RESPONSE:
                printf("SET_COMMAND_RESPONSE %u", rdm_message->slot16.response_type);
                break;
            default:
                printf("CC {%.2x}", rdm_message->command_class);
                break;
        }

        const auto kSubDevice = static_cast<uint16_t>((rdm_message->sub_device[0] << 8) + rdm_message->sub_device[1]);
        printf(", sub-dev: %d, tn: %d, PID 0x%.2x%.2x, pdl: %d", kSubDevice, rdm_message->transaction_number, rdm_message->param_id[0],
               rdm_message->param_id[1], rdm_message->param_data_length);

        if (rdm_message->param_data_length != 0)
        {
            printf(" -> ");
            for (uint32_t i = 0; (i < 12) && (i < rdm_message->param_data_length); i++)
            {
                printf("%.2x ", rdm_message->param_data[i]);
            }
        }

        puts("");
    }
    else if (rdm_data[0] == 0xFE)
    {
        for (uint32_t i = 0; i < 24; i++)
        {
            printf("%.2x ", rdm_data[i]);
        }
        puts("");
    }
    else
    {
        printf("Corrupted? RDM data [0-3]: %.2x:%.2x:%.2x:%.2x\n", rdm_data[0], rdm_data[1], rdm_data[2], rdm_data[3]);
    }
}

void MessagePrintNoStartcode(const uint8_t* rdm_data_no_sc)
{
    assert(rdm_data_no_sc != nullptr);

    const auto* data = reinterpret_cast<const struct TRdmMessageNoSc*>(rdm_data_no_sc);

    uint8_t message[sizeof(struct TRdmMessage)];
    message[0] = E120_SC_RDM;

    memcpy(&message[1], data, data->message_length - 1U);

    MessagePrint(message);
}
} // namespace rdm
