/**
 * @file rdm_discovery_statemachine.cpp
 */
/* Copyright (C) 2023-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_RDM_DISCOVERY)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "rdm_discovery_statemachine.h"
#include "hal.h"
#include "hal_micros.h"
#include "firmware/debug/debug_debug.h"

namespace rdm::discovery
{
#ifndef NDEBUG
static constexpr const char* kStateName[] = {
    "IDLE", "UNMUTE", "MUTE", "DISCOVERY", "DISCOVERY_SINGLE_DEVICE", "DUB", "QUICKFIND", "QUICKFIND_DISCOVERY", "LATE_RESPONSE", "FINISHED"};
#endif

typedef union cast
{
    uint64_t uint;
    uint8_t uid[RDM_UID_SIZE];
} _cast;

static _cast uuid_cast;

static const uint8_t* ConvertUid(uint64_t uid)
{
    uuid_cast.uint = __builtin_bswap64(uid << 16);
    return uuid_cast.uid;
}

#ifndef NDEBUG
static void PrintUid(const uint8_t* uid)
{
    printf("%.2x%.2x:%.2x%.2x%.2x%.2x", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5]);
}
#endif

#define NEW_STATE(state, late) NewState(state, late, __LINE__);
#define SAVED_STATE() SavedState(__LINE__);

StateMachine::StateMachine(const uint8_t* uid)
{
    memcpy(uid_, uid, RDM_UID_SIZE);
    message_.SetSrcUid(uid);

#ifndef NDEBUG
    printf("Uid : ");
    rdm::discovery::PrintUid(uid_);
    puts("");
#endif
}

uint32_t StateMachine::CopyWorkingQueue(char* out_buffer, uint32_t out_buffer_size)
{
    const auto kSize = static_cast<int32_t>(out_buffer_size);
    int32_t index = 0;
    int32_t length = 0;
    uint8_t lower_bound[RDM_UID_SIZE];
    uint8_t upper_bound[RDM_UID_SIZE];

    while (index <= discovery_.stack.top)
    {
        memcpy(lower_bound, rdm::discovery::ConvertUid(discovery_.stack.items[index].lower_bound), RDM_UID_SIZE);
        memcpy(upper_bound, rdm::discovery::ConvertUid(discovery_.stack.items[index].upper_bound), RDM_UID_SIZE);

        length += snprintf(&out_buffer[length], static_cast<size_t>(kSize - length), "\"%.2x%.2x:%.2x%.2x%.2x%.2x-%.2x%.2x:%.2x%.2x%.2x%.2x\",", lower_bound[0],
                           lower_bound[1], lower_bound[2], lower_bound[3], lower_bound[4], lower_bound[5], upper_bound[0], upper_bound[1], upper_bound[2],
                           upper_bound[3], upper_bound[4], upper_bound[5]);

        index++;
    }

    if (length == 0)
    {
        return 0;
    }

    out_buffer[length - 1] = '\0';

    return static_cast<uint32_t>(length - 1);
}

bool StateMachine::Full(uint32_t port_index, rdm::Tod* tod)
{
    DEBUG_ENTRY();
    tod->Reset();
    const auto kStart = Start(port_index, tod, false);
    DEBUG_EXIT();
    return kStart;
}

bool StateMachine::Incremental(uint32_t port_index, rdm::Tod* tod)
{
    DEBUG_ENTRY();
    mute_.tod_entries = tod->UidCount();
    const auto kStart = Start(port_index, tod, true);
    DEBUG_EXIT();
    return kStart;
}

bool StateMachine::Start(uint32_t port_index, rdm::Tod* tod, bool do_incremental)
{
    DEBUG_ENTRY();

    if (state_ != rdm::discovery::State::kIdle)
    {
        DEBUG_PUTS("Is already running.");
        DEBUG_EXIT();
        return false;
    }

    port_index_ = port_index;
    tod_ = tod;

    do_incremental_ = do_incremental;
    is_finished_ = false;

    unmute_.counter = rdm::discovery::kUnmuteCounter;
    unmute_.is_command_running = false;

    mute_.counter = rdm::discovery::kMuteCounter;
    mute_.is_command_running = false;

    discovery_.stack.top = -1;
    discovery_.stack.Push(0x000000000000, 0xfffffffffffe);
    discovery_.counter = rdm::discovery::kDiscoveryCounter;

    discovery_.is_command_running = false;

    discovery_single_device_.counter = rdm::discovery::kMuteCounter;
    discovery_single_device_.is_command_running = false;

    quick_find_.counter = rdm::discovery::kQuikfindCounter;
    quick_find_.is_command_running = false;

    quick_find_discovery_.counter = rdm::discovery::kQuikfindDiscoveryCounter;
    quick_find_discovery_.is_command_running = false;

    NEW_STATE(rdm::discovery::State::kUnmute, false);

    DEBUG_EXIT();
    return true;
}

bool StateMachine::Stop()
{
    DEBUG_ENTRY();

    if (state_ == rdm::discovery::State::kIdle)
    {
        DEBUG_PUTS("Not running.");
        DEBUG_EXIT();
        return false;
    }

    is_finished_ = false;

    NEW_STATE(rdm::discovery::State::kIdle, true);

    DEBUG_EXIT();
    return true;
}

bool StateMachine::IsValidDiscoveryResponse(uint8_t* uid)
{
    uint8_t checksum[2];
    uint16_t rdm_checksum = 6 * 0xFF;
    auto is_valid = false;

    if (response_[0] == 0xFE)
    {
        uid[0] = response_[8] & response_[9];
        uid[1] = response_[10] & response_[11];

        uid[2] = response_[12] & response_[13];
        uid[3] = response_[14] & response_[15];
        uid[4] = response_[16] & response_[17];
        uid[5] = response_[18] & response_[19];

        checksum[0] = response_[22] & response_[23];
        checksum[1] = response_[20] & response_[21];

        for (uint32_t i = 0; i < 6; i++)
        {
            rdm_checksum = static_cast<uint16_t>(rdm_checksum + uid[i]);
        }

        if (((rdm_checksum >> 8) == checksum[1]) && ((rdm_checksum & 0xFF) == checksum[0]))
        {
            is_valid = true;
        }

#ifndef NDEBUG
        rdm::discovery::PrintUid(uid);
        printf(", checksum %.2x%.2x -> %.4x {%c}\n", checksum[1], checksum[0], rdm_checksum, is_valid ? 'Y' : 'N');
#endif
    }

    return is_valid;
}

void StateMachine::SavedState([[maybe_unused]] uint32_t line)
{
    assert(saved_state_ != state_);
#ifndef NDEBUG
    printf("State %s->%s at line %u\n", rdm::discovery::kStateName[static_cast<uint32_t>(state_)],
           rdm::discovery::kStateName[static_cast<uint32_t>(saved_state_)], line);
#endif
    state_ = saved_state_;
}

void StateMachine::NewState(rdm::discovery::State state, bool do_state_late_response, [[maybe_unused]] uint32_t line)
{
    assert(state_ != state);

    if (do_state_late_response && (state_ != rdm::discovery::State::kLateResponse))
    {
#ifndef NDEBUG
        assert(static_cast<uint32_t>(state) < sizeof(rdm::discovery::kStateName) / sizeof(rdm::discovery::kStateName[0]));
        printf("State %s->%s [%s] at line %u\n", rdm::discovery::kStateName[static_cast<uint32_t>(state_)],
               rdm::discovery::kStateName[static_cast<uint32_t>(rdm::discovery::State::kLateResponse)],
               rdm::discovery::kStateName[static_cast<uint32_t>(state)], line);
#endif
        late_response_.micros = hal::Micros();
        saved_state_ = state;
        state_ = rdm::discovery::State::kLateResponse;
    }
    else
    {
#ifndef NDEBUG
        printf("State %s->%s at line %u\n", rdm::discovery::kStateName[static_cast<uint32_t>(state_)], rdm::discovery::kStateName[static_cast<uint32_t>(state)],
               line);
#endif
        state_ = state;
    }
}

void StateMachine::Process()
{
    switch (state_)
    {
        case rdm::discovery::State::kLateResponse: ///< LATE_RESPONSE
            message_.Receive(port_index_);

            if ((hal::Micros() - late_response_.micros) > rdm::discovery::kLateResponseTimeOut)
            {
                SAVED_STATE();
            }

            return;
            break;
        case rdm::discovery::State::kUnmute: ///< UNMUTE
            if (unmute_.counter == 0)
            {
                unmute_.counter = rdm::discovery::kUnmuteCounter;
                unmute_.is_command_running = false;

                if (do_incremental_)
                {
                    NEW_STATE(rdm::discovery::State::kMute, false);
                    return;
                }

                NEW_STATE(rdm::discovery::State::kDiscovery, false);
                return;
            }

            if (!unmute_.is_command_running)
            {
                message_.SetPortID(static_cast<uint8_t>(1 + port_index_));
                message_.SetDstUid(UID_ALL);
                message_.SetCc(E120_DISCOVERY_COMMAND);
                message_.SetPid(E120_DISC_UN_MUTE);
                message_.SetPd(nullptr, 0);
                message_.Send(port_index_);

                unmute_.micros = hal::Micros();
                unmute_.is_command_running = true;
                return;
            }

            message_.Receive(port_index_);

            if ((hal::Micros() - unmute_.micros) > rdm::discovery::kReceiveTimeOut)
            {
                assert(unmute_.counter > 0);
                unmute_.counter--;
                unmute_.is_command_running = false;
            }

            return;
            break;
        case rdm::discovery::State::kMute: ///< MUTE
            if (mute_.tod_entries == 0)
            {
                mute_.is_command_running = false;
                NEW_STATE(rdm::discovery::State::kDiscovery, false);
                return;
            }

            if (mute_.counter == 0)
            {
                mute_.counter = rdm::discovery::kMuteCounter;
                mute_.is_command_running = false;
#ifndef NDEBUG
                printf("Device is gone ");
                rdm::discovery::PrintUid(mute_.uid);
                puts("");
#endif
                tod_->Delete(mute_.uid);

                if (mute_.tod_entries > 0)
                {
                    mute_.tod_entries--;
                }

                return;
            }

            if (!mute_.is_command_running)
            {
                assert(mute_.tod_entries > 0);
                tod_->CopyUidEntry(mute_.tod_entries - 1, mute_.uid);

                message_.SetPortID(static_cast<uint8_t>(1 + port_index_));
                message_.SetDstUid(mute_.uid);
                message_.SetCc(E120_DISCOVERY_COMMAND);
                message_.SetPid(E120_DISC_MUTE);
                message_.SetPd(nullptr, 0);
                message_.Send(port_index_);

                mute_.micros = hal::Micros();
                mute_.is_command_running = true;
                return;
            }

            response_ = const_cast<uint8_t*>(message_.Receive(port_index_));

            if (response_ != nullptr)
            {
                assert(mute_.tod_entries > 0);
                mute_.tod_entries--;
                mute_.is_command_running = false;
                return;
            }

            if ((hal::Micros() - mute_.micros) > rdm::discovery::kReceiveTimeOut)
            {
                assert(mute_.counter > 0);
                mute_.counter--;
                message_.Send(port_index_);
                mute_.micros = hal::Micros();
            }

            return;
            break;
        case rdm::discovery::State::kDiscovery: ///< DISCOVERY
            if (discovery_.is_command_running)
            {
                response_ = const_cast<uint8_t*>(message_.Receive(port_index_));

                if ((response_ != nullptr) || (discovery_.counter == 0))
                {
                    discovery_.is_command_running = false;
                    NEW_STATE(rdm::discovery::State::kDub, false);
                    return;
                }

                if ((hal::Micros() - discovery_.micros) > rdm::discovery::kReceiveTimeOut)
                {
                    assert(discovery_.counter > 0);
                    discovery_.counter--;
                    message_.Send(port_index_);
                    discovery_.micros = hal::Micros();
                }

                return;
            }

            if (!discovery_.stack.Pop(discovery_.lower_bound, discovery_.upper_bound))
            {
                discovery_.is_command_running = false;
                NEW_STATE(rdm::discovery::State::kFinished, true);
                return;
            }

            if (discovery_.lower_bound == discovery_.upper_bound)
            {
                quick_find_discovery_.is_command_running = false;
                NEW_STATE(rdm::discovery::State::kDiscoverySingleDevice, true);
                return;
            }

            memcpy(discovery_.pdl[0], rdm::discovery::ConvertUid(discovery_.lower_bound), RDM_UID_SIZE);
            memcpy(discovery_.pdl[1], rdm::discovery::ConvertUid(discovery_.upper_bound), RDM_UID_SIZE);

#ifndef NDEBUG
            printf("DISC_UNIQUE_BRANCH -> ");
            rdm::discovery::PrintUid(discovery_.pdl[0]);
            printf(" ");
            rdm::discovery::PrintUid(discovery_.pdl[1]);
            puts("");
#endif

            message_.SetDstUid(UID_ALL);
            message_.SetCc(E120_DISCOVERY_COMMAND);
            message_.SetPid(E120_DISC_UNIQUE_BRANCH);
            message_.SetPd(reinterpret_cast<const uint8_t*>(discovery_.pdl), 2 * RDM_UID_SIZE);
            message_.Send(port_index_);

            discovery_.counter = rdm::discovery::kDiscoveryCounter;
            discovery_.micros = hal::Micros();
            discovery_.is_command_running = true;

            return;
            break;
        case rdm::discovery::State::kDiscoverySingleDevice: ///< DISCOVERY_SINGLE_DEVICE
            if (discovery_single_device_.counter == 0)
            {
                discovery_single_device_.counter = rdm::discovery::kQuikfindDiscoveryCounter;
                discovery_single_device_.is_command_running = false;
                NEW_STATE(rdm::discovery::State::kDiscovery, true);
                return;
            }

            if (!discovery_single_device_.is_command_running)
            {
                memcpy(discovery_.uid, rdm::discovery::ConvertUid(discovery_.lower_bound), RDM_UID_SIZE);

                message_.SetCc(E120_DISCOVERY_COMMAND);
                message_.SetPid(E120_DISC_MUTE);
                message_.SetDstUid(discovery_.uid);
                message_.SetPd(nullptr, 0);
                message_.Send(port_index_);

                discovery_single_device_.micros = hal::Micros();
                discovery_single_device_.is_command_running = true;
                return;
            }

            response_ = const_cast<uint8_t*>(message_.Receive(port_index_));

            if (response_ != nullptr)
            {
                const auto* response = reinterpret_cast<struct TRdmMessage*>(response_);

                if ((response->command_class == E120_DISCOVERY_COMMAND_RESPONSE) && (memcmp(discovery_.uid, response->source_uid, RDM_UID_SIZE) == 0))
                {
                    tod_->AddUid(discovery_.uid);
#ifndef NDEBUG
                    printf("AddUid : ");
                    rdm::discovery::PrintUid(discovery_.uid);
                    puts("");
#endif

                    discovery_single_device_.counter = rdm::discovery::kQuikfindDiscoveryCounter;
                    discovery_single_device_.is_command_running = false;
                    NEW_STATE(rdm::discovery::State::kDiscovery, false);
                }

                return;
            }

            if ((hal::Micros() - discovery_single_device_.micros) > rdm::discovery::kReceiveTimeOut)
            {
                assert(mute_.counter > 0);
                discovery_single_device_.counter--;
                message_.Send(port_index_);
                discovery_single_device_.micros = hal::Micros();
            }

            return;
            break;
        case rdm::discovery::State::kDub: ///< DUB
            if (response_ == nullptr)
            {
#ifndef NDEBUG
                puts("No responses");
#endif
                NEW_STATE(rdm::discovery::State::kDiscovery, false);
                return;
            }

            if (IsValidDiscoveryResponse(quick_find_.uid))
            {
                NEW_STATE(rdm::discovery::State::kQuickfind, true);
                return;
            }

            discovery_.mid_position = ((discovery_.lower_bound & (0x0000800000000000 - 1)) + (discovery_.upper_bound & (0x0000800000000000 - 1))) / 2 +
                                      (discovery_.upper_bound & (0x0000800000000000) ? 0x0000400000000000 : 0) +
                                      (discovery_.lower_bound & (0x0000800000000000) ? 0x0000400000000000 : 0);

            discovery_.stack.Push(discovery_.lower_bound, discovery_.mid_position);
            discovery_.stack.Push(discovery_.mid_position + 1, discovery_.upper_bound);

            NEW_STATE(rdm::discovery::State::kDiscovery, true);
            break;
        case rdm::discovery::State::kQuickfind: ///< QUICKFIND
            if (quick_find_.counter == 0)
            {
                quick_find_.is_command_running = false;
                NEW_STATE(rdm::discovery::State::kQuickfindDiscovery, false);
                return;
            }

            if (!quick_find_.is_command_running)
            {
#ifndef NDEBUG
                printf("QuickFind : ");
                rdm::discovery::PrintUid(quick_find_.uid);
                puts("");
#endif

                message_.SetCc(E120_DISCOVERY_COMMAND);
                message_.SetPid(E120_DISC_MUTE);
                message_.SetDstUid(quick_find_.uid);
                message_.SetPd(nullptr, 0);
                message_.Send(port_index_);

                quick_find_.counter = rdm::discovery::kQuikfindCounter;
                quick_find_.micros = hal::Micros();
                quick_find_.is_command_running = true;
                return;
            }

            response_ = const_cast<uint8_t*>(message_.Receive(port_index_));

            if ((response_ != nullptr))
            {
                const auto* response = reinterpret_cast<struct TRdmMessage*>(response_);

                if ((response->command_class != E120_DISCOVERY_COMMAND_RESPONSE) ||
                    ((static_cast<uint16_t>((response->param_id[0] << 8) + response->param_id[1])) != E120_DISC_MUTE))
                {
                    puts("QUICKFIND invalid response");
                    // assert(0);
                    return;
                }

                if ((response->command_class == E120_DISCOVERY_COMMAND_RESPONSE) && (memcmp(quick_find_.uid, response->source_uid, RDM_UID_SIZE) == 0))
                {
                    tod_->AddUid(quick_find_.uid);
#ifndef NDEBUG
                    printf("AddUid : ");
                    rdm::discovery::PrintUid(quick_find_.uid);
                    puts("");
#endif
                }

                quick_find_.counter = rdm::discovery::kQuikfindCounter;
                quick_find_.is_command_running = false;
                NEW_STATE(rdm::discovery::State::kQuickfindDiscovery, false);
                return;
            }

            if ((hal::Micros() - quick_find_.micros) > rdm::discovery::kReceiveTimeOut)
            {
                assert(quick_find_.counter > 0);
                quick_find_.counter--;
                quick_find_.is_command_running = false;
            }

            return;
            break;
        case rdm::discovery::State::kQuickfindDiscovery: ///< QUICKFIND_DISCOVERY
            if (quick_find_discovery_.counter == 0)
            {
                quick_find_discovery_.counter = rdm::discovery::kQuikfindDiscoveryCounter;
                quick_find_discovery_.is_command_running = false;
                NEW_STATE(rdm::discovery::State::kDiscovery, true);
                return;
            }

            if (!quick_find_discovery_.is_command_running)
            {
                message_.SetDstUid(UID_ALL);
                message_.SetCc(E120_DISCOVERY_COMMAND);
                message_.SetPid(E120_DISC_UNIQUE_BRANCH);
                message_.SetPd(reinterpret_cast<const uint8_t*>(discovery_.pdl), 2 * RDM_UID_SIZE);
                message_.Send(port_index_);

                quick_find_discovery_.micros = hal::Micros();
                quick_find_discovery_.is_command_running = true;
                return;
            }

            response_ = const_cast<uint8_t*>(message_.Receive(port_index_));

            if ((response_ != nullptr) && (IsValidDiscoveryResponse(quick_find_.uid)))
            {
                quick_find_discovery_.counter = rdm::discovery::kQuikfindDiscoveryCounter;
                quick_find_discovery_.is_command_running = false;
                NEW_STATE(rdm::discovery::State::kQuickfind, true);
                return;
            }

            if ((response_ != nullptr) && (!IsValidDiscoveryResponse(quick_find_.uid)))
            {
                quick_find_discovery_.counter = rdm::discovery::kQuikfindDiscoveryCounter;
                quick_find_discovery_.is_command_running = false;
                NEW_STATE(rdm::discovery::State::kDub, false);
                return;
            }

            if ((hal::Micros() - quick_find_discovery_.micros) > rdm::discovery::kReceiveTimeOut)
            {
                assert(quick_find_.counter > 0);
                quick_find_discovery_.counter--;
                quick_find_discovery_.is_command_running = false;
            }

            return;
            break;
        case rdm::discovery::State::kFinished: ///< FINISHED
            is_finished_ = true;
            NEW_STATE(rdm::discovery::State::kIdle, false);
#ifndef NDEBUG
            tod_->Dump();
#endif
            break;
        default:
            assert(0);
            __builtin_unreachable();
            break;
    }
}
} // namespace rdm::discovery
