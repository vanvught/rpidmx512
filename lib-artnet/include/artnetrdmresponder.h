/**
 * @file artnetrdmresponder.h
 *
 */
/* Copyright (C) 2018-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef ARTNETRDMRESPONDER_H_
#define ARTNETRDMRESPONDER_H_

#include <cstdint>
#include <cstring>

#include "rdmdevice.h"
#include "rdmdeviceresponder.h"
#include "rdmpersonality.h"
#include "rdmhandler.h"
#include "e120.h"
#include "rdm_message_print.h"
#include "firmware/debug/debug_debug.h"

class ArtNetRdmResponder final : public RDMDeviceResponder
{
   public:
    ArtNetRdmResponder(RDMPersonality** rdm_personalities, uint32_t personality_count) : RDMDeviceResponder(rdm_personalities, personality_count)
    {
        DEBUG_ENTRY();

        DEBUG_EXIT();
    }

    ~ArtNetRdmResponder() override = default;

    void Print()
    {
        RDMDeviceResponder::Print();
    }

    void TodCopy(uint32_t port_index, unsigned char* tod)
    {
        DEBUG_PRINTF("port_index=%u", port_index);

        if (port_index == 0)
        {
            memcpy(tod, rdm::device::Base::Instance().GetUID(), RDM_UID_SIZE);
        }
        else
        {
            memcpy(tod, UID_ALL, RDM_UID_SIZE);
        }
    }

    const uint8_t* Handler(uint32_t port_index, const uint8_t* rdm_data_no_sc)
    {
        DEBUG_ENTRY();

        if (port_index != 0)
        {
            DEBUG_EXIT();
            return nullptr;
        }

        if (rdm_data_no_sc == nullptr)
        {
            DEBUG_EXIT();
            return nullptr;
        }

#ifndef NDEBUG
        rdm::message::PrintNoStartcode(rdm_data_no_sc);
#endif

        RDMHandler::Instance().HandleData(rdm_data_no_sc, reinterpret_cast<uint8_t*>(&s_rdm_command), RDMHandler::Type::kTypeRdm);

        if (s_rdm_command.start_code != E120_SC_RDM)
        {
            DEBUG_EXIT();
            return nullptr;
        }

#ifndef NDEBUG
        rdm::message::Print(reinterpret_cast<uint8_t*>(&s_rdm_command));
#endif

        DEBUG_EXIT();
        return reinterpret_cast<const uint8_t*>(&s_rdm_command);
    }

   private:
    static inline TRdmMessage s_rdm_command;
};

#endif // ARTNETRDMRESPONDER_H_
