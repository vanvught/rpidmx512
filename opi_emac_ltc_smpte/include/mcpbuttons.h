/**
 * @file mcpbuttons.h
 *
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

#ifndef MCPBUTTONS_H_
#define MCPBUTTONS_H_

#include <stdint.h>

#include "hal_i2c.h"
#include "displayedittimecode.h"
#include "displayeditfps.h"
#include "rotaryencoder.h"

enum class RunStatus
{
    kIdle,
    kContinue,
    kReboot,
    kTcReset
};

class McpButtons
{
   public:
    McpButtons(ltc::Source source, bool use_alt_function, uint32_t skip_seconds, bool rotary_half_step);

    bool Check();
    bool Wait(ltc::Source& source, struct ltc::TimeCode& start_timecode, struct ltc::TimeCode& stop_timecode);
    bool IsConnected() const { return is_connected_; }
    void SetRunGpsTimeClient(bool run_gps_timeclient) { run_gps_time_client_ = run_gps_timeclient; }
    bool GetRotaryHalfstep() const { return rotary_encoder_.GetHalfstep(); }
    void Run();

   private:
    uint32_t LedBlink(uint8_t port);
    void HandleActionLeft(ltc::Source& source);
    void HandleActionRight(ltc::Source& source);
    void HandleActionSelect(const ltc::Source& source);
    void HandleRotary(uint8_t input_ab, ltc::Source& source);
    void UpdateDisplays(ltc::Source source);

    // Running mode
    void HandleRunActionSelect();
    void SetRunState(RunStatus run_state);

    // Internal
    void HandleInternalTimeCodeStart(struct ltc::TimeCode& start_timecode);
    void HandleInternalTimeCodeStop(struct ltc::TimeCode& start_timecode);
    void HandleInternalTimeCodeFps(struct ltc::TimeCode& start_timecode);
    void HandleInternalKeyEsc();

   private:
    HAL_I2C hal_i2c_;
    DisplayEditTimeCode display_edit_time_code_;
    DisplayEditFps display_edit_fps_;
    enum
    {
        kSourceSelect,
        kEditTimecodeStart,
        kEditTimecodeStop,
        kEditFps
    } state_{kSourceSelect};
    ltc::Source source_;
    bool use_alt_function_;
    uint32_t skip_seconds_;
    bool is_connected_{false};
    uint8_t port_a_previous_{0};
    uint8_t port_b_{0};
    uint32_t millis_previous_{0};
    uint32_t led_ticker_{0};
    uint32_t led_ticker_max_{20}; // 10 seconds
    RotaryEncoder rotary_encoder_;
    uint8_t rotary_direction_{RotaryEncoder::NONE};
    RunStatus run_status_{RunStatus::kIdle};
    uint32_t select_millis_{0};
    int key_{input::KEY_NOT_DEFINED};
    char timecode_[ltc::timecode::CODE_MAX_LENGTH];
    bool run_gps_time_client_{false};
};

#endif // MCPBUTTONS_H_
