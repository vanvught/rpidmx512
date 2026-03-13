/**
 * @file mcpbuttons.cpp
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

#include "mcpbuttons.h"
#include "displayset.h"
#include "hal.h"
#include "mcpbuttons.h"
#include "display.h"
#include "ltcdisplaymax7219.h"
#include "rotaryencoder.h"
#include "hal_i2c.h"
#include "hal_gpio.h"
#include "hal_millis.h"
#include "mcp23x17.h"
#include "displayedittimecode.h"
#include "displayeditfps.h"
#include "input.h"
#include "ltcsourceconst.h"
#include "ltcsource.h"
#include "ltcstore.h"
#include "arm/ltcgenerator.h"
#include "arm/systimereader.h"
#if !(defined(CONFIG_LTC_DISABLE_RGB_PANEL) && defined(CONFIG_LTC_DISABLE_WS28XX))
#include "ltcdisplayrgb.h"
#else
#define LTC_NO_DISPLAY_RGB
#endif
#include "firmware/debug/debug_debug.h"

namespace mcp23017
{
static constexpr auto kI2CAddress = 0x20;
}

namespace gpio
{
static constexpr auto kInta = GPIO_EXT_12; // PA7
}

namespace button
{
static constexpr auto kSelect = 2;
static constexpr auto kLeft = 3;
static constexpr auto kRight = 4;
static constexpr auto kStart = 5;
static constexpr auto kStop = 6;
static constexpr auto kResume = 7;
} // namespace button

#define BUTTON(x) ((nButtonsChanged >> x) & 0x01)
#define BUTTON_STATE(x) ((nButtonsChanged & (1U << x)) == (1U << x))

McpButtons::McpButtons(ltc::Source source, bool use_alt_function, int32_t skip_seconds, bool rotary_half_step)
    : hal_i2c_(mcp23017::kI2CAddress), source_(source), use_alt_function_(use_alt_function), skip_seconds_(skip_seconds), rotary_encoder_(rotary_half_step)
{
    ltc::init_timecode(timecode_);
}

bool McpButtons::Check()
{
    DEBUG_ENTRY();

    is_connected_ = hal_i2c_.IsConnected();

    if (!is_connected_)
    {
        DEBUG_EXIT();
        return false;
    }

    // Rotary and buttons
    hal_i2c_.WriteRegister(mcp23x17::REG_IODIRA, static_cast<uint8_t>(0xFF)); // All input
    hal_i2c_.WriteRegister(mcp23x17::REG_GPPUA, static_cast<uint8_t>(0xFF));  // Pull-up
    hal_i2c_.WriteRegister(mcp23x17::REG_IPOLA, static_cast<uint8_t>(0xFF));  // Invert read
    hal_i2c_.WriteRegister(mcp23x17::REG_INTCONA, static_cast<uint8_t>(0x00));
    hal_i2c_.WriteRegister(mcp23x17::REG_GPINTENA, static_cast<uint8_t>(0xFF)); // Interrupt on Change
    hal_i2c_.ReadRegister(mcp23x17::REG_INTCAPA);                               // Clear interrupts
    // Led's
    hal_i2c_.WriteRegister(mcp23x17::REG_IODIRB, static_cast<uint8_t>(0x00)); // All output
    hal_i2c_.WriteRegister(mcp23x17::REG_GPIOB, static_cast<uint8_t>(1U << static_cast<uint8_t>(source_)));

    UpdateDisplays(source_);

    FUNC_PREFIX(GpioFsel(gpio::kInta, GPIO_FSEL_INPUT));
    FUNC_PREFIX(GpioSetPud(gpio::kInta, GPIO_PULL_UP));

    DEBUG_EXIT();
    return true;
}

void McpButtons::HandleActionSelect(const ltc::Source& source)
{
    if (source_ != source)
    {
        source_ = source;
        ltc_store::SaveSource(static_cast<uint8_t>(source_));
    }

    hal_i2c_.WriteRegister(mcp23x17::REG_GPIOB, static_cast<uint8_t>(1U << static_cast<uint8_t>(source)));
    hal_i2c_.ReadRegister(mcp23x17::REG_INTCAPA); // Clear interrupts

    Display::Get()->SetCursor(display::cursor::kOff);
    Display::Get()->SetCursorPos(0, 0);
    Display::Get()->ClearLine(1);
    Display::Get()->ClearLine(2);
}

bool McpButtons::Wait(ltc::Source& source, struct ltc::TimeCode& start_timecode, struct ltc::TimeCode& stop_timecode)
{
    const auto kSource = static_cast<uint32_t>(source);

    if (__builtin_expect((LedBlink(static_cast<uint8_t>(1U << kSource)) >= led_ticker_max_), 0))
    {
        hal_i2c_.WriteRegister(mcp23x17::REG_GPIOB, static_cast<uint8_t>(1U << kSource));
        return false;
    }

    if (__builtin_expect(FUNC_PREFIX(GpioLev(gpio::kInta)) == 0, 0))
    {
        led_ticker_max_ = UINT32_MAX;

        const auto kPortA = hal_i2c_.ReadRegister(mcp23x17::REG_GPIOA);
        const uint8_t nButtonsChanged = (kPortA ^ port_a_previous_) & kPortA;

        port_a_previous_ = kPortA;

        printf("%.2x %.2x\r", kPortA, nButtonsChanged);

        /* P = m_nPortAPrevious
         * N = m_nPortA
         * X = m_nPortA ^ m_nPortAPrevious
         * C = nButtonsChanged
         *
         * P N	X N	C
         * 0 0	0 0	0
         * 0 1	1 1	1
         * 1 0	1 0	0
         * 1 1	0 1	0
         */

        key_ = input::KEY_NOT_DEFINED;

        if (nButtonsChanged != 0)
        {
            if (BUTTON_STATE(button::kLeft))
            { // LEFT
                HandleActionLeft(source);
            }
            else if (BUTTON_STATE(button::kRight))
            { // RIGHT
                HandleActionRight(source);
            }
            else if (BUTTON_STATE(button::kSelect))
            { // SELECT
                HandleActionSelect(source);
                return false;
            }
            else if (BUTTON_STATE(button::kStart))
            { // START
                if (source == ltc::Source::INTERNAL)
                {
                    if (state_ != kEditTimecodeStart)
                    {
                        Display::Get()->SetCursorPos(static_cast<uint8_t>(Display::Get()->GetColumns() - 7U), 0);
                        Display::Get()->PutString("[Start]");
                    }

                    key_ = input::KEY_ENTER;

                    switch (state_)
                    {
                        case kSourceSelect:
                            state_ = kEditTimecodeStart;
                            break;
                        case kEditTimecodeStart:
                            state_ = kEditFps;
                            break;
                        case kEditTimecodeStop:
                        case kEditFps:
                            state_ = kEditTimecodeStart;
                            break;
                        default:
                            break;
                    }
                }
            }
            else if (BUTTON_STATE(button::kStop))
            { // STOP
                if (source == ltc::Source::INTERNAL)
                {
                    if (state_ != kEditTimecodeStop)
                    {
                        Display::Get()->SetCursorPos(static_cast<uint8_t>(Display::Get()->GetColumns() - 7U), 0);
                        Display::Get()->PutString("[Stop] ");
                    }
                    key_ = input::KEY_ENTER;
                    state_ = kEditTimecodeStop;
                }
            }
            else if (BUTTON_STATE(button::kResume))
            { // RESUME
                if (source == ltc::Source::INTERNAL)
                {
                    key_ = input::KEY_ESC;
                }
            }
        }

        HandleRotary(kPortA, source);

        if (key_ != input::KEY_NOT_DEFINED)
        {
            switch (state_)
            {
                case kEditTimecodeStart:
                    HandleInternalTimeCodeStart(start_timecode);
                    break;
                case kEditTimecodeStop:
                    HandleInternalTimeCodeStop(stop_timecode);
                    break;
                case kEditFps:
                    HandleInternalTimeCodeFps(start_timecode);
                    break;
                default:
                    break;
            }
        }

        port_b_ = 0;
    }

    return true;
}

