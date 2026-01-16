/**
 * @file l6470dmxmode4.cpp
 *
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "configstore.h"
#include "common/utils/utils_flags.h"
#include "l6470dmxmode4.h"
#include "common/utils/utils_enum.h"
#include "l6470.h"
 #include "firmware/debug/debug_debug.h"

L6470DmxMode4::L6470DmxMode4(L6470* l6470, uint32_t motor_index) : l6470_(l6470), motor_index_(motor_index)
{
    DEBUG_ENTRY();;

    assert(l6470_ != nullptr);

    const auto kMaxSteps = ConfigStore::Instance().DmxL6470GetModeIndexed(motor_index_, &common::store::l6470dmx::Mode::max_steps);

    steps_ = static_cast<float>(kMaxSteps) / 0xFF;

    DEBUG_EXIT();;
}

L6470DmxMode4::~L6470DmxMode4()
{
    DEBUG_ENTRY();;

    DEBUG_EXIT();;
}

void L6470DmxMode4::InitSwitch()
{
    DEBUG_ENTRY();;

    const auto kFlags = ConfigStore::Instance().DmxL6470GetModeIndexed(motor_index_, &common::store::l6470dmx::Mode::flags);

    if (common::IsFlagSet(kFlags, common::store::l6470dmx::mode::Flags::Flag::kUseSwitch))
    {
        const auto kAction =
            common::FromValue<TL6470Action>(ConfigStore::Instance().DmxL6470GetModeIndexed(motor_index_, &common::store::l6470dmx::Mode::switch_action));
        const auto kDir =
            common::FromValue<TL6470Direction>(ConfigStore::Instance().DmxL6470GetModeIndexed(motor_index_, &common::store::l6470dmx::Mode::switch_dir));
        const auto kStepsPerSec = ConfigStore::Instance().DmxL6470GetModeIndexed(motor_index_, &common::store::l6470dmx::Mode::switch_steps_per_sec);

        l6470_->goUntil(kAction, kDir, kStepsPerSec);
    }

    DEBUG_EXIT();;
}

void L6470DmxMode4::InitPos()
{
    DEBUG_ENTRY();;

    l6470_->resetPos();

    DEBUG_EXIT();;
}

void L6470DmxMode4::Start()
{
    DEBUG_ENTRY();;

    DEBUG_EXIT();;
}

void L6470DmxMode4::Stop()
{
    DEBUG_ENTRY();;

    DEBUG_EXIT();;
}

void L6470DmxMode4::HandleBusy()
{
    DEBUG_ENTRY();

    if (l6470_->busyCheck())
    {
#ifndef NDEBUG
        printf("\t\t\tBusy!\n");
#endif
        l6470_->softStop();
        was_busy_ = true;
    }
    else
    {
        was_busy_ = false;
    }

    DEBUG_EXIT();
}

bool L6470DmxMode4::BusyCheck()
{
    DEBUG_ENTRY();;

    DEBUG_EXIT();;
    return l6470_->busyCheck();
}

void L6470DmxMode4::Data(const uint8_t* data)
{
    DEBUG_ENTRY();;

    const auto kSteps = static_cast<int32_t>(static_cast<float>(data[0]) * steps_);
    bool is_rev;
#ifndef NDEBUG
    int32_t difference;
#endif

    if (was_busy_)
    {
        auto current_position = l6470_->getPos();
        is_rev = current_position > kSteps;
#ifndef NDEBUG
        printf("\t\t\tCurrent position=%d\n", current_position);
        difference = kSteps - current_position;
#endif
    }
    else
    {
        is_rev = previous_data_ > data[0];
#ifndef NDEBUG
        difference = data[0] - previous_data_;
#endif
    }

#ifndef NDEBUG
    printf("\t\t\tm_fSteps=%f, steps=%d, data[0]=%d, nDifference=%d [%s]\n", steps_, static_cast<int>(kSteps), data[0], difference,
           is_rev ? "L6470_DIR_REV" : "L6470_DIR_FWD");
#endif

    l6470_->goToDir(is_rev ? L6470_DIR_REV : L6470_DIR_FWD, kSteps);

    previous_data_ = data[0];

    DEBUG_EXIT();;
}
