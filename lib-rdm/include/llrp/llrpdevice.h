/**
 * @file llrpdevice.h
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

#ifndef LLRP_LLRPDEVICE_H_
#define LLRP_LLRPDEVICE_H_

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "network.h"
#include "llrp/llrppacket.h"
#include "e133.h"
#include "e120.h"
#include "rdmhandler.h"
#include "ip4/ip4_address.h"
#include "apps/mdns.h"
#include "firmware/debug/debug_debug.h"

namespace llrp::device
{
static constexpr auto kIpV4LlrpRequest = network::ConvertToUint(239, 255, 250, 133);
static constexpr auto kIpV4LlrpResponse = network::ConvertToUint(239, 255, 250, 134);
static constexpr uint16_t kLlrpPort = 5569;
} // namespace llrp::device

class LLRPDevice
{
   public:
    LLRPDevice()
    {
        DEBUG_ENTRY();
        assert(s_this == nullptr);
        s_this = this;

        handle_llrp = network::udp::Begin(llrp::device::kLlrpPort, LLRPDevice::StaticCallbackFunction);
        assert(handle_llrp != -1);
        network::igmp::JoinGroup(handle_llrp, llrp::device::kIpV4LlrpRequest);

        network::apps::mdns::ServiceRecordAdd(nullptr, network::apps::mdns::Services::kRdmnetLlrp, "node=RDMNet LLRP Only");

        DEBUG_EXIT();
    }

    ~LLRPDevice()
    {
        DEBUG_ENTRY();

		network::apps::mdns::ServiceRecordDelete(network::apps::mdns::Services::kRdmnetLlrp);
		
        network::igmp::LeaveGroup(handle_llrp, llrp::device::kIpV4LlrpRequest);
        network::udp::End(llrp::device::kLlrpPort);

        s_this = nullptr;

        DEBUG_EXIT();
    }

    void Input(const uint8_t* buffer, [[maybe_unused]] uint32_t size, uint32_t from_ip, [[maybe_unused]] uint16_t from_port)
    {
        llrp = const_cast<uint8_t*>(buffer);
        ip_address_from = from_ip;

#ifndef NDEBUG
        DumpCommon();
#endif

        const auto* common = reinterpret_cast<struct TLLRPCommonPacket*>(llrp);

        switch (__builtin_bswap32(common->LlrpPDU.vector))
        {
            case VECTOR_LLRP_PROBE_REQUEST:
#ifdef SHOW_LLRP_MESSAGE
                printf("> VECTOR_LLRP_PROBE_REQUEST\n");
                DumpLLRP();
#endif
                HandleRequestMessage();
                break;
            case VECTOR_LLRP_PROBE_REPLY:
                // Nothing to do here
                DEBUG_PUTS("VECTOR_LLRP_PROBE_REPLY");
                break;
            case VECTOR_LLRP_RDM_CMD:
#ifdef SHOW_LLRP_MESSAGE
                printf("> VECTOR_LLRP_RDM_CMD\n");
                DumpLLRP();
#endif
                HandleRdmCommand();
                break;
            default:
                break;
        }
    }

    void Print()
    {
        puts("LLRP Device");
        printf(" Port UDP           : %d\n", llrp::device::kLlrpPort);
        printf(" Join Request       : " IPSTR "\n", IP2STR(llrp::device::kIpV4LlrpRequest));
        printf(" Multicast Response : " IPSTR "\n", IP2STR(llrp::device::kIpV4LlrpResponse));
    }

   private:
    uint8_t* LLRPHandleRdmCommand(const uint8_t* rdm_data_no_sc)
    {
        rdm_handler_.HandleData(rdm_data_no_sc, reinterpret_cast<uint8_t*>(&rdm_command));
        return reinterpret_cast<uint8_t*>(&rdm_command);
    }

    void HandleRequestMessage();
    void HandleRdmCommand();
    // DEBUG subject for deletions
    void DumpCommon();
    void DumpLLRP();
    void DumpRdmMessageInNoSc();

    void static StaticCallbackFunction(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        s_this->Input(buffer, size, from_ip, from_port);
    }

   private:
    RDMHandler rdm_handler_{false};

    static inline int32_t handle_llrp;
    static inline uint32_t ip_address_from;
    static inline uint8_t* llrp;
    static inline TRdmMessage rdm_command;
    static inline LLRPDevice* s_this;
};

#endif // LLRP_LLRPDEVICE_H_
