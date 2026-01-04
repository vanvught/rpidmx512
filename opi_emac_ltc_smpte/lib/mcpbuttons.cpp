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

#include <cstdint>
#include <cstdio>

#include "displayset.h"
#include "hal.h"
#include "mcpbuttons.h"
#include "ltcdisplayrgb.h"
#include "display.h"
#include "ltcdisplaymax7219.h"
#include "rotaryencoder.h"
#include "hal_i2c.h"
#include "hal_gpio.h"
#include "h3/hal_millis.h"
#include "mcp23x17.h"
#include "displayedittimecode.h"
#include "displayeditfps.h"
#include "input.h"
#include "ltcsourceconst.h"
#include "ltcsource.h"
#include "ltcstore.h"
#include "arm/ltcgenerator.h"
#include "arm/systimereader.h"
#include "configstore.h"
 #include "firmware/debug/debug_debug.h"

namespace mcp23017
{
static constexpr auto I2C_ADDRESS = 0x20;
}

namespace gpio
{
static constexpr auto INTA = GPIO_EXT_12; // PA7
}

namespace button
{
static constexpr auto SELECT = 2;
static constexpr auto LEFT = 3;
static constexpr auto RIGHT = 4;
static constexpr auto START = 5;
static constexpr auto STOP = 6;
static constexpr auto RESUME = 7;
} // namespace button

#define BUTTON(x) ((nButtonsChanged >> x) & 0x01)
#define BUTTON_STATE(x) ((nButtonsChanged & (1U << x)) == (1U << x))

McpButtons::McpButtons(ltc::Source tLtcReaderSource, bool bUseAltFunction, int32_t nSkipSeconds, bool bRotaryHalfStep)
    : hal_i2c_(mcp23017::I2C_ADDRESS), m_tLtcReaderSource(tLtcReaderSource), m_bUseAltFunction(bUseAltFunction), m_nSkipSeconds(nSkipSeconds), rotary_encoder_(bRotaryHalfStep)
{
    ltc::init_timecode(m_aTimeCode);
}

uint32_t McpButtons::LedBlink(uint8_t nPortB)
{
    const auto nMillisNow =hal::Millis();

    if (__builtin_expect(((nMillisNow - m_nMillisPrevious) < 500), 1))
    {
        return m_nLedTicker;
    }

    m_nMillisPrevious = nMillisNow;
    port_b_ ^= nPortB;
    hal_i2c_.WriteRegister(mcp23x17::REG_GPIOB, port_b_);

    return ++m_nLedTicker;
}

void McpButtons::HandleActionLeft(ltc::Source& ltcSource)
{
    if (state_ == SOURCE_SELECT)
    {
        if (ltcSource == static_cast<ltc::Source>(0))
        {
            ltcSource = static_cast<ltc::Source>(static_cast<uint32_t>(ltc::Source::UNDEFINED) - 1);
        }
        else
        {
            ltcSource = static_cast<ltc::Source>(static_cast<uint32_t>(ltcSource) - 1);
        }
        UpdateDisplays(ltcSource);
        return;
    }

    key_ = input::KEY_LEFT;
}

void McpButtons::HandleActionRight(ltc::Source& ltcSource)
{
    if (state_ == SOURCE_SELECT)
    {
        if (ltcSource == static_cast<ltc::Source>(static_cast<uint32_t>(ltc::Source::UNDEFINED) - 1))
        {
            ltcSource = static_cast<ltc::Source>(0);
        }
        else
        {
            ltcSource = static_cast<ltc::Source>(static_cast<uint32_t>(ltcSource) + 1);
        }
        UpdateDisplays(ltcSource);
        return;
    }

    key_ = input::KEY_RIGHT;
}

void McpButtons::HandleActionSelect(const ltc::Source& ltcSource)
{
    if (m_tLtcReaderSource != ltcSource)
    {
        m_tLtcReaderSource = ltcSource;
        ltc_store::SaveSource(static_cast<uint8_t>(m_tLtcReaderSource));
    }

    hal_i2c_.WriteRegister(mcp23x17::REG_GPIOB, static_cast<uint8_t>(1U << static_cast<uint8_t>(ltcSource)));
    hal_i2c_.ReadRegister(mcp23x17::REG_INTCAPA); // Clear interrupts

    Display::Get()->SetCursor(display::cursor::kOff);
    Display::Get()->SetCursorPos(0, 0);
    Display::Get()->ClearLine(1);
    Display::Get()->ClearLine(2);
}

