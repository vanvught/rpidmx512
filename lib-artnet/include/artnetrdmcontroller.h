/**
 * @file artnetrdmcontroller.h
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

#ifndef ARTNETRDMCONTROLLER_H_
#define ARTNETRDMCONTROLLER_H_

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "rdmdiscovery.h"
#include "rdmdevice.h"
#include "dmxnode.h"
#include "superloop/softwaretimers.h"

namespace artnet::rdm::controller
{
inline static constexpr uint32_t kBackgroundIntervalMinutes = 15;

void DiscoveryStart(uint32_t port_index);
void DiscoveryDone(uint32_t port_index);
} // namespace artnet::rdm::controller

class ArtNetRdmController final : RDMDiscovery
{
   public:
    ArtNetRdmController() : RDMDiscovery(RdmDevice::Get().GetUID()) { RdmDevice::Get().Print(); }

    ~ArtNetRdmController() = default;

    ArtNetRdmController(const ArtNetRdmController&) = delete;
    ArtNetRdmController& operator=(const ArtNetRdmController&) = delete;

    // Discovery process BEGIN

    void Enable(uint32_t port_index)
    {
        assert(port_index < dmxnode::kMaxPorts);
        enabled_ |= (1U << port_index);

        printf("%s: enabled=%.2x, bg=%.2x, waiting=%.2x, type=%.2x [running_=%d]\n", __FUNCTION__, enabled_, s_bg_discovery, waiting_, type_, running_);
    }

    void Disable(uint32_t port_index)
    {
        assert(port_index < dmxnode::kMaxPorts);
        enabled_ &= ~(1U << port_index);

        printf("%s: enabled=%.2x, bg=%.2x, waiting=%.2x, type=%.2x [running_=%d]\n", __FUNCTION__, enabled_, s_bg_discovery, waiting_, type_, running_);
    }

    bool IsEnabled(uint32_t port_index) const
    {
        assert(port_index < dmxnode::kMaxPorts);
        return (((1U << port_index) & enabled_) == (1U << port_index));
    }

    void EnableBackground(uint32_t port_index)
    {
        assert(port_index < dmxnode::kMaxPorts);
        if (((1U << port_index_) & enabled_) == (1U << port_index_))
        {
            s_bg_discovery |= (1U << port_index);
        }

        if (s_timer_id < 0)
        {
            s_timer_id = SoftwareTimerAdd((1000U * 60U) * artnet::rdm::controller::kBackgroundIntervalMinutes, TimerBackGround);
            printf("s_timer_id=%d\n", s_timer_id);
        }

        printf("%s: enabled=%.2x, bg=%.2x, waiting=%.2x, type=%.2x [running_=%d]\n", __FUNCTION__, enabled_, s_bg_discovery, waiting_, type_, running_);
    }

    void DisableBackground(uint32_t port_index)
    {
        assert(port_index < dmxnode::kMaxPorts);
        if (((1U << port_index_) & enabled_) == (1U << port_index_))
        {
            s_bg_discovery &= ~(1U << port_index);
        }

        if (s_bg_discovery == 0)
        {
            SoftwareTimerDelete(s_timer_id);
        }

        printf("%s: enabled=%.2x, bg=%.2x, waiting=%.2x, type=%.2x [running_=%d]\n", __FUNCTION__, enabled_, s_bg_discovery, waiting_, type_, running_);
    }

    bool IsEnabledBackground(uint32_t port_index) const { return (((1U << port_index) & s_bg_discovery) == (1U << port_index)); }

    void Full(uint32_t port_index)
    {
        assert(port_index < dmxnode::kMaxPorts);
        if (((1U << port_index_) & enabled_) == (1U << port_index_))
        {
            waiting_ |= (1U << port_index);
            type_ |= (1U << port_index);
            running_ = true;
        }

        printf("%s: enabled=%.2x, bg=%.2x, waiting=%.2x, type=%.2x [running_=%d]\n", __FUNCTION__, enabled_, s_bg_discovery, waiting_, type_, running_);
    }

    void Incremental(uint32_t port_index)
    {
        assert(port_index < dmxnode::kMaxPorts);
        if (((1U << port_index_) & enabled_) == (1U << port_index_))
        {
            waiting_ |= (1U << port_index);
            type_ &= ~(1U << port_index);
            running_ = true;
        }

        printf("%s: enabled=%.2x, bg=%.2x, waiting=%.2x, type=%.2x [running_=%d]\n", __FUNCTION__, enabled_, s_bg_discovery, waiting_, type_, running_);
    }

    void Stop(uint32_t port_index)
    {
        assert(port_index < dmxnode::kMaxPorts);
        if (((1U << port_index_) & enabled_) == (1U << port_index_))
        {
            bool is_incremental;
            uint32_t index;
            if (RDMDiscovery::IsRunning(index, is_incremental))
            {
                if (index == port_index)
                {
                    RDMDiscovery::Stop();
                    artnet::rdm::controller::DiscoveryDone(port_index);
                    waiting_ &= ~(1U << port_index);
                }
            }
        }

        printf("%s: enabled=%.2x, bg=%.2x, waiting=%.2x, type=%.2x [running_=%d]\n", __FUNCTION__, enabled_, s_bg_discovery, waiting_, type_, running_);
    }

    bool IsRunning(uint32_t port_index, bool& is_incremental)
    {
        assert(port_index < dmxnode::kMaxPorts);

        uint32_t portindex;

        if (RDMDiscovery::IsRunning(portindex, is_incremental))
        {
            return port_index == portindex;
        }

        return false;
    }

    bool IsRunning(uint32_t port_index)
    {
        assert(port_index < dmxnode::kMaxPorts);
        bool is_incremental;
        return IsRunning(port_index, is_incremental);
    }

    void Run()
    {
        RDMDiscovery::Run();

        if (__builtin_expect((!running_), 1))
        {
            return;
        }

        uint32_t port_index;
        bool is_incremental;

        const auto kIsRunning = RDMDiscovery::IsRunning(port_index, is_incremental);

        if ((!kIsRunning) && (waiting_))
        {
            if (((1U << port_index_) & waiting_) == (1U << port_index_))
            {
                if (((1U << port_index_) & type_) == (1U << port_index_))
                {
                    RDMDiscovery::Full(port_index_, &s_tod[port_index_]);
                    printf("Full:%u\n", port_index_);
                }
                else
                {
                    RDMDiscovery::Incremental(port_index_, &s_tod[port_index_]);
                    printf("Incremental:%u\n", port_index_);
                }

                artnet::rdm::controller::DiscoveryStart(port_index_);

                waiting_ &= ~(1U << port_index_);
            }
            else
            {
                port_index_++;
                if (port_index_ == dmxnode::kMaxPorts) port_index_ = 0;
            }

            return;
        }

        if (RDMDiscovery::IsFinished(port_index, is_incremental))
        {
            assert(port_index_ == port_index);
            artnet::rdm::controller::DiscoveryDone(port_index_);

            port_index_++;
            if (port_index_ == dmxnode::kMaxPorts) port_index_ = 0;

            if (waiting_ == 0)
            {
                running_ = false;
                port_index_ = 0;
            }
        }
    }

    // Discovery process END

    uint32_t GetUidCount(uint32_t port_index)
    {
        assert(port_index < dmxnode::kMaxPorts);
        return s_tod[port_index].GetUidCount();
    }

    void TodCopy(uint32_t port_index, uint8_t* tod)
    {
        assert(port_index < dmxnode::kMaxPorts);
        s_tod[port_index].Copy(tod);
    }

    uint32_t CopyWorkingQueue(char* out_buffer, uint32_t out_buffer_size) { return RDMDiscovery::CopyWorkingQueue(out_buffer, out_buffer_size); }

    uint32_t CopyTod(uint32_t port_index, char* out_buffer, uint32_t out_buffer_size)
    {
        assert(port_index < dmxnode::kMaxPorts);

        const auto kSize = static_cast<int32_t>(out_buffer_size);
        int32_t length = 0;

        for (uint32_t count = 0; count < s_tod[port_index].GetUidCount(); count++)
        {
            uint8_t uid[RDM_UID_SIZE];

            s_tod[port_index].CopyUidEntry(count, uid);

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
        s_tod[port_index].Reset();
    }

    bool TodAddUid(uint32_t port_index, const uint8_t* uid) { return s_tod[port_index].AddUid(uid); }

    // Generic

    bool CopyTodEntry(uint32_t port_index, uint32_t index, uint8_t uid[RDM_UID_SIZE])
    {
        assert(port_index < dmxnode::kMaxPorts);
        return s_tod[port_index].CopyUidEntry(index, uid);
    }

    void TodDump(uint32_t port_index)
    {
        assert(port_index < dmxnode::kMaxPorts);
        s_tod[port_index].Dump();
    }

    RDMTod* GetTod(uint32_t port_index)
    {
        assert(port_index < dmxnode::kMaxPorts);
        return &s_tod[port_index];
    }

   private:
    static ArtNetRdmController& Instance()
    {
        assert(s_this != nullptr);
        return *s_this;
    }

    static void TimerBackGround([[maybe_unused]] TimerHandle_t handle)
    {
        for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
        {
            if (((1U << port_index) & s_bg_discovery) == (1U << port_index))
            {
                ArtNetRdmController::Instance().Incremental(port_index);
            }
        }
    }

   private:
    uint8_t port_index_{0};
    uint8_t enabled_{0};
    uint8_t waiting_{0};
    uint8_t type_{0};
    bool running_{false};

    inline static ArtNetRdmController* s_this;
    inline static RDMTod s_tod[dmxnode::kMaxPorts];

    inline static uint8_t s_bg_discovery{0};
    inline static int32_t s_timer_id = -1;
};

#endif // ARTNETRDMCONTROLLER_H_
