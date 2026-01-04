/**
 * @file l6470dmxmode0.cpp
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

#include <cstdint>
#include <cassert>

#include "l6470dmxmode0.h"
#include "l6470.h"
 #include "firmware/debug/debug_debug.h"

L6470DmxMode0::L6470DmxMode0(L6470* l6470)
{
    DEBUG_ENTRY();;

    assert(l6470 != nullptr);

    l6470_ = l6470;

    min_speed_ = l6470_->getMinSpeed();
    max_speed_ = l6470_->getMaxSpeed();

    DEBUG_EXIT();;
}

L6470DmxMode0::~L6470DmxMode0()
{
    DEBUG_ENTRY();;

    DEBUG_EXIT();;
}

void L6470DmxMode0::Start()
{
    DEBUG_ENTRY();;

    l6470_->run(L6470_DIR_FWD, max_speed_);

    DEBUG_EXIT();;
}

void L6470DmxMode0::Stop()
{
    DEBUG_ENTRY();;

    l6470_->hardHiZ();

    DEBUG_EXIT();;
}

void L6470DmxMode0::Data(const uint8_t* data)
{
    DEBUG_ENTRY();;

    if (data[0] <= 126)
    { // Left-hand rotation
        l6470_->run(L6470_DIR_FWD, min_speed_ + static_cast<float>((127 - data[0])) * ((max_speed_ - min_speed_) / 127));
        return;
    }

    if (data[0] >= 130)
    { // Right-hand rotation
        l6470_->run(L6470_DIR_REV, min_speed_ + static_cast<float>((data[0] - 129)) * ((max_speed_ - min_speed_) / 127));
        return;
    }

    l6470_->softStop();

    DEBUG_EXIT();;
}
