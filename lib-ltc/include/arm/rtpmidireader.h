/**
 * @file rtpmidireader.h
 *
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

#ifndef ARM_RTPMIDIREADER_H_
#define ARM_RTPMIDIREADER_H_

#include <cstdint>

#include "net/rtpmidihandler.h"
#include "ltc.h"
#include "midibpm.h"

class RtpMidiReader final : public RtpMidiHandler
{
   public:
    void Start();
    void Stop();
    void Run();

    void MidiMessage(const struct midi::Message* message) override;

   private:
    void HandleMtc(const struct midi::Message* message);
    void HandleMtcQf(const struct midi::Message* message);
    void Update();

   private:
    struct ltc::TimeCode timecode_;
    uint8_t part_previous_{0};
    bool direction_{true};
    uint32_t mtc_qf_previous_{0};
    uint32_t mtc_qf_delta_{0};
    MidiBPM midi_bpm_;
};

#endif  // ARM_RTPMIDIREADER_H_
