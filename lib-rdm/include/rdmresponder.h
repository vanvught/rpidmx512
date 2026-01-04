/**
 * @file rdmresponder.h
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

#ifndef RDMRESPONDER_H_
#define RDMRESPONDER_H_

#include <cstdint>
#include <cassert>

#include "rdm.h"
#include "rdmhandler.h"
#include "rdmdeviceresponder.h"
#include "rdmpersonality.h"
#include "dmxreceiver.h"
#include "dmxnode_outputtype.h"
#include "rdm_message_print.h"
 #include "firmware/debug/debug_debug.h"

#if defined(NODE_RDMNET_LLRP_ONLY)
#error "Cannot be both RDMNet Device and RDM Responder"
#endif

namespace rdm::responder
{
inline constexpr int kNoData = 0;
inline constexpr int kDiscoveryResponse = -1;
inline constexpr int kInvalidDataReceived = -2;
inline constexpr int kInvalidResponse = -3;
} // namespace rdm::responder

namespace configstore
{
void Delay();
} // namespace configstore

class RDMResponder final : DMXReceiver, public RDMDeviceResponder, RDMHandler
{
   public:
    RDMResponder(RDMPersonality** personalities, uint32_t personality_count, uint32_t current_personality = rdm::device::responder::kDefaultCurrentPersonality)
        : DMXReceiver(personalities[current_personality - 1]->GetDmxNodeOutputType()), RDMDeviceResponder(personalities, personality_count, current_personality)
    {
        assert(s_this == nullptr);
        s_this = this;
    }

    ~RDMResponder() override = default;

    void Init()
    {
        RDMDeviceResponder::Init();
        // There is no DMXReceiver::Init()
    }

    int Run()
    {
        int16_t length;

#if !defined(CONFIG_RDM_ENABLE_SUBDEVICES)
        DMXReceiver::Run(length);
#else
        const auto* dmx_data_in = DMXReceiver::Run(length);

        if (RDMSubDevices::Get()->GetCount() != 0)
        {
            if (length == -1)
            {
                if (is_sub_device_active)
                {
                    RDMSubDevices::Get()->Stop();
                    is_sub_device_active = false;
                }
            }
            else if (dmx_data_in != nullptr)
            {
                RDMSubDevices::Get()->SetData(dmx_data_in, static_cast<uint16_t>(length));
                if (!is_sub_device_active)
                {
                    RDMSubDevices::Get()->Start();
                    is_sub_device_active = true;
                }
            }
        }
#endif

        const auto* rdm_data_in = Rdm::Receive(0);

        if (rdm_data_in == nullptr) [[likely]]
        {
            return rdm::responder::kNoData;
        }

#ifndef NDEBUG
        rdm::MessagePrint(rdm_data_in);
#endif

        if (rdm_data_in[0] == E120_SC_RDM)
        {
            const auto* rdm_in = reinterpret_cast<const struct TRdmMessage*>(rdm_data_in);

            switch (rdm_in->command_class)
            {
                case E120_DISCOVERY_COMMAND:
                case E120_GET_COMMAND:
                case E120_SET_COMMAND:
                    HandleData(&rdm_data_in[1], reinterpret_cast<uint8_t*>(&rdm_command));
                    return HandleResponse(reinterpret_cast<uint8_t*>(&rdm_command));
                    break;
                default:
                    DEBUG_PUTS("RDM_RESPONDER_INVALID_DATA_RECEIVED");
                    return rdm::responder::kInvalidDataReceived;
                    break;
            }
        }

        DEBUG_PUTS("RDM_RESPONDER_DISCOVERY_RESPONSE");
        return rdm::responder::kDiscoveryResponse;
    }

    void Print()
    {
        RDMDeviceResponder::Print();
        DMXReceiver::Print();
    }

    void Start()
    {
        // There is no RDMDeviceResponder::Start()
        DMXReceiver::Start();
    }

    void DmxDisableOutput(bool disable) { DMXReceiver::SetDisableOutput(disable); }

    static RDMResponder* Get() { return s_this; }

    void PersonalityUpdate(uint32_t personality) __attribute__((weak));
    void DmxStartAddressUpdate(uint16_t dmx_start_address) __attribute__((weak));

   private:
    int HandleResponse(const uint8_t* response)
    {
        auto length = rdm::responder::kInvalidResponse;

        if (response[0] == E120_SC_RDM)
        {
            const auto* p = reinterpret_cast<const struct TRdmMessage*>(response);
            length = static_cast<int>(p->message_length + RDM_MESSAGE_CHECKSUM_SIZE);
            Rdm::SendRawRespondMessage(0, response, static_cast<uint16_t>(length));
        }
        else if (response[0] == 0xFE)
        {
            length = sizeof(struct TRdmDiscoveryMsg);
            Rdm::SendDiscoveryRespondMessage(0, response, static_cast<uint16_t>(length));
        }

#ifndef NDEBUG
        if (length != rdm::responder::kInvalidResponse)
        {
            rdm::MessagePrint(response);
        }
#endif

        configstore::Delay();
        return length;
    }

    void PersonalityUpdate(DmxNodeOutputType* dmx_node_output_type) override
    {
        DMXReceiver::SetDmxNodeOutputType(dmx_node_output_type);
        PersonalityUpdate(static_cast<uint32_t>(RDMDeviceResponder::GetPersonalityCurrent(RDM_ROOT_DEVICE)));
    }

    void DmxStartAddressUpdate() override { DmxStartAddressUpdate(RDMDeviceResponder::GetDmxStartAddress(RDM_ROOT_DEVICE)); }

   private:
    static inline TRdmMessage rdm_command;
    static inline bool is_sub_device_active;

    static inline RDMResponder* s_this;
};

#endif  // RDMRESPONDER_H_
