/**
 * @file rdm.h
 *
 */
/* Copyright (C) 2015-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDM_H_
#define RDM_H_

#include <cstdint>
#include <cassert>

#include "hal_udelay.h" 
#include "dmx.h" // IWYU pragma: keep
#include "e120.h"
#include "rdmconst.h"

class Rdm
{
   public:
    static void SendRaw(uint32_t port_index, const uint8_t* rdm_data, uint32_t length)
    {
        assert(rdm_data != nullptr);
        assert(length != 0);

        Dmx::Get()->SetPortDirection(port_index, dmx::PortDirection::kOutput, false);

        Dmx::Get()->RdmSendRaw(port_index, rdm_data, length);

        udelay(RDM_RESPONDER_DATA_DIRECTION_DELAY);

        Dmx::Get()->SetPortDirection(port_index, dmx::PortDirection::kInput, true);
    }

    static void Send(uint32_t port_index, struct TRdmMessage* rdm_command)
    {
        assert(port_index < dmx::config::max::PORTS);
        assert(rdm_command != nullptr);

        auto* data = reinterpret_cast<uint8_t*>(rdm_command);
        uint32_t i;
        uint16_t checksum = 0;

        rdm_command->transaction_number = s_transaction_number[port_index];

        for (i = 0; i < rdm_command->message_length; i++)
        {
            checksum = static_cast<uint16_t>(checksum + data[i]);
        }

        data[i++] = static_cast<uint8_t>(checksum >> 8);
        data[i] = static_cast<uint8_t>(checksum & 0XFF);

        SendRaw(port_index, reinterpret_cast<const uint8_t*>(rdm_command), rdm_command->message_length + RDM_MESSAGE_CHECKSUM_SIZE);

        s_transaction_number[port_index]++;
    }

    static void SendRawRespondMessage(uint32_t port_index, const uint8_t* rdm_data, uint32_t length)
    {
        assert(port_index < dmx::config::max::PORTS);
        assert(rdm_data != nullptr);
        assert(length != 0);

        extern volatile uint32_t gsv_RdmDataReceiveEnd;
        // 3.2.2 Responder Packet spacing
        udelay(RDM_RESPONDER_PACKET_SPACING, gsv_RdmDataReceiveEnd);

        SendRaw(port_index, rdm_data, length);
    }

    static void SendDiscoveryRespondMessage(uint32_t port_index, const uint8_t* rdm_data, uint32_t length)
    {
        Dmx::Get()->RdmSendDiscoveryRespondMessage(port_index, rdm_data, length);
    }

    static const uint8_t* Receive(uint32_t port_index) { return Dmx::Get()->RdmReceive(port_index); }

    static const uint8_t* ReceiveTimeOut(uint32_t port_index, uint16_t time_out) { return Dmx::Get()->RdmReceiveTimeOut(port_index, time_out); }

   private:
    static uint8_t s_transaction_number[dmx::config::max::PORTS];
};

#endif  // RDM_H_
