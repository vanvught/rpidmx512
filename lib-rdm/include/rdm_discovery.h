/**
 * @file rdm_discovery.h
 *
 */
/* Copyright (C) 2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDM_DISCOVERY_H_
#define RDM_DISCOVERY_H_

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "rdm_discovery_statemachine.h"
#include "rdm_device_base.h"
#include "dmx.h"            // IWYU pragma: keep
#include "softwaretimers.h" // IWYU pragma: keep

namespace rdm
{
namespace discovery
{
	enum class Type {
		kFull, kIncremental
	};
	
	void Starting(uint32_t port_index, Type type);
	void Finished(uint32_t port_index, Type type);
}
inline static constexpr uint32_t kBackgroundIntervalMinutes = 1;

class Discovery : rdm::discovery::StateMachine
{
    static constexpr auto kPorts = dmx::config::max::PORTS;

   public:
    Discovery() : rdm::discovery::StateMachine(rdm::device::Base::Instance().GetUID())
    {
        assert(s_this == nullptr);
        s_this = this;
    }

    ~Discovery() = default;

    Discovery(const Discovery&) = delete;
    Discovery& operator=(const Discovery&) = delete;

    void Print() { rdm::device::Base::Instance().Print(); }

    void Enable(uint32_t port_index)
    {
        assert(port_index < kPorts);
        enabled_ |= (1U << port_index);

        printf("%s: %u -> enabled=%.2x, bg=%.2x, waiting=%.2x, type=%.2x [running=%d]\n", __FUNCTION__, port_index, enabled_, s_bg_discovery, waiting_, type_,
               running_);
    }

    bool IsEnabled(uint32_t port_index) const { return (((1U << port_index) & enabled_) == (1U << port_index)); }

    void Disable(uint32_t port_index)
    {
        assert(port_index < kPorts);
        enabled_ &= ~(1U << port_index);

        printf("%s: %u -> enabled=%.2x, bg=%.2x, waiting=%.2x, type=%.2x [running=%d]\n", __FUNCTION__, port_index, enabled_, s_bg_discovery, waiting_, type_,
               running_);
    }

    void EnableBackground(uint32_t port_index)
    {
        assert(port_index < kPorts);
        if (((1U << port_index) & enabled_) == (1U << port_index))
        {
            s_bg_discovery |= (1U << port_index);
        }

        if (s_timer_id < 0)
        {
            s_timer_id = SoftwareTimerAdd((1000U * 60U) * kBackgroundIntervalMinutes, TimerBackGround);
            printf("s_timer_id=%d\n", s_timer_id);
        }

        printf("%s: %u -> enabled=%.2x, bg=%.2x, waiting=%.2x, type=%.2x [running=%d]\n", __FUNCTION__, port_index, enabled_, s_bg_discovery, waiting_, type_,
               running_);
    }

    void DisableBackground(uint32_t port_index)
    {
        assert(port_index < kPorts);
        if (((1U << port_index) & enabled_) == (1U << port_index))
        {
            s_bg_discovery &= ~(1U << port_index);
            Stop(port_index);
        }

        if (s_bg_discovery == 0)
        {
            SoftwareTimerDelete(s_timer_id);
        }

        printf("%s: %u -> enabled=%.2x, bg=%.2x, waiting=%.2x, type=%.2x [running=%d]\n", __FUNCTION__, port_index, enabled_, s_bg_discovery, waiting_, type_,
               running_);
    }

    bool IsEnabledBackground(uint32_t port_index) const { return (((1U << port_index) & s_bg_discovery) == (1U << port_index)); }

    void Full(uint32_t port_index)
    {
        assert(port_index < kPorts);
        if (((1U << port_index) & enabled_) == (1U << port_index))
        {
            waiting_ |= (1U << port_index);
            type_ |= (1U << port_index);
            running_ = true;
        }

        printf("%s: %u -> enabled=%.2x, bg=%.2x, waiting=%.2x, type=%.2x [running=%d]\n", __FUNCTION__, port_index, enabled_, s_bg_discovery, waiting_, type_,
               running_);
    }

    void Incremental(uint32_t port_index)
    {
        assert(port_index < kPorts);
        if (((1U << port_index) & enabled_) == (1U << port_index))
        {
            waiting_ |= (1U << port_index);
            type_ &= ~(1U << port_index);
            running_ = true;
        }

        printf("%s: %u -> enabled=%.2x, bg=%.2x, waiting=%.2x, type=%.2x [running=%d]\n", __FUNCTION__, port_index, enabled_, s_bg_discovery, waiting_, type_,
               running_);
    }

    void Stop(uint32_t port_index)
    {
        assert(port_index < kPorts);
        if (((1U << port_index) & enabled_) == (1U << port_index))
        {
            bool is_incremental;
            uint32_t index;
            if (rdm::discovery::StateMachine::IsRunning(index, is_incremental))
            {
                if (index == port_index)
                {
                    rdm::discovery::StateMachine::Stop();
                    waiting_ &= ~(1U << port_index);
                }
            }
        }

        printf("%s: %u -> enabled=%.2x, bg=%.2x, waiting=%.2x, type=%.2x [running=%d]\n", __FUNCTION__, port_index, enabled_, s_bg_discovery, waiting_, type_,
               running_);
    }

    bool IsRunning(uint32_t portindex, bool& is_incremental) { return rdm::discovery::StateMachine::IsRunning(portindex, is_incremental); }

    bool IsRunning(uint32_t port_index)
    {
        assert(port_index < kPorts);

        uint32_t portindex;
        bool is_incremental;

        if (rdm::discovery::StateMachine::IsRunning(portindex, is_incremental))
        {
            return port_index == portindex;
        }

        return false;
    }

    void GetStatus(uint8_t data[5])
    {
        data[0] = enabled_;
        data[1] = waiting_;
        data[2] = type_;
        data[3] = s_bg_discovery;
        data[4] = 0;
        for (uint32_t port_index = 0; port_index < kPorts; port_index++)
        {
            data[4] |= (IsRunning(port_index) ? (1U << port_index) : 0);
        }
    }

    uint32_t CopyWorkingQueue(char* out_buffer, uint32_t out_buffer_size)
    {
        return rdm::discovery::StateMachine::CopyWorkingQueue(out_buffer, out_buffer_size);
    }

    void Run()
    {
        rdm::discovery::StateMachine::Run();

        if (__builtin_expect((!running_), 1))
        {
            return;
        }

        uint32_t port_index;
        bool is_incremental;

        const auto kIsRunning = rdm::discovery::StateMachine::IsRunning(port_index, is_incremental);

        if ((!kIsRunning) && (waiting_))
        {
            if (((1U << port_index_) & waiting_) == (1U << port_index_))
            {
                if (((1U << port_index_) & type_) == (1U << port_index_))
                {
					rdm::discovery::Starting(port_index, rdm::discovery::Type::kFull);
                    rdm::discovery::StateMachine::Full(port_index_, &s_tod[port_index_]);
					printf("Full:%u\n", port_index_);
                }
                else
                {
					rdm::discovery::Starting(port_index, rdm::discovery::Type::kIncremental);
                    rdm::discovery::StateMachine::Incremental(port_index_, &s_tod[port_index_]);
					printf("Incremental:%u\n", port_index_);
                }

                waiting_ &= ~(1U << port_index_);
            }
            else
            {
                port_index_++;
                if (port_index_ == kPorts) port_index_ = 0;
            }

            return;
        }

        if (rdm::discovery::StateMachine::IsFinished(port_index, is_incremental))
        {
            assert(port_index_ == port_index);
            printf("Finished:%u\n", port_index);
			rdm::discovery::Finished(port_index, is_incremental ? rdm::discovery::Type::kIncremental : rdm::discovery::Type::kFull);

            port_index_++;
            if (port_index_ == kPorts) port_index_ = 0;

            if (waiting_ == 0)
            {
                running_ = false;
                port_index_ = 0;
            }
        }
    }

    uint32_t TodUidCount(uint32_t port_index)
    {
        assert(port_index < kPorts);
        return s_tod[port_index].UidCount();
    }

    bool TodCopyUidEntry(uint32_t port_index, uint32_t index, uint8_t uid[RDM_UID_SIZE])
    {
        assert(port_index < kPorts);
        return s_tod[port_index].CopyUidEntry(index, uid);
    }

    void TodCopy(uint32_t port_index, uint8_t* tod)
    {
        assert(port_index < kPorts);
        s_tod[port_index].Copy(tod);
    }

    void TodReset(uint32_t port_index)
    {
        assert(port_index < kPorts);
        s_tod[port_index].Reset();
    }

    bool TodAddUid(uint32_t port_index, const uint8_t* uid)
    {
        assert(port_index < kPorts);
        return s_tod[port_index].AddUid(uid);
    }

    bool TodExist(uint32_t port_index, const uint8_t* uid)
    {
        assert(port_index < kPorts);
        return s_tod[port_index].Exist(uid);
    }

    const uint8_t* TodNext(uint32_t port_index)
    {
        assert(port_index < kPorts);
        return s_tod[port_index].Next();
    }

    bool TodIsMuted(uint32_t port_index)
    {
        assert(port_index < kPorts);
        return s_tod[port_index].IsMuted();
    }

    void TodMute(uint32_t port_index)
    {
        assert(port_index < kPorts);
        s_tod[port_index].Mute();
    }

    void TodUnMute(uint32_t port_index)
    {
        assert(port_index < kPorts);
        s_tod[port_index].UnMute();
    }

    void TodUnMuteAll(uint32_t port_index)
    {
        assert(port_index < kPorts);
        s_tod[port_index].UnMuteAll();
    }

    void TodDump(uint32_t port_index)
    {
        assert(port_index < kPorts);
        s_tod[port_index].Dump();
    }

    static Discovery& Instance()
    {
        assert(s_this != nullptr);
        return *s_this;
    }

    static void TimerBackGround([[maybe_unused]] TimerHandle_t handle)
    {
        for (uint32_t port_index = 0; port_index < kPorts; port_index++)
        {
            if (((1U << port_index) & s_bg_discovery) == (1U << port_index))
            {
                Discovery::Instance().Incremental(port_index);
            }
        }
    }

   private:
    uint8_t port_index_{0};
    uint8_t enabled_{0};
    uint8_t waiting_{0};
    uint8_t type_{0};
    bool running_{false};

    inline static Discovery* s_this;
    inline static rdm::Tod s_tod[kPorts];

    inline static uint8_t s_bg_discovery{0};
    inline static int32_t s_timer_id = -1;
};
} // namespace rdm

#endif // RDM_DISCOVERY_H_
