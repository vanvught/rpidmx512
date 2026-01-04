/**
 * @file tcnetreader.h
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

#ifndef ARM_TCNETREADER_H_
#define ARM_TCNETREADER_H_

#include <cassert>

#include "tcnettimecode.h"
#include "ltc.h"
#include "ltcoutputs.h"

class TCNetReader
{
   public:
    TCNetReader()
    {
        assert(s_this == nullptr);
        s_this = this;
    }

    ~TCNetReader() = default;

    void Start();
    void Stop();

    void Run();

    void Input(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port);

    void static StaticCallbackFunctionHandler(const struct tcnet::TimeCode* timecode)
    {
        assert(s_this != nullptr);
        s_this->Handler(timecode);
    }

   private:
    void static StaticCallbackFunctionInput(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        s_this->Input(buffer, size, from_ip, from_port);
    }

    void Handler(const struct tcnet::TimeCode* timecode);

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

    void ResetTimer(bool do_reset, const struct tcnet::TimeCode* timecode);

   private:
    int32_t handle_{-1};

    tcnet::TimeCode timecode_;

    uint8_t type_previous_{UINT8_MAX};
    uint8_t frame_previous_{UINT8_MAX};

    bool reset_timecode_{true};
    bool do_reset_timer_{true};

    static inline TCNetReader* s_this;
};

#endif  // ARM_TCNETREADER_H_
