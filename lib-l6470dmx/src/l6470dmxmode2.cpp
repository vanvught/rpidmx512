/**
 * @file l6470dmxmode2.cpp
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
#include <cassert>

#include "l6470dmxmode2.h"
#include "l6470.h"

 #include "firmware/debug/debug_debug.h"

L6470DmxMode2::L6470DmxMode2(L6470* l6470) : l6470_(l6470)
{
    DEBUG_ENTRY();;

    assert(l6470_ != nullptr);

    min_speed_ = l6470_->getMinSpeed();
    max_speed_ = l6470_->getMaxSpeed();

    DEBUG_EXIT();;
}

L6470DmxMode2::~L6470DmxMode2()
{
    DEBUG_ENTRY();;

    DEBUG_EXIT();;
}

void L6470DmxMode2::Start()
{
    DEBUG_ENTRY();;

    l6470_->run(L6470_DIR_FWD, min_speed_);

    DEBUG_EXIT();;
}

void L6470DmxMode2::Stop()
{
    DEBUG_ENTRY();;

    l6470_->hardHiZ();

    DEBUG_EXIT();;
}

void L6470DmxMode2::Data(const uint8_t* dmx_data)
{
    DEBUG_ENTRY();;

    l6470_->run(L6470_DIR_FWD, min_speed_ + static_cast<float>(dmx_data[0]) * ((max_speed_ - min_speed_) / 255));

    DEBUG_EXIT();;
}
