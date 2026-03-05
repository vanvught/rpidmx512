/**
 * @file artnetrdmcontroller.h
 *
 */
/* Copyright (C) 2017-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef ARTNETRDMCONTROLLER_H_
#define ARTNETRDMCONTROLLER_H_

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "rdm_discovery.h"
#include "rdm_device_base.h"
#include "dmxnode.h"
#if defined(NODE_RDMNET_LLRP_ONLY)
#include "rdmdevice.h"
#include "llrp/llrpdevice.h"
#endif

namespace artnet::rdm::controller
{
inline static constexpr uint32_t kBackgroundIntervalMinutes = 15;

void DiscoveryStart(uint32_t port_index);
void DiscoveryDone(uint32_t port_index);
} // namespace artnet::rdm::controller

#if defined(NODE_RDMNET_LLRP_ONLY)
class ArtNetRdmController final : rdm::Discovery, LLRPDevice
#else
class ArtNetRdmController final : rdm::Discovery
#endif
{
   public:
    ArtNetRdmController()
    {
#if defined(NODE_RDMNET_LLRP_ONLY)
		rdm::device::Device::Instance().Print();
#else
        rdm::device::Base::Instance().Print();
#endif
    }

    ~ArtNetRdmController() = default;

    ArtNetRdmController(const ArtNetRdmController&) = delete;
    ArtNetRdmController& operator=(const ArtNetRdmController&) = delete;

    // Discovery process BEGIN

    void Enable(uint32_t port_index)
    {
        auto& rdmdiscovery = rdm::Discovery::Instance();
        rdmdiscovery.Enable(port_index);
    }

    void Disable(uint32_t port_index)
    {
        auto& rdmdiscovery = rdm::Discovery::Instance();
        rdmdiscovery.Disable(port_index);
    }

    bool IsEnabled(uint32_t port_index) const
    {
        auto& rdmdiscovery = rdm::Discovery::Instance();
        return rdmdiscovery.IsEnabled(port_index);
    }

    void EnableBackground(uint32_t port_index)
    {
        auto& rdmdiscovery = rdm::Discovery::Instance();
        rdmdiscovery.EnableBackground(port_index);
    }

    void DisableBackground(uint32_t port_index)
    {
        auto& rdmdiscovery = rdm::Discovery::Instance();
        rdmdiscovery.DisableBackground(port_index);
    }

    bool IsEnabledBackground(uint32_t port_index) const
    {
        auto& rdmdiscovery = rdm::Discovery::Instance();
        return rdmdiscovery.IsEnabledBackground(port_index);
    }

    void Full(uint32_t port_index)
    {
        auto& rdmdiscovery = rdm::Discovery::Instance();
        rdmdiscovery.Full(port_index);
    }

    void Incremental(uint32_t port_index)
    {
        auto& rdmdiscovery = rdm::Discovery::Instance();
        rdmdiscovery.Incremental(port_index);
    }

    void Stop(uint32_t port_index)
    {
        auto& rdmdiscovery = rdm::Discovery::Instance();
        rdmdiscovery.Stop(port_index);
    }

    bool IsRunning(uint32_t port_index, bool& is_incremental)
    {
        auto& rdmdiscovery = rdm::Discovery::Instance();
        return rdmdiscovery.IsRunning(port_index, is_incremental);
    }

    bool IsRunning(uint32_t port_index)
    {
        assert(port_index < dmxnode::kMaxPorts);
        bool is_incremental;
        return IsRunning(port_index, is_incremental);
    }

    void Run()
    {
        auto& rdmdiscovery = rdm::Discovery::Instance();
        rdmdiscovery.Run();
    }

    uint32_t CopyWorkingQueue(char* out_buffer, uint32_t out_buffer_size)
    {
        auto& rdmdiscovery = rdm::Discovery::Instance();
        return rdmdiscovery.CopyWorkingQueue(out_buffer, out_buffer_size);
    }

    // Discovery process END

    uint32_t TodUidCount(uint32_t port_index)
    {
        assert(port_index < dmxnode::kMaxPorts);
        auto& rdmdiscovery = rdm::Discovery::Instance();
        return rdmdiscovery.TodUidCount(port_index);
    }

    void TodCopy(uint32_t port_index, uint8_t* tod)
    {
        assert(port_index < dmxnode::kMaxPorts);
        auto& rdmdiscovery = rdm::Discovery::Instance();
        rdmdiscovery.TodCopy(port_index, tod);
    }

    uint32_t CopyTod(uint32_t port_index, char* out_buffer, uint32_t out_buffer_size)
    {
        assert(port_index < dmxnode::kMaxPorts);

        const auto kSize = static_cast<int32_t>(out_buffer_size);
        int32_t length = 0;

        for (uint32_t count = 0; count < TodUidCount(port_index); count++)
        {
            uint8_t uid[RDM_UID_SIZE];

            CopyTodEntry(port_index, count, uid);

            length += snprintf(&out_buffer[length], static_cast<size_t>(kSize - length), "\"%.2x%.2x:%.2x%.2x%.2x%.2x\",", uid[0], uid[1], uid[2], uid[3],
                               uid[4], uid[5]);
        }

        if (length == 0)
        {
            return 0;
        }

        out_buffer[length - 1] = '\0';

        return static_cast<uint32_t>(length - 1);
    }

    // Gateway

    bool RdmReceive(uint32_t port_index, const uint8_t* rdm_data);

    void TodReset(uint32_t port_index)
    {
        assert(port_index < dmxnode::kMaxPorts);
        auto& rdmdiscovery = rdm::Discovery::Instance();
        rdmdiscovery.TodReset(port_index);
    }

    bool TodAddUid(uint32_t port_index, const uint8_t* uid)
    {
        auto& rdmdiscovery = rdm::Discovery::Instance();
        return rdmdiscovery.TodAddUid(port_index, uid);
    }

    // Generic

    bool CopyTodEntry(uint32_t port_index, uint32_t index, uint8_t uid[RDM_UID_SIZE])
    {
        assert(port_index < dmxnode::kMaxPorts);
        auto& rdmdiscovery = rdm::Discovery::Instance();
        return rdmdiscovery.TodCopyUidEntry(port_index, index, uid);
    }

    void TodDump(uint32_t port_index)
    {
        assert(port_index < dmxnode::kMaxPorts);
        auto& rdmdiscovery = rdm::Discovery::Instance();
        rdmdiscovery.TodDump(port_index);
    }

   private:
    static ArtNetRdmController& Instance()
    {
        assert(s_this != nullptr);
        return *s_this;
    }

   private:
    inline static ArtNetRdmController* s_this;
};

#endif // ARTNETRDMCONTROLLER_H_
