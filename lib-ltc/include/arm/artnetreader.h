/**
 * @file artnetreader.h
 */
/* Copyright (C) 2019-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "artnet.h"
#include "artnettimecode.h"
#include "ltcoutputs.h"
#include "hal_statusled.h"
#include "hal_millis.h" // IWYU pragma: keep

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
            hal::statusled::SetMode(hal::statusled::Mode::kNormal);
            Reset(true);
        }
        else
        {
            hal::statusled::SetMode(hal::statusled::Mode::kData);
            Reset(false);
        }
    }

    void static StaticCallbackFunction(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        s_this->InputUdp(buffer, size, from_ip, from_port);
    }

   private:
    void Handler(const struct artnet::TimeCode*);

    void InputUdp(const uint8_t* buffer, uint32_t size, [[maybe_unused]] uint32_t from_ip, [[maybe_unused]] uint16_t from_port)
    {
        if (size != sizeof(artnet::ArtTimeCode)) [[unlikely]]
        {
            return;
        }

        if (__builtin_expect(((buffer[10] != 0) || (buffer[11] != artnet::kProtocolRevision)), 0)) [[unlikely]]
        {
            return;
        }

        const auto kOpCode = static_cast<artnet::OpCodes>((static_cast<uint16_t>(buffer[9] << 8)) + buffer[8]);

        if (kOpCode == artnet::OpCodes::kOpTimecode) [[likely]]
        {
            {
                const auto* const kArtTimeCode = reinterpret_cast<const artnet::ArtTimeCode*>(buffer);
                Handler(reinterpret_cast<const struct artnet::TimeCode*>(&kArtTimeCode->frames));
            }
        }
    }

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
    int32_t handle_{-1};
    uint32_t timestamp_{0};
    bool reset_timecode_{true};
    static inline ArtNetReader* s_this;
};

#endif // ARM_ARTNETREADER_H_
