/**
 * @file rdmmessage.h
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

#ifndef RDMMESSAGE_H_
#define RDMMESSAGE_H_

#include <cassert>
#include <cstring>

#include "rdm.h"
#include "e120.h"
#include "rdm_e120.h"
#ifndef NDEBUG
#include "rdm_message_print.h"
#endif

class RDMMessage final : public Rdm
{
   public:
    RDMMessage()
    {
        message_.start_code = E120_SC_RDM;
        message_.sub_start_code = E120_SC_SUB_MESSAGE;
        message_.message_length = RDM_MESSAGE_MINIMUM_SIZE;
        memcpy(message_.source_uid, UID_ALL, RDM_UID_SIZE);
        memcpy(message_.destination_uid, UID_ALL, RDM_UID_SIZE);
        message_.slot16.port_id = 1;
        message_.message_count = 0;
        message_.sub_device[0] = 0;
        message_.sub_device[1] = 0;
        message_.param_data_length = 0;
    }

    void SetPortID(uint8_t port_id)
    {
        assert(port_id > 0);
        message_.slot16.port_id = port_id;
    }

    void SetSrcUid(const uint8_t* src_uid) { memcpy(message_.source_uid, src_uid, RDM_UID_SIZE); }

    void SetDstUid(const uint8_t* dst_uid) { memcpy(message_.destination_uid, dst_uid, RDM_UID_SIZE); }

    void SetSubDevice(uint16_t sub_device)
    {
        message_.sub_device[0] = static_cast<uint8_t>(sub_device >> 8);
        message_.sub_device[1] = static_cast<uint8_t>(sub_device);
    }

    void SetCc(uint8_t cc) { message_.command_class = cc; }

    void SetPid(uint16_t pid)
    {
        message_.param_id[0] = static_cast<uint8_t>(pid >> 8);
        message_.param_id[1] = static_cast<uint8_t>(pid);
    }

    void SetPd(const uint8_t* param_data, uint8_t length)
    {
        message_.message_length = static_cast<uint8_t>(message_.message_length - message_.param_data_length);
        message_.param_data_length = length;
        if ((param_data != nullptr) && (length != 0))
        {
            memcpy(message_.param_data, param_data, length);
        }
        message_.message_length = static_cast<uint8_t>(message_.message_length + length);
    }

    void Send(uint32_t port_index)
    {
#ifndef NDEBUG
        rdm::MessagePrint(reinterpret_cast<const uint8_t*>(&message_));
#endif
        Rdm::Send(port_index, &message_);
    }

   private:
    struct TRdmMessage message_;
};

#endif  // RDMMESSAGE_H_
