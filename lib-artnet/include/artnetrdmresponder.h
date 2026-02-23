/**
 * @file artnetrdmresponder.h
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "rdmdeviceresponder.h"
#include "rdmpersonality.h"
#include "rdmhandler.h"
#include "e120.h"
#include "rdm_message_print.h"
#include "firmware/debug/debug_debug.h"
#if defined(NODE_RDMNET_LLRP_ONLY)
#error "Cannot be both RDMNet Device and RDM Responder"
#endif

class ArtNetRdmResponder final : public RDMDeviceResponder, RDMHandler
{
   public:
    ArtNetRdmResponder(RDMPersonality** rdm_personalities, uint32_t personality_count) : RDMDeviceResponder(rdm_personalities, personality_count)
    {
        DEBUG_ENTRY();
        DEBUG_EXIT();
    }

    ~ArtNetRdmResponder() override = default;

    void TodCopy(uint32_t port_index, unsigned char* tod)
    {
        DEBUG_PRINTF("port_index=%u", port_index);

        if (port_index == 0)
        {
            memcpy(tod, RdmDevice::Get().GetUID(), RDM_UID_SIZE);
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
        rdm::MessagePrintNoStartcode(rdm_data_no_sc);
#endif

        RDMHandler::HandleData(rdm_data_no_sc, reinterpret_cast<uint8_t*>(&s_rdm_command));

        if (s_rdm_command.start_code != E120_SC_RDM)
        {
            DEBUG_EXIT();
            return nullptr;
        }

#ifndef NDEBUG
        rdm::MessagePrint(reinterpret_cast<uint8_t*>(&s_rdm_command));
#endif

        DEBUG_EXIT();
        return reinterpret_cast<const uint8_t*>(&s_rdm_command);
    }

   private:
    static inline TRdmMessage s_rdm_command;
};

#endif // ARTNETRDMRESPONDER_H_