void McpButtons::HandleRotary(uint8_t nInputAB, ltc::Source& tLtcReaderSource)
{
    m_tRotaryDirection = rotary_encoder_.Process(nInputAB);

    if (state_ == SOURCE_SELECT)
    {
        if (m_tRotaryDirection == RotaryEncoder::CW)
        {
            HandleActionRight(tLtcReaderSource);
        }
        else if (m_tRotaryDirection == RotaryEncoder::CCW)
        {
            HandleActionLeft(tLtcReaderSource);
        }
        return;
    }

    if (m_tRotaryDirection == RotaryEncoder::CW)
    {
        key_ = input::KEY_UP;
    }
    else if (m_tRotaryDirection == RotaryEncoder::CCW)
    {
        key_ = input::KEY_DOWN;
    }
}

void McpButtons::UpdateDisplays(const ltc::Source ltcSource)
{
    const auto nSource = static_cast<uint8_t>(ltcSource);

    Display::Get()->TextStatus(LtcSourceConst::NAME[nSource]);

    //	if (!ltc::g_DisabledOutputs.bMax7219) {
    if (ltc::Destination::IsEnabled(ltc::Destination::Output::MAX7219))
    {
        LtcDisplayMax7219::Get()->WriteChar(nSource);
        return;
    }
#if !defined(CONFIG_LTC_DISABLE_WS28XX)
    //	if (!ltc::g_DisabledOutputs.bWS28xx){
    if (ltc::Destination::IsEnabled(ltc::Destination::Output::WS28XX))
    {
        LtcDisplayRgb::Get()->WriteChar(nSource);
        return;
    }
#endif
#if !defined(CONFIG_LTC_DISABLE_RGB_PANEL)
    //	if (!ltc::g_DisabledOutputs.bRgbPanel) {
    if (ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL))
    {
        LtcDisplayRgb::Get()->ShowSource(ltcSource);
        return;
    }
#endif
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
    hal_i2c_.WriteRegister(mcp23x17::REG_GPIOB, static_cast<uint8_t>(1U << static_cast<uint8_t>(m_tLtcReaderSource)));

    UpdateDisplays(m_tLtcReaderSource);

    FUNC_PREFIX(GpioFsel(gpio::INTA, GPIO_FSEL_INPUT));
    FUNC_PREFIX(GpioSetPud(gpio::INTA, GPIO_PULL_UP));

    DEBUG_EXIT();
    return true;
}

