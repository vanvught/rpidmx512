/**
 * @file rdmidentify.h
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDMIDENTIFY_H_
#define RDMIDENTIFY_H_

#include <cstdint>
#include <cassert>

#include "hal_statusled.h"

class RDMIdentify
{
   public:
    enum class Mode : uint8_t
    {
        kQuiet = 0x00,
        kLoud = 0xFF,
    };

    RDMIdentify()
    {
        assert(s_this == nullptr);
        s_this = this;
    }

    ~RDMIdentify() = default;

    void On()
    {
        s_is_enabled = true;
        hal::statusled::SetModeWithLock(hal::statusled::Mode::FAST, true);

        if (s_mode != Mode::kQuiet)
        {
            On(s_mode);
        }
    }

    void Off()
    {
        s_is_enabled = false;
        hal::statusled::SetModeWithLock(hal::statusled::Mode::NORMAL, false);

        if (s_mode != Mode::kQuiet)
        {
            Off(s_mode);
        }
    }

    bool IsEnabled() const { return s_is_enabled; }

    void SetMode(Mode mode)
    {
        if ((mode == Mode::kQuiet) || (mode == Mode::kLoud))
        {
            s_mode = mode;

            if (s_is_enabled)
            {
                if (mode != Mode::kQuiet)
                {
                    On(mode);
                }
                else
                {
                    Off(mode);
                }
            }
            else
            {
                Off(mode);
            }
        }
    }

    Mode GetMode() const { return s_mode; }

    static RDMIdentify* Get() { return s_this; }

   private:
    void On(Mode mode);
    void Off(Mode mode);

   private:
    static inline bool s_is_enabled;
    static inline Mode s_mode;
    static inline RDMIdentify* s_this;
};

#endif  // RDMIDENTIFY_H_
