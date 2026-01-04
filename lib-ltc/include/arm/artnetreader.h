/**
 * @file artnetreader.h
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

#ifndef ARM_ARTNETREADER_H_
#define ARM_ARTNETREADER_H_

#include <cstdint>
#include <cassert>

#include "artnettimecode.h"
#include "ltcoutputs.h"
#include "hal_millis.h"
#include "hal_statusled.h"

class ArtNetReader
{
   public:
    ArtNetReader()
    {
        assert(s_this == nullptr);
        s_this = this;
    }

    ~ArtNetReader() = default;

    void Start();
    void Stop();

    void Run()
    {
        const auto kTimeStamp = hal::Millis();

        if ((kTimeStamp - timestamp_) >= 50U)
        {
            LtcOutputs::Get()->ShowSysTime();
            hal::statusled::SetMode(hal::statusled::Mode::NORMAL);
            Reset(true);
        }
        else
        {
            hal::statusled::SetMode(hal::statusled::Mode::DATA);
            Reset(false);
        }
    }

    void static StaticCallbackFunction(const struct artnet::TimeCode* timecode)
    {
        assert(s_this != nullptr);
        s_this->Handler(timecode);
    }

   private:
    void Handler(const struct artnet::TimeCode*);

    void Reset(bool do_reset)
    {
        if (reset_timecode_ != do_reset)
        {
            reset_timecode_ = do_reset;
            if (reset_timecode_)
            {
                LtcOutputs::Get()->ResetTimeCodeTypePrevious();
            }
        }
    }

   private:
    uint32_t timestamp_{0};
    bool reset_timecode_{true};
    static inline ArtNetReader* s_this;
};

#endif  // ARM_ARTNETREADER_H_
