/**
 * @file ltcgenerator.h
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

#ifndef ARM_LTCGENERATOR_H_
#define ARM_LTCGENERATOR_H_

#include <cstdint>

#include "ltc.h"
#include "hal_statusled.h"

namespace ltcgenerator {
enum class Direction { kDirectionForward, kDirectionBackward };
enum class Pitch { kPitchNormal, kPitchFaster, kPitchSlower };
} // namespace ltcgenerator

class LtcGenerator {
   public:
    LtcGenerator(const struct ltc::TimeCode* start_timecode, const struct ltc::TimeCode* stop_timecode, bool skip_free = false, bool ignore_start = false, bool ignore_stop = false);

    ~LtcGenerator() { Stop(); }

    void Start();
    void Stop();

    void Run() {
        Update();
        HandleButtons();

        if (state_ == kStarted) {
            hal::statusled::SetMode(hal::statusled::Mode::kData);
        } else {
            hal::statusled::SetMode(hal::statusled::Mode::kNormal);
        }
    }

    void Print();

    void HandleRequest(char* buffer = nullptr, uint32_t nBufferLength = 0);

    void ActionStart(bool reset = true);
    void ActionStop();
    void ActionResume();
    void ActionReset();
    void ActionSetStart(const char* timecode);
    void ActionSetStop(const char* timecode);
    void ActionSetRate(const char* rate);
    void ActionGoto(const char* timecode);
    void ActionSetDirection(const char* direction);
    void ActionSetPitch(float pitch);
    void ActionForward(uint32_t seconds);
    void ActionBackward(uint32_t seconds);

    void Input(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port);

    static LtcGenerator* Get() { return s_this; }

   private:
    void HandleButtons();
    void Update();
    void Increment();
    void Decrement();
    bool PitchControl();
    void SetPitch(const char* pitch, uint32_t size);
    void SetSkip(const char* seconds, uint32_t size, ltcgenerator::Direction direction);
    void SetTimeCode(uint32_t seconds);

    uint32_t GetSeconds(const struct ltc::TimeCode& timecode) {
        uint32_t seconds = timecode.hours;
        seconds *= 60U;
        seconds += timecode.minutes;
        seconds *= 60U;
        seconds += timecode.seconds;

        return seconds;
    }

    void static StaticCallbackFunction(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port) { 
		s_this->Input(buffer, size, from_ip, from_port); 
	}

   private:
    ltc::TimeCode* m_pStartLtcTimeCode;
    ltc::TimeCode* m_pStopLtcTimeCode;
    bool skip_free_;
    bool m_bIgnoreStart;
    bool m_bIgnoreStop;
    bool m_bDropFrame;
    uint32_t m_nStartSeconds;
    uint32_t m_nStopSeconds;

    uint32_t pitch_ticker_{1};
    uint32_t m_nPitchPrevious{0};
    float m_fPitchControl{0};
    uint32_t buttons_{0};
    int32_t handle_{-1};
    uint32_t bytes_received_{0};

    char* udp_buffer_{nullptr};

    uint8_t fps_{0};

    ltcgenerator::Direction m_tDirection{ltcgenerator::Direction::kDirectionForward};
    ltcgenerator::Pitch m_tPitch{ltcgenerator::Pitch::kPitchFaster};

    enum { kStopped, kStarted, kLimit } state_{kStopped};

    static inline LtcGenerator* s_this;
};

#endif /* ARM_LTCGENERATOR_H_ */