void McpButtons::Run()
{
    if (__builtin_expect(!is_connected_, 0))
    {
        return;
    }

    if (__builtin_expect(FUNC_PREFIX(GpioLev(gpio::kInta)) == 0, 0))
    {
        const auto nPortA = hal_i2c_.ReadRegister(mcp23x17::REG_GPIOA);
        const uint8_t nButtonsChanged = (nPortA ^ port_a_previous_) & nPortA;

        port_a_previous_ = nPortA;

        DEBUG_PRINTF("\n\n%.2x %.2x", nPortA, nButtonsChanged);

        if (source_ == ltc::Source::INTERNAL)
        {
            if (BUTTON_STATE(button::kStart))
            {
                LtcGenerator::Get()->ActionStart(!use_alt_function_);
                return;
            }

            if (BUTTON_STATE(button::kStop))
            {
                LtcGenerator::Get()->ActionStop();
                return;
            }

            if (BUTTON_STATE(button::kResume))
            {
                if (!use_alt_function_)
                {
                    LtcGenerator::Get()->ActionResume();
                    return;
                }

                if (run_status_ == RunStatus::kIdle)
                {
                    SetRunState(RunStatus::kTcReset);
                    return;
                }

                if (run_status_ == RunStatus::kTcReset)
                {
                    SetRunState(RunStatus::kIdle);
                    return;
                }

                return;
            }
        }
        else if (source_ == ltc::Source::SYSTIME)
        {
            if (BUTTON_STATE(button::kStart))
            {
                SystimeReader::Get()->ActionStart();
                return;
            }
            if (BUTTON_STATE(button::kStop))
            {
                SystimeReader::Get()->ActionStop();
                return;
            }
        }

        if (BUTTON_STATE(button::kSelect))
        {
            HandleRunActionSelect();
            return;
        }

        if (BUTTON_STATE(button::kLeft))
        {
            if (run_status_ == RunStatus::kReboot)
            {
                SetRunState(RunStatus::kContinue);
                return;
            }

            if (run_status_ == RunStatus::kIdle)
            {
                LtcGenerator::Get()->ActionBackward(skip_seconds_);
                return;
            }

            return;
        }

        if (BUTTON_STATE(button::kRight))
        {
            if (run_status_ == RunStatus::kContinue)
            {
                SetRunState(RunStatus::kReboot);
                return;
            }

            if (run_status_ == RunStatus::kIdle)
            {
                LtcGenerator::Get()->ActionForward(skip_seconds_);
                return;
            }

            return;
        }

        rotary_direction_ = rotary_encoder_.Process(nPortA);

        DEBUG_PRINTF("%d %s", rotary_direction_, rotary_direction_ == RotaryEncoder::CCW ? "CCW" : (rotary_direction_ == RotaryEncoder::CW ? "CW" : "!!"));

        if (rotary_direction_ == RotaryEncoder::CCW)
        {
            if (run_status_ == RunStatus::kReboot)
            {
                SetRunState(RunStatus::kContinue);
                return;
            }

            if (run_status_ == RunStatus::kIdle)
            {
                LtcGenerator::Get()->ActionBackward(skip_seconds_);
                return;
            }

            return;
        }

        if (rotary_direction_ == RotaryEncoder::CW)
        {
            if (run_status_ == RunStatus::kContinue)
            {
                SetRunState(RunStatus::kReboot);
                return;
            }

            if (run_status_ == RunStatus::kIdle)
            {
                LtcGenerator::Get()->ActionForward(skip_seconds_);
                return;
            }

            return;
        }
    }
}


