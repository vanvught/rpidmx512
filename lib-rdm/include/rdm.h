/**
 * @file rdm.h
 *
 */
/* Copyright (C) 2015-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "dmx.h" // IWYU pragma: keep
#include "e120.h"
#include "rdmconst.h"
#include "timing.h"

class Rdm {
   public:
    static void TransmitRaw(uint32_t port_index, const uint8_t* rdm_data, uint32_t length) {
        assert(rdm_data != nullptr);
        assert(length != 0);

        Dmx::Get()->RdmTransmit(port_index, rdm_data, length);
    }

    static void Transmit(uint32_t port_index, struct TRdmMessage* rdm_command) {
        assert(port_index < dmx::config::max::kPorts);
        assert(rdm_command != nullptr);

        auto* data = reinterpret_cast<uint8_t*>(rdm_command);
        uint32_t i;
        uint16_t checksum = 0;

        rdm_command->transaction_number = s_transaction_number[port_index];

        for (i = 0; i < rdm_command->message_length; i++) {
            checksum = static_cast<uint16_t>(checksum + data[i]);
        }

        data[i++] = static_cast<uint8_t>(checksum >> 8);
        data[i] = static_cast<uint8_t>(checksum & 0XFF);

        TransmitRaw(port_index, reinterpret_cast<const uint8_t*>(rdm_command), rdm_command->message_length + rdm::kMessageChecksumSize);

        s_transaction_number[port_index]++;
    }

    static void TransmitRawRespondMessage(uint32_t port_index, const uint8_t* rdm_data, uint32_t length) {
        assert(port_index < dmx::config::max::kPorts);
        assert(rdm_data != nullptr);
        assert(length != 0);

        extern volatile uint32_t gsv_rdm_data_receive_end[dmx::config::max::kPorts];
        // 3.2.2 Responder Packet spacing
        timing::DelayUs(rdm::responder::kPacketSpacing, gsv_rdm_data_receive_end[port_index]);

        TransmitRaw(port_index, rdm_data, length);
    }

    static void TransmitDiscoveryRespondMessage(uint32_t port_index, const uint8_t* rdm_data, uint32_t length) { Dmx::Get()->RdmTransmitDiscoveryRespondMessage(port_index, rdm_data, length); }

    static const uint8_t* Receive(uint32_t port_index) { return Dmx::Get()->RdmReceive(port_index); }
    static const uint8_t* ReceiveTimeOut(uint32_t port_index, uint16_t time_out) { return Dmx::Get()->RdmReceiveTimeOut(port_index, time_out); }

   private:
    static uint8_t s_transaction_number[dmx::config::max::kPorts];
};

#endif // RDM_H_