bool McpButtons::Wait(ltc::Source& ltcSource, struct ltc::TimeCode& StartTimeCode, struct ltc::TimeCode& StopTimeCode)
{
    const auto nSource = static_cast<uint32_t>(ltcSource);

    if (__builtin_expect((LedBlink(static_cast<uint8_t>(1U << nSource)) >= m_nLedTickerMax), 0))
    {
        hal_i2c_.WriteRegister(mcp23x17::REG_GPIOB, static_cast<uint8_t>(1U << nSource));
        return false;
    }

    if (__builtin_expect(FUNC_PREFIX(GpioLev(gpio::INTA)) == 0, 0))
    {
        m_nLedTickerMax = UINT32_MAX;

        const auto nPortA = hal_i2c_.ReadRegister(mcp23x17::REG_GPIOA);
        const uint8_t nButtonsChanged = (nPortA ^ m_nPortAPrevious) & nPortA;

        m_nPortAPrevious = nPortA;

        printf("%.2x %.2x\r", nPortA, nButtonsChanged);

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
            if (BUTTON_STATE(button::LEFT))
            { // LEFT
                HandleActionLeft(ltcSource);
            }
            else if (BUTTON_STATE(button::RIGHT))
            { // RIGHT
                HandleActionRight(ltcSource);
            }
            else if (BUTTON_STATE(button::SELECT))
            { // SELECT
                HandleActionSelect(ltcSource);
                return false;
            }
            else if (BUTTON_STATE(button::START))
            { // START
                if (ltcSource == ltc::Source::INTERNAL)
                {
                    if (state_ != EDIT_TIMECODE_START)
                    {
                        Display::Get()->SetCursorPos(static_cast<uint8_t>(Display::Get()->GetColumns() - 7U), 0);
                        Display::Get()->PutString("[Start]");
                    }

                    key_ = input::KEY_ENTER;

                    switch (state_)
                    {
                        case SOURCE_SELECT:
                            state_ = EDIT_TIMECODE_START;
                            break;
                        case EDIT_TIMECODE_START:
                            state_ = EDIT_FPS;
                            break;
                        case EDIT_TIMECODE_STOP:
                        case EDIT_FPS:
                            state_ = EDIT_TIMECODE_START;
                            break;
                        default:
                            break;
                    }
                }
            }
            else if (BUTTON_STATE(button::STOP))
            { // STOP
                if (ltcSource == ltc::Source::INTERNAL)
                {
                    if (state_ != EDIT_TIMECODE_STOP)
                    {
                        Display::Get()->SetCursorPos(static_cast<uint8_t>(Display::Get()->GetColumns() - 7U), 0);
                        Display::Get()->PutString("[Stop] ");
                    }
                    key_ = input::KEY_ENTER;
                    state_ = EDIT_TIMECODE_STOP;
                }
            }
            else if (BUTTON_STATE(button::RESUME))
            { // RESUME
                if (ltcSource == ltc::Source::INTERNAL)
                {
                    key_ = input::KEY_ESC;
                }
            }
        }

        HandleRotary(nPortA, ltcSource);

        if (key_ != input::KEY_NOT_DEFINED)
        {
            switch (state_)
            {
                case EDIT_TIMECODE_START:
                    HandleInternalTimeCodeStart(StartTimeCode);
                    break;
                case EDIT_TIMECODE_STOP:
                    HandleInternalTimeCodeStop(StopTimeCode);
                    break;
                case EDIT_FPS:
                    HandleInternalTimeCodeFps(StartTimeCode);
                    break;
                default:
                    break;
            }
        }

        port_b_ = 0;
    }

    return true;
}

/*
 * Run state
 */

void McpButtons::SetRunState(RunStatus runState)
{
    DEBUG_PRINTF("%d %d", runState, m_tRunStatus);

    m_tRunStatus = runState;

    switch (runState)
    {
        case RunStatus::IDLE:
            hal_i2c_.WriteRegister(mcp23x17::REG_GPIOB, static_cast<uint8_t>(1U << static_cast<uint32_t>(m_tLtcReaderSource)));
            ltc::source::Show(m_tLtcReaderSource, m_bRunGpsTimeClient);
            break;
        case RunStatus::CONTINUE:
            hal_i2c_.WriteRegister(mcp23x17::REG_GPIOB, static_cast<uint8_t>(0x0F));
            Display::Get()->TextLine(4, ">CONTINUE?< ", 12);
            break;
        case RunStatus::REBOOT:
            hal_i2c_.WriteRegister(mcp23x17::REG_GPIOB, static_cast<uint8_t>(0xF0));
            Display::Get()->TextLine(4, ">REBOOT?  < ", 12);
            break;
        case RunStatus::TC_RESET:
            hal_i2c_.WriteRegister(mcp23x17::REG_GPIOB, static_cast<uint8_t>(0xAA));
            Display::Get()->TextLine(4, ">RESET TC?< ", 12);
            break;
        default:
            break;
    }
}

void McpButtons::HandleRunActionSelect()
{
    DEBUG_PRINTF("%d", m_tRunStatus);

    if (Display::Get()->IsSleep())
    {
        Display::Get()->SetSleep(false);
    }

    const auto nMillisNow =hal::Millis();

    if ((nMillisNow - m_nSelectMillis) < 300)
    {
        return;
    }

    m_nSelectMillis = nMillisNow;

    if (m_tRunStatus == RunStatus::IDLE)
    {
        SetRunState(RunStatus::CONTINUE);
        return;
    }

    if (m_tRunStatus == RunStatus::CONTINUE)
    {
        SetRunState(RunStatus::IDLE);
        return;
    }

    if (m_tRunStatus == RunStatus::REBOOT)
    {
        hal_i2c_.WriteRegister(mcp23x17::REG_GPIOB, static_cast<uint8_t>(0xFF));

        Display::Get()->SetSleep(false);
        Display::Get()->Cls();
        Display::Get()->TextStatus("Reboot ...");

        hal::Reboot();
        __builtin_unreachable();
        return;
    }

    if (m_tRunStatus == RunStatus::TC_RESET)
    {
        SetRunState(RunStatus::IDLE);
        LtcGenerator::Get()->ActionReset();
        return;
    }
}