uint32_t McpButtons::LedBlink(uint8_t port)
{
    const auto kMillisNow = hal::Millis();

    if (__builtin_expect(((kMillisNow - millis_previous_) < 500), 1))
    {
        return led_ticker_;
    }

    millis_previous_ = kMillisNow;
    port_b_ ^= port;
    hal_i2c_.WriteRegister(mcp23x17::REG_GPIOB, port_b_);

    return ++led_ticker_;
}

void McpButtons::HandleActionLeft(ltc::Source& source)
{
    if (state_ == kSourceSelect)
    {
        if (source == static_cast<ltc::Source>(0))
        {
            source = static_cast<ltc::Source>(static_cast<uint32_t>(ltc::Source::UNDEFINED) - 1);
        }
        else
        {
            source = static_cast<ltc::Source>(static_cast<uint32_t>(source) - 1);
        }
        UpdateDisplays(source);
        return;
    }

    key_ = input::KEY_LEFT;
}

void McpButtons::HandleActionRight(ltc::Source& source)
{
    if (state_ == kSourceSelect)
    {
        if (source == static_cast<ltc::Source>(static_cast<uint32_t>(ltc::Source::UNDEFINED) - 1))
        {
            source = static_cast<ltc::Source>(0);
        }
        else
        {
            source = static_cast<ltc::Source>(static_cast<uint32_t>(source) + 1);
        }
        UpdateDisplays(source);
        return;
    }

    key_ = input::KEY_RIGHT;
}

