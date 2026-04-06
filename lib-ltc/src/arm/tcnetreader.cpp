/**
 * @file tcnetreader.cpp
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

#if defined(DEBUG_ARM_TCNETREADER)
#undef NDEBUG
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cstring>
#include <cassert>
#include "arm/tcnetreader.h"
#include "ltc.h"
#include "timecodeconst.h"
#include "hal_statusled.h"
// Input
#include "tcnet.h"
// Output
#include "ltcetc.h"
#include "ltcsender.h"
#include "tcnetdisplay.h"
#include "arm/ltcoutputs.h"
#include "firmware/debug/debug_dump.h"
 #include "firmware/debug/debug_debug.h"

static constexpr char kCmdLayer[] = "layer#";
static constexpr auto kLayerLength = sizeof(kCmdLayer) - 1U;

static constexpr char kCmdType[] = "type#";
static constexpr auto kTypeLength = sizeof(kCmdType) - 1U;

static constexpr char kCmdTimecode[] = "timecode#";
static constexpr auto kTimecodeLength = sizeof(kCmdTimecode) - 1U;

static constexpr auto kUdpPort = 0x0ACA;

#if defined(H3)
static void irq_timer0_handler([[maybe_unused]] uint32_t clo)
{
    gv_ltc_bTimeCodeAvailable = true;
}
#elif defined(GD32)
// Defined in platform_ltc.cpp
#endif

#if defined(H3)
static void arm_timer_handler()
{
    gv_ltc_nUpdatesPerSecond = gv_ltc_nUpdates - gv_ltc_nUpdatesPrevious;
    gv_ltc_nUpdatesPrevious = gv_ltc_nUpdates;
}
#elif defined(GD32)
// Defined in platform_ltc.cpp
#endif

void TCNetReader::Start()
{
    DEBUG_ENTRY();

#if defined(H3)
    irq_timer_set(IRQ_TIMER_0, static_cast<thunk_irq_timer_t>(irq_timer0_handler));
    H3_TIMER->TMR0_CUR = 0;
#elif defined(GD32)
    platform::ltc::timer11_config();
#endif

#if defined(H3)
    irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(arm_timer_handler));
    irq_handler_init();
#elif defined(GD32)
#endif

    handle_ = network::udp::Begin(kUdpPort, StaticCallbackFunctionInput);
    assert(handle_ != -1);

    LtcOutputs::Get()->Init();
    hal::statusled::SetMode(hal::statusled::Mode::NORMAL);

    DEBUG_EXIT();
}

void TCNetReader::Stop()
{
    DEBUG_ENTRY();

#if defined(H3)
    irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(nullptr));
#elif defined(GD32)
#endif

    DEBUG_EXIT();
}

void TCNetReader::Input(const uint8_t* buffer, uint32_t size, [[maybe_unused]] uint32_t from_ip, [[maybe_unused]] uint16_t from_port)
{
    if (__builtin_expect((memcmp("tcnet!", buffer, 6) != 0), 0))
    {
        return;
    }

    if (buffer[size - 1] == '\n')
    {
        size--;
    }

    debug::Dump(buffer, size);

    if ((size == (6 + kLayerLength + 1)) && (memcmp(&buffer[6], kCmdLayer, kLayerLength) == 0))
    {
        const auto kLayer = tcnet::GetLayer(buffer[6 + kTypeLength]);

        TCNet::Get()->SetLayer(kLayer);
        tcnet::display::show();

        DEBUG_PRINTF("tcnet!layer#%c -> %d", buffer[6 + kLayerLength + 1], kLayer);
        return;
    }

    if ((size == (6 + kTypeLength + 2)) && (memcmp(&buffer[6], kCmdType, kTypeLength) == 0))
    {
        if (buffer[6 + kTypeLength] == '2')
        {
            const auto kValue = 20U + buffer[6 + kTypeLength + 1] - '0';

            switch (kValue)
            {
                case 24:
                    TCNet::Get()->SetTimeCodeType(tcnet::TimeCodeType::kTimecodeTypeFilm);
                    break;
                case 25:
                    TCNet::Get()->SetTimeCodeType(tcnet::TimeCodeType::kTimecodeTypeEbu25Fps);
                    break;
                case 29:
                    TCNet::Get()->SetTimeCodeType(tcnet::TimeCodeType::kTimecodeTypeDf);
                    break;
                    ;
                default:
                    break;
            }

            tcnet::display::show();

            DEBUG_PRINTF("tcnet!type#%d", kValue);
            return;
        }

        if ((buffer[6 + kTypeLength] == '3') && (buffer[6 + kTypeLength + 1] == '0'))
        {
            TCNet::Get()->SetTimeCodeType(tcnet::TimeCodeType::kTimecodeTypeSmpte30Fps);
            tcnet::display::show();

            DEBUG_PUTS("tcnet!type#30");
            return;
        }

        DEBUG_PUTS("Invalid tcnet!type command");
        return;
    }

    if ((size == (6 + kTimecodeLength + 1)) && (memcmp(&buffer[6], kCmdTimecode, kTimecodeLength) == 0))
    {
        const auto kChar = buffer[6 + kTimecodeLength];
        const auto kUseTimeCode = ((kChar == 'y') || (kChar == 'Y'));

        TCNet::Get()->SetUseTimeCode(kUseTimeCode);
        tcnet::display::show();

        DEBUG_PRINTF("tcnet!timecode#%c -> %d", kChar, static_cast<int>(kUseTimeCode));
        return;
    }

    DEBUG_PUTS("Invalid command");
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#endif

void TCNetReader::ResetTimer(bool do_reset, const struct tcnet::TimeCode* timecode)
{
    if (do_reset_timer_ != do_reset)
    {
        do_reset_timer_ = do_reset;

        if (do_reset_timer_)
        {
            memcpy(&timecode_, timecode, sizeof(struct tcnet::TimeCode));

#if defined(H3)
            H3_TIMER->TMR0_CUR = 0;
            H3_TIMER->TMR0_INTV = TimeCodeConst::TMR_INTV[timecode->type];
            H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
#elif defined(GD32)
            platform::ltc::timer11_set_type(timecode->type);
#endif
            gv_ltc_bTimeCodeAvailable = true;
        }
    }
}

void TCNetReader::Handler(const struct tcnet::TimeCode* timecode)
{
    if (__builtin_expect((timecode->type != type_previous_), 0))
    {
        type_previous_ = timecode->type;
        do_reset_timer_ = false;
    }

    ResetTimer(!(timecode->frames != 0), timecode);

    gv_ltc_nUpdates = gv_ltc_nUpdates + 1;
}

void TCNetReader::Run()
{
    // Update timecode outputs if available
    __DMB(); // Data memory barrier to ensure memory consistency
    if (__builtin_expect((gv_ltc_bTimeCodeAvailable), 0))
    {
        if (ltc::Destination::IsEnabled(ltc::Destination::Output::LTC))
        {
            LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct ltc::TimeCode*>(&timecode_));
        }

        if (ltc::Destination::IsEnabled(ltc::Destination::Output::ARTNET))
        {
            artnet::SendTimeCode(reinterpret_cast<const struct artnet::TimeCode*>(&timecode_));
        }

        if (ltc::Destination::IsEnabled(ltc::Destination::Output::ETC))
        {
            LtcEtc::Get()->Send(reinterpret_cast<const struct midi::Timecode*>(&timecode_));
        }

        memcpy(&g_ltc_LtcTimeCode, &timecode_, sizeof(struct midi::Timecode));

        LtcOutputs::Get()->Update(const_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode));

        timecode_.frames++;

        if (timecode_.frames >= TimeCodeConst::FPS[timecode_.type])
        {
#if defined(H3)
            H3_TIMER->TMR0_CTRL &= ~TIMER_CTRL_EN_START;
#elif defined(GD32)
            TIMER_CTL0(TIMER11) &= ~TIMER_CTL0_CEN;
#endif
            timecode_.frames = 0;
        }

        gv_ltc_bTimeCodeAvailable = false;
    }

    __DMB();
    if (gv_ltc_nUpdatesPerSecond != 0)
    {
        hal::statusled::SetMode(hal::statusled::Mode::DATA);
        Reset(false);
    }
    else
    {
        LtcOutputs::Get()->ShowSysTime();
        hal::statusled::SetMode(hal::statusled::Mode::NORMAL);
        Reset(true);
    }
}
