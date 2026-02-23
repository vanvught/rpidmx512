/**
 * @file rdmdiscovery.h
 *
 */
/* Copyright (C) 2023-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDMDISCOVERY_H_
#define RDMDISCOVERY_H_

#include <rdmtod.h>
#include <cstdint>
#include <algorithm>

#include "rdmmessage.h"

namespace rdmdiscovery
{
#ifndef NDEBUG
inline constexpr uint32_t kReceiveTimeOut = 58000;
inline constexpr uint32_t kLateResponseTimeOut = 40000;
#else
inline constexpr uint32_t kReceiveTimeOut = 5800;
inline constexpr uint32_t kLateResponseTimeOut = 1000;
#endif
inline constexpr uint32_t kUnmuteCounter = 3;
inline constexpr uint32_t kMuteCounter = 10;
inline constexpr uint32_t kDiscoveryStackSize = rdmtod::kTodTableSize;
inline constexpr uint32_t kDiscoveryCounter = 3;
inline constexpr uint32_t kQuikfindCounter = 5;
inline constexpr uint32_t kQuikfindDiscoveryCounter = 5;

enum class State
{
    kIdle,
    kUnmute,
    kMute,
    kDiscovery,
    kDiscoverySingleDevice,
    kDub,
    kQuickfind,
    kQuickfindDiscovery,
    kLateResponse,
    kFinished
};
} // namespace rdmdiscovery

class RDMDiscovery
{
   public:
    explicit RDMDiscovery(const uint8_t* uid);

    bool Full(uint32_t port_index, RDMTod* tod);
    bool Incremental(uint32_t port_index, RDMTod* tod);

    bool Stop();

    bool IsRunning(uint32_t& port_index, bool& is_incremental) const
    {
        port_index = port_index_;
        is_incremental = do_incremental_;
        return (state_ != rdmdiscovery::State::kIdle);
    }

    bool IsFinished(uint32_t& port_index, bool& is_incremental)
    {
        port_index = port_index_;
        is_incremental = do_incremental_;

        if (is_finished_)
        {
            is_finished_ = false;
            return true;
        }

        return false;
    }

    uint32_t CopyWorkingQueue(char* out_buffer, uint32_t out_buffer_size);

    void Run()
    {
        if (__builtin_expect((state_ == rdmdiscovery::State::kIdle), 1))
        {
            return;
        }

        Process();
    }

   private:
    void Process();
    bool Start(uint32_t port_index, RDMTod* tod, bool do_incremental);
    bool IsValidDiscoveryResponse(uint8_t* uid);

    void SavedState(uint32_t line);
    void NewState(rdmdiscovery::State state, bool do_state_late_response, uint32_t line);

   private:
    RDMMessage message_;
    uint8_t* response_{nullptr};
    uint8_t uid_[RDM_UID_SIZE];
    uint32_t port_index_{0};
    RDMTod* tod_{nullptr};

    bool is_finished_{false};
    bool do_incremental_{false};
    rdmdiscovery::State state_{rdmdiscovery::State::kIdle};
    rdmdiscovery::State saved_state_{rdmdiscovery::State::kIdle};

    struct
    {
        uint32_t micros;
    } late_response_;

    struct
    {
        uint32_t counter;
        uint32_t micros;
        bool is_command_running;
    } unmute_;

    struct
    {
        uint32_t tod_entries;
        uint32_t counter;
        uint32_t micros;
        uint8_t uid[RDM_UID_SIZE];
        bool is_command_running;
    } mute_;

    struct
    {
        struct
        {
            bool Push(uint64_t lower_bound, uint64_t upper_bound)
            {
                if (top == rdmdiscovery::kDiscoveryStackSize - 1)
                {
                    assert(0);
                    return false;
                }

                top++;
                items[top].lower_bound = lower_bound;
                items[top].upper_bound = upper_bound;

                debug_stack_top_max = std::max(debug_stack_top_max, top);
                return true;
            }

            bool Pop(uint64_t& lower_bound, uint64_t& upper_bound)
            {
                if (top == -1)
                {
                    return false;
                }

                lower_bound = items[top].lower_bound;
                upper_bound = items[top].upper_bound;
                top--;

                return true;
            }

            int32_t top;

            struct
            {
                uint64_t lower_bound;
                uint64_t upper_bound;
            } items[rdmdiscovery::kDiscoveryStackSize];

            int32_t debug_stack_top_max;
        } stack;

        uint64_t lower_bound;
        uint64_t mid_position;
        uint64_t upper_bound;
        uint32_t counter;
        uint32_t micros;
        uint8_t uid[RDM_UID_SIZE];
        uint8_t pdl[2][RDM_UID_SIZE];
        bool is_command_running;
    } discovery_;

    struct
    {
        uint32_t counter;
        uint32_t micros;
        bool is_command_running;
    } discovery_single_device_;

    struct
    {
        uint32_t counter;
        uint32_t micros;
        bool is_command_running;
        uint8_t uid[RDM_UID_SIZE];
    } quick_find_;

    struct
    {
        uint32_t counter;
        uint32_t micros;
        bool is_command_running;
        uint8_t uid[RDM_UID_SIZE];
    } quick_find_discovery_;
};

#endif  // RDMDISCOVERY_H_