void McpButtons::HandleRotary(uint8_t input_ab, ltc::Source& source)
{
    rotary_direction_ = rotary_encoder_.Process(input_ab);

    if (state_ == kSourceSelect)
    {
        if (rotary_direction_ == RotaryEncoder::CW)
        {
            HandleActionRight(source);
        }
        else if (rotary_direction_ == RotaryEncoder::CCW)
        {
            HandleActionLeft(source);
        }
        return;
    }

    if (rotary_direction_ == RotaryEncoder::CW)
    {
        key_ = input::KEY_UP;
    }
    else if (rotary_direction_ == RotaryEncoder::CCW)
    {
        key_ = input::KEY_DOWN;
    }
}

    void McpButtons::UpdateDisplays(ltc::Source source)
    {
        DEBUG_ENTRY();

        const auto nSource = static_cast<uint8_t>(source);
        DEBUG_PRINTF("nSource=%u", nSource);

        assert(Display::Get() != nullptr);

        Display::Get()->TextStatus(LtcSourceConst::NAME[nSource]);

        DEBUG_PUTS("");

        if (ltc::Destination::IsEnabled(ltc::Destination::Output::MAX7219))
        {
            assert(LtcDisplayMax7219::Get() != nullptr);
            LtcDisplayMax7219::Get()->WriteChar(nSource);
        }

        DEBUG_PUTS("");

#if !defined(CONFIG_LTC_DISABLE_WS28XX)
        if (ltc::Destination::IsEnabled(ltc::Destination::Output::WS28XX))
        {
            assert(LtcDisplayRgb::Get() != nullptr);
            LtcDisplayRgb::Get()->WriteChar(nSource);
        }
#endif

        DEBUG_PUTS("");

#if !defined(CONFIG_LTC_DISABLE_RGB_PANEL)
        if (ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL))
        {
            assert(LtcDisplayRgb::Get() != nullptr);
            LtcDisplayRgb::Get()->ShowSource(source);
        }
#endif

        DEBUG_EXIT();
    }

// Running mode