void McpButtons::Run()
{
    if (__builtin_expect(!is_connected_, 0))
    {
        return;
    }

    if (__builtin_expect(FUNC_PREFIX(GpioLev(gpio::INTA)) == 0, 0))
    {
        const auto nPortA = hal_i2c_.ReadRegister(mcp23x17::REG_GPIOA);
        const uint8_t nButtonsChanged = (nPortA ^ m_nPortAPrevious) & nPortA;

        m_nPortAPrevious = nPortA;

        DEBUG_PRINTF("\n\n%.2x %.2x", nPortA, nButtonsChanged);

        if (m_tLtcReaderSource == ltc::Source::INTERNAL)
        {
            if (BUTTON_STATE(button::START))
            {
                LtcGenerator::Get()->ActionStart(!m_bUseAltFunction);
                return;
            }

            if (BUTTON_STATE(button::STOP))
            {
                LtcGenerator::Get()->ActionStop();
                return;
            }

            if (BUTTON_STATE(button::RESUME))
            {
                if (!m_bUseAltFunction)
                {
                    LtcGenerator::Get()->ActionResume();
                    return;
                }

                if (m_tRunStatus == RunStatus::IDLE)
                {
                    SetRunState(RunStatus::TC_RESET);
                    return;
                }

                if (m_tRunStatus == RunStatus::TC_RESET)
                {
                    SetRunState(RunStatus::IDLE);
                    return;
                }

                return;
            }
        }
        else if (m_tLtcReaderSource == ltc::Source::SYSTIME)
        {
            if (BUTTON_STATE(button::START))
            {
                SystimeReader::Get()->ActionStart();
                return;
            }
            if (BUTTON_STATE(button::STOP))
            {
                SystimeReader::Get()->ActionStop();
                return;
            }
        }

        if (BUTTON_STATE(button::SELECT))
        {
            HandleRunActionSelect();
            return;
        }

        if (BUTTON_STATE(button::LEFT))
        {
            if (m_tRunStatus == RunStatus::REBOOT)
            {
                SetRunState(RunStatus::CONTINUE);
                return;
            }

            if (m_tRunStatus == RunStatus::IDLE)
            {
                LtcGenerator::Get()->ActionBackward(m_nSkipSeconds);
                return;
            }

            return;
        }

        if (BUTTON_STATE(button::RIGHT))
        {
            if (m_tRunStatus == RunStatus::CONTINUE)
            {
                SetRunState(RunStatus::REBOOT);
                return;
            }

            if (m_tRunStatus == RunStatus::IDLE)
            {
                LtcGenerator::Get()->ActionForward(m_nSkipSeconds);
                return;
            }

            return;
        }

        m_tRotaryDirection = rotary_encoder_.Process(nPortA);

        DEBUG_PRINTF("%d %s", m_tRotaryDirection, m_tRotaryDirection == RotaryEncoder::CCW ? "CCW" : (m_tRotaryDirection == RotaryEncoder::CW ? "CW" : "!!"));

        if (m_tRotaryDirection == RotaryEncoder::CCW)
        {
            if (m_tRunStatus == RunStatus::REBOOT)
            {
                SetRunState(RunStatus::CONTINUE);
                return;
            }

            if (m_tRunStatus == RunStatus::IDLE)
            {
                LtcGenerator::Get()->ActionBackward(m_nSkipSeconds);
                return;
            }

            return;
        }

        if (m_tRotaryDirection == RotaryEncoder::CW)
        {
            if (m_tRunStatus == RunStatus::CONTINUE)
            {
                SetRunState(RunStatus::REBOOT);
                return;
            }

            if (m_tRunStatus == RunStatus::IDLE)
            {
                LtcGenerator::Get()->ActionForward(m_nSkipSeconds);
                return;
            }

            return;
        }
    }
}
