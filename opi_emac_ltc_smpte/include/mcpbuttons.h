/**
 * @file mcpbuttons.h
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

#ifndef MCPBUTTONS_H_
#define MCPBUTTONS_H_

#include <stdint.h>

#include "rotaryencoder.h"
#include "display.h"
#include "displayedittimecode.h"
#include "displayeditfps.h"
#include "ltcdisplaymax7219.h"
#include "ltcdisplayrgb.h"
#include "input.h"
#include "hal_i2c.h"

enum class RunStatus
{
    IDLE,
    CONTINUE,
    REBOOT,
    TC_RESET
};

class McpButtons
{
   public:
    McpButtons(ltc::Source reader_source, bool use_alt_function, int32_t skip_seconds, bool rotary_half_step);

    bool Check();
    bool Wait(ltc::Source& tLtcReaderSource, struct ltc::TimeCode& StartTimeCode, struct ltc::TimeCode& StopTimeCode);

    bool IsConnected() const { return is_connected_; }

    void SetRunGpsTimeClient(bool run_gps_timeclient) { m_bRunGpsTimeClient = run_gps_timeclient; }

    bool GetRotaryHalfstep() const { return rotary_encoder_.GetHalfstep(); }

    void Run();

   private:
    uint32_t LedBlink(uint8_t portb);
    void HandleActionLeft(ltc::Source& ltc_source);
    void HandleActionRight(ltc::Source& ltc_source);
    void HandleActionSelect(const ltc::Source& ltc_source);
    void HandleRotary(uint8_t nInputAB, ltc::Source& ltc_source);
    void UpdateDisplays(ltc::Source ltc_source);
    // Running mode
    void HandleRunActionSelect();
    void SetRunState(RunStatus run_state);
    // Internal
    void HandleInternalTimeCodeStart(struct ltc::TimeCode& start_timecode)
    {
        displayEditTimeCode.HandleKey(key_, start_timecode, m_aTimeCode);

        //	if (!ltc::g_DisabledOutputs.bMax7219) {
        if (ltc::Destination::IsEnabled(ltc::Destination::Output::MAX7219))
        {
            LtcDisplayMax7219::Get()->Show(m_aTimeCode);
        }
        else if (ltc::Destination::IsEnabled(ltc::Destination::Output::WS28XX) || ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL))
        {
            LtcDisplayRgb::Get()->Show(m_aTimeCode);
        }

        HandleInternalKeyEsc();
    }

    void HandleInternalTimeCodeStop(struct ltc::TimeCode& start_timecode)
    {
        displayEditTimeCode.HandleKey(key_, start_timecode, m_aTimeCode);

        //	if (!ltc::g_DisabledOutputs.bMax7219) {
        if (ltc::Destination::IsEnabled(ltc::Destination::Output::MAX7219))
        {
            LtcDisplayMax7219::Get()->Show(m_aTimeCode);
        }
        else if (ltc::Destination::IsEnabled(ltc::Destination::Output::WS28XX) || ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL))
        {
            LtcDisplayRgb::Get()->Show(m_aTimeCode);
        }

        HandleInternalKeyEsc();
    }

    void HandleInternalTimeCodeFps(struct ltc::TimeCode& StartTimeCode)
    {
        displayEditFps.HandleKey(key_, StartTimeCode.type);

        HandleInternalKeyEsc();
    }

    void HandleInternalKeyEsc()
    {
        if (key_ == input::KEY_ESC)
        {
            Display::Get()->SetCursor(display::cursor::kOff);
            Display::Get()->SetCursorPos(0, 0);
            Display::Get()->ClearLine(1);
            Display::Get()->ClearLine(2);
            state_ = SOURCE_SELECT;
        }
    }

   private:
    HAL_I2C hal_i2c_;
    DisplayEditTimeCode displayEditTimeCode;
    DisplayEditFps displayEditFps;
    enum
    {
        SOURCE_SELECT,
        EDIT_TIMECODE_START,
        EDIT_TIMECODE_STOP,
        EDIT_FPS
    } state_{SOURCE_SELECT};
    ltc::Source m_tLtcReaderSource;
    bool m_bUseAltFunction;
    int32_t m_nSkipSeconds;
    bool is_connected_{false};
    uint8_t m_nPortAPrevious{0};
    uint8_t port_b_{0};
    uint32_t m_nMillisPrevious{0};
    uint32_t m_nLedTicker{0};
    uint32_t m_nLedTickerMax{20}; // 10 seconds
    RotaryEncoder rotary_encoder_;
    uint8_t m_tRotaryDirection{RotaryEncoder::NONE};
    RunStatus m_tRunStatus{RunStatus::IDLE};
    uint32_t m_nSelectMillis{0};
    int key_{input::KEY_NOT_DEFINED};
    char m_aTimeCode[ltc::timecode::CODE_MAX_LENGTH];
    bool m_bRunGpsTimeClient{false};
};

#endif  // MCPBUTTONS_H_