void McpButtons::HandleRunActionSelect()
{
    DEBUG_PRINTF("%d", run_status_);

    if (Display::Get()->IsSleep())
    {
        Display::Get()->SetSleep(false);
    }

    const auto kMillisNow = hal::Millis();

    if ((kMillisNow - select_millis_) < 300)
    {
        return;
    }

    select_millis_ = kMillisNow;

    if (run_status_ == RunStatus::kIdle)
    {
        SetRunState(RunStatus::kContinue);
        return;
    }

    if (run_status_ == RunStatus::kContinue)
    {
        SetRunState(RunStatus::kIdle);
        return;
    }

    if (run_status_ == RunStatus::kReboot)
    {
        hal_i2c_.WriteRegister(mcp23x17::REG_GPIOB, static_cast<uint8_t>(0xFF));

        Display::Get()->SetSleep(false);
        Display::Get()->Cls();
        Display::Get()->TextStatus("Reboot ...");

        hal::Reboot();
        __builtin_unreachable();
        return;
    }

    if (run_status_ == RunStatus::kTcReset)
    {
        SetRunState(RunStatus::kIdle);
        LtcGenerator::Get()->ActionReset();
        return;
    }
}

void McpButtons::SetRunState(RunStatus run_state)
{
    DEBUG_PRINTF("%d %d", run_state, run_status_);

    run_status_ = run_state;

    switch (run_state)
    {
        case RunStatus::kIdle:
            hal_i2c_.WriteRegister(mcp23x17::REG_GPIOB, static_cast<uint8_t>(1U << static_cast<uint32_t>(source_)));
            ltc::source::Show(source_, run_gps_time_client_);
            break;
        case RunStatus::kContinue:
            hal_i2c_.WriteRegister(mcp23x17::REG_GPIOB, static_cast<uint8_t>(0x0F));
            Display::Get()->TextLine(4, ">CONTINUE?< ", 12);
            break;
        case RunStatus::kReboot:
            hal_i2c_.WriteRegister(mcp23x17::REG_GPIOB, static_cast<uint8_t>(0xF0));
            Display::Get()->TextLine(4, ">REBOOT?  < ", 12);
            break;
        case RunStatus::kTcReset:
            hal_i2c_.WriteRegister(mcp23x17::REG_GPIOB, static_cast<uint8_t>(0xAA));
            Display::Get()->TextLine(4, ">RESET TC?< ", 12);
            break;
        default:
            break;
    }
}

void McpButtons::HandleInternalTimeCodeStart(struct ltc::TimeCode& start_timecode)
{
    display_edit_time_code_.HandleKey(key_, start_timecode, timecode_);

    if (ltc::Destination::IsEnabled(ltc::Destination::Output::MAX7219))
    {
        LtcDisplayMax7219::Get()->Show(timecode_);
    }
#if !defined(LTC_NO_DISPLAY_RGB)
    else if (ltc::Destination::IsEnabled(ltc::Destination::Output::WS28XX) || ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL))
    {
        LtcDisplayRgb::Get()->Show(timecode_);
    }
#endif
    HandleInternalKeyEsc();
}

void McpButtons::HandleInternalTimeCodeStop(struct ltc::TimeCode& start_timecode)
{
    display_edit_time_code_.HandleKey(key_, start_timecode, timecode_);

    if (ltc::Destination::IsEnabled(ltc::Destination::Output::MAX7219))
    {
        LtcDisplayMax7219::Get()->Show(timecode_);
    }
#if !defined(LTC_NO_DISPLAY_RGB)
    else if (ltc::Destination::IsEnabled(ltc::Destination::Output::WS28XX) || ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL))
    {
        LtcDisplayRgb::Get()->Show(timecode_);
    }
#endif

    HandleInternalKeyEsc();
}

void McpButtons::HandleInternalTimeCodeFps(struct ltc::TimeCode& start_timecode)
{
    display_edit_fps_.HandleKey(key_, start_timecode.type);

    HandleInternalKeyEsc();
}

void McpButtons::HandleInternalKeyEsc()
{
    if (key_ == input::KEY_ESC)
    {
        Display::Get()->SetCursor(display::cursor::kOff);
        Display::Get()->SetCursorPos(0, 0);
        Display::Get()->ClearLine(1);
        Display::Get()->ClearLine(2);
        state_ = kSourceSelect;
    }
}