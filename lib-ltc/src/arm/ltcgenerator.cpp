/**
 * @file ltcgenerator.cpp
 */
/* Copyright (C) 2022-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_ARM_LTCGENERATOR)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "arm/ltcgenerator.h"
#include "ltc.h"
#include "timecodeconst.h"
#include "network.h"
#include "hal.h"
#include "hal_statusled.h"
// Output
#include "ltcetc.h"
#include "ltcsender.h"
#include "arm/ltcoutputs.h"
#include "firmware/debug/debug_dump.h"
#include "firmware/debug/debug_debug.h"

#if !defined(CONFIG_LTC_DISABLE_GPIO_BUTTONS)
#define BUTTON(x) ((buttons_ >> x) & 0x01)
#define BUTTON_STATE(x) ((buttons_ & (1 << x)) == (1 << x))

#define BUTTON0_GPIO GPIO_EXT_22 // PA2 Start
#define BUTTON1_GPIO GPIO_EXT_15 // PA3 Stop
#define BUTTON2_GPIO GPIO_EXT_7  // PA6 Resume

#define BUTTONS_MASK ((1 << BUTTON0_GPIO) | (1 << BUTTON1_GPIO) | (1 << BUTTON2_GPIO))
#endif

#if defined(H3)
static void irq_timer0_handler([[maybe_unused]] uint32_t clo) {
    gv_ltc_bTimeCodeAvailable = true;
}
#elif defined(GD32)
// Defined in platform_ltc.cpp
#endif

static constexpr char kCmdStart[] = "start";
static constexpr auto kStartLength = sizeof(kCmdStart) - 1;

static constexpr char kCmdStop[] = "stop";
static constexpr auto kStopLength = sizeof(kCmdStop) - 1;

static constexpr char kCmdResume[] = "resume";
static constexpr auto kResumeLength = sizeof(kCmdResume) - 1;

static constexpr char kCmdRate[] = "rate#";
static constexpr auto kRateLength = sizeof(kCmdRate) - 1;

static constexpr char kCmdDirection[] = "direction#";
static constexpr auto kDirectionLength = sizeof(kCmdDirection) - 1;

static constexpr char kCmdPitch[] = "pitch#";
static constexpr auto kPitchLength = sizeof(kCmdPitch) - 1;

static constexpr char kCmdForward[] = "forward#";
static constexpr auto kForwardLength = sizeof(kCmdForward) - 1;

static constexpr char kCmdBackward[] = "backward#";
static constexpr auto kBackwardLength = sizeof(kCmdBackward) - 1;

static constexpr auto kUdpPort = 0x5443;

static int32_t Atoi(const char* buffer, uint32_t size) {
    assert(buffer != nullptr);
    assert(size <= 4); // -100

    const char* p = buffer;
    int32_t sign = 1;
    int32_t res = 0;

    if (*p == '-') {
        sign = -1;
        size--;
        p++;
    }

    for (; (size > 0) && (*p >= '0' && *p <= '9'); size--) {
        res = res * 10 + *p - '0';
        p++;
    }

    return sign * res;
}

LtcGenerator::LtcGenerator(const struct ltc::TimeCode* pStartLtcTimeCode, const struct ltc::TimeCode* pStopLtcTimeCode, bool bSkipFree, bool bIgnoreStart, bool bIgnoreStop)
    : m_pStartLtcTimeCode(const_cast<struct ltc::TimeCode*>(pStartLtcTimeCode)),
      m_pStopLtcTimeCode(const_cast<struct ltc::TimeCode*>(pStopLtcTimeCode)),
      skip_free_(bSkipFree),
      m_bIgnoreStart(bIgnoreStart),
      m_bIgnoreStop(bIgnoreStop),
      m_nStartSeconds(GetSeconds(*m_pStartLtcTimeCode)),
      m_nStopSeconds(GetSeconds(*m_pStopLtcTimeCode)) {
    assert(pStartLtcTimeCode != nullptr);
    assert(pStopLtcTimeCode != nullptr);

    assert(s_this == nullptr);
    s_this = this;

    gv_ltc_bTimeCodeAvailable = false;

    memcpy(&g_ltc_LtcTimeCode, pStartLtcTimeCode, sizeof(struct ltc::TimeCode));

    fps_ = TimeCodeConst::FPS[pStartLtcTimeCode->type];

    if (m_pStartLtcTimeCode->frames >= fps_) {
        m_pStartLtcTimeCode->frames = static_cast<uint8_t>(fps_ - 1);
    }

    if (m_pStopLtcTimeCode->frames >= fps_) {
        m_pStopLtcTimeCode->frames = static_cast<uint8_t>(fps_ - 1);
    }
}

void LtcGenerator::Start() {
    DEBUG_ENTRY();

#if !defined(CONFIG_LTC_DISABLE_GPIO_BUTTONS)
#if defined(H3)
    H3GpioFsel(BUTTON0_GPIO, GPIO_FSEL_EINT); // PA2
    H3GpioFsel(BUTTON1_GPIO, GPIO_FSEL_EINT); // PA3
    H3GpioFsel(BUTTON2_GPIO, GPIO_FSEL_EINT); // PA6

    H3GpioSetPud(BUTTON0_GPIO, GPIO_PULL_UP);
    H3GpioSetPud(BUTTON1_GPIO, GPIO_PULL_UP);
    H3GpioSetPud(BUTTON2_GPIO, GPIO_PULL_UP);

    H3GpioIntCfg(BUTTON0_GPIO, GPIO_INT_CFG_NEG_EDGE);
    H3GpioIntCfg(BUTTON1_GPIO, GPIO_INT_CFG_NEG_EDGE);
    H3GpioIntCfg(BUTTON2_GPIO, GPIO_INT_CFG_NEG_EDGE);

    H3_PIO_PA_INT->CTL |= BUTTONS_MASK;
    H3_PIO_PA_INT->STA = BUTTONS_MASK;
    H3_PIO_PA_INT->DEB = (0x0 << 0) | (0x7 << 4);
#else
#endif
#endif

    const auto kType = static_cast<uint32_t>(ltc::g_Type);

#if defined(H3)
    irq_handler_init();
    irq_timer_set(IRQ_TIMER_0, static_cast<thunk_irq_timer_t>(irq_timer0_handler));

    H3_TIMER->TMR0_INTV = TimeCodeConst::TMR_INTV[kType];
    H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
    H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
#elif defined(GD32)
    platform::ltc::timer11_config();
    platform::ltc::timer11_set_type(kType);
#endif

    handle_ = network::udp::Begin(kUdpPort, StaticCallbackFunction);
    assert(handle_ != -1);

    LtcOutputs::Get()->Init();

    hal::statusled::SetMode(hal::statusled::Mode::kNormal);

    DEBUG_EXIT();
}

void LtcGenerator::Stop() {
    DEBUG_ENTRY();

#if defined(H3)
    __disable_irq();
    irq_timer_set(IRQ_TIMER_0, nullptr);
#elif defined(GD32)
#endif

    handle_ = network::udp::End(kUdpPort);

    DEBUG_EXIT();
}

void LtcGenerator::ActionStart(bool do_reset) {
    DEBUG_ENTRY();

    if (state_ == kStarted) {
        DEBUG_EXIT();
        return;
    }

    state_ = kStarted;

    if (do_reset) {
        ActionReset();
    }

    LtcOutputs::Get()->ResetTimeCodeTypePrevious();

    DEBUG_EXIT();
}

void LtcGenerator::ActionStop() {
    DEBUG_ENTRY();

    state_ = kStopped;

    DEBUG_EXIT();
}

void LtcGenerator::ActionResume() {
    DEBUG_ENTRY();

    if (state_ != kStarted) {
        state_ = kStarted;
    }

    DEBUG_EXIT();
}

void LtcGenerator::ActionSetStart(const char* time_code) {
    DEBUG_ENTRY();

    const auto kSucces = ltc::parse_timecode(time_code, fps_, m_pStartLtcTimeCode);

    if (kSucces) {
        m_nStartSeconds = GetSeconds(*m_pStartLtcTimeCode);
    }

    DEBUG_EXIT();
}

void LtcGenerator::ActionSetStop(const char* time_code) {
    DEBUG_ENTRY();

    const auto kSucces = ltc::parse_timecode(time_code, fps_, m_pStopLtcTimeCode);

    if (kSucces) {
        m_nStopSeconds = GetSeconds(*m_pStopLtcTimeCode);
    }

    DEBUG_EXIT();
}

void LtcGenerator::ActionGoto(const char* time_code) {
    DEBUG_ENTRY();

    ActionStop();
    ActionSetStart(time_code);
    ActionStart();
    ActionStop();

    DEBUG_EXIT();
}

void LtcGenerator::ActionSetDirection(const char* time_code_direction) {
    DEBUG_ENTRY();

    if (memcmp("forward", time_code_direction, 7) == 0) {
        m_tDirection = ltcgenerator::Direction::kDirectionForward;
    } else if (memcmp("backward", time_code_direction, 8) == 0) {
        m_tDirection = ltcgenerator::Direction::kDirectionBackward;
    }

    DEBUG_PRINTF("m_tDirection=%d", m_tDirection);

    DEBUG_EXIT();
}

void LtcGenerator::ActionSetPitch(float time_code_pitch) {
    DEBUG_ENTRY();

    if ((time_code_pitch < -1) || (time_code_pitch > 1)) {
        return;
    }

    if (time_code_pitch < 0) {
        m_tPitch = ltcgenerator::Pitch::kPitchSlower;
        m_fPitchControl = -time_code_pitch;
    } else if (time_code_pitch == 0) {
        m_tPitch = ltcgenerator::Pitch::kPitchNormal;
        return;
    } else {
        m_tPitch = ltcgenerator::Pitch::kPitchFaster;
        m_fPitchControl = time_code_pitch;
    }

    m_nPitchPrevious = 0;
    pitch_ticker_ = 1;

    DEBUG_PRINTF("m_fPitchControl=%f, m_tPitch=%d", m_fPitchControl, m_tPitch);

    DEBUG_EXIT();
}

void LtcGenerator::ActionReset() {
    DEBUG_ENTRY();

    memcpy(&g_ltc_LtcTimeCode, m_pStartLtcTimeCode, sizeof(struct ltc::TimeCode));

    LtcOutputs::Get()->ResetTimeCodeTypePrevious();

    DEBUG_EXIT();
}

void LtcGenerator::ActionSetRate(const char* time_code_rate) {
    DEBUG_ENTRY();

    uint8_t fps;

    if ((state_ == kStopped) && (ltc::parse_timecode_rate(time_code_rate, fps))) {
        if (fps != fps_) {
            fps_ = fps;

            const auto kType = static_cast<uint8_t>(ltc::g_Type);
            g_ltc_LtcTimeCode.type = kType;

            m_pStartLtcTimeCode->type = kType;

            if (m_pStartLtcTimeCode->frames >= fps_) {
                m_pStartLtcTimeCode->frames = static_cast<uint8_t>(fps_ - 1);
            }

            m_pStopLtcTimeCode->type = kType;

            if (m_pStopLtcTimeCode->frames >= fps_) {
                m_pStopLtcTimeCode->frames = static_cast<uint8_t>(fps_ - 1);
            }

#if defined(H3)
            H3_TIMER->TMR0_INTV = TimeCodeConst::TMR_INTV[kType];
            H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
            H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
#elif defined(GD32)
            platform::ltc::timer11_set_type(kType);
#endif

            if (ltc::Destination::IsEnabled(ltc::Destination::Output::LTC)) {
                LtcSender::Get()->SetTimeCode(const_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode), false);
            }

            if (ltc::Destination::IsEnabled(ltc::Destination::Output::ARTNET)) {
                artnet::SendTimeCode(reinterpret_cast<const struct artnet::TimeCode*>(&g_ltc_LtcTimeCode));
            }

            if (ltc::Destination::IsEnabled(ltc::Destination::Output::ETC)) {
                LtcEtc::Get()->Send(reinterpret_cast<const struct midi::Timecode*>(&g_ltc_LtcTimeCode));
            }

            LtcOutputs::Get()->Update(const_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode));
        }
    }

    DEBUG_EXIT();
}

void LtcGenerator::ActionForward(uint32_t seconds) {
    DEBUG_ENTRY();

    if (state_ == kStarted) {
        DEBUG_EXIT();
        return;
    }

    if (state_ == kLimit) {
        state_ = kStarted;
    }

    const uint32_t kSeconds = GetSeconds(g_ltc_LtcTimeCode) + seconds;
    constexpr uint32_t kMaxSeconds = ((23 * 60) + 59) * 60 + 59;
    const auto kLimitSeconds = skip_free_ ? kMaxSeconds : m_nStopSeconds;

    if (kSeconds <= kLimitSeconds) {
        SetTimeCode(kSeconds);
    } else {
        memcpy(&g_ltc_LtcTimeCode, m_pStopLtcTimeCode, sizeof(struct ltc::TimeCode));
    }

    DEBUG_EXIT();
}

void LtcGenerator::ActionBackward(uint32_t seconds) {
    DEBUG_ENTRY();

    if (state_ == kStarted) {
        DEBUG_EXIT();
        return;
    }

    if (state_ == kLimit) {
        state_ = kStarted;
    }

    const auto kSeconds = GetSeconds(g_ltc_LtcTimeCode) - seconds;
    const auto kLimitSeconds = skip_free_ ? 0 : m_nStartSeconds;

    if (kSeconds >= kLimitSeconds) {
        SetTimeCode(kSeconds);
    } else {
        memcpy(&g_ltc_LtcTimeCode, m_pStartLtcTimeCode, sizeof(struct ltc::TimeCode));
    }

    DEBUG_EXIT();
}

void LtcGenerator::SetPitch(const char* pitch, uint32_t size) {
    DEBUG_ENTRY();
    debug::Dump(pitch, size);

    const auto kF = static_cast<float>(Atoi(pitch, size)) / 100;

    DEBUG_PRINTF("f=%f", kF);

    ActionSetPitch(kF);

    DEBUG_EXIT();
}

void LtcGenerator::SetSkip(const char* seconds, uint32_t size, ltcgenerator::Direction direction) {
    DEBUG_ENTRY();

    const auto kSeconds = static_cast<uint32_t>(Atoi(seconds, size));

    DEBUG_PRINTF("seconds=%d", kSeconds);

    if (direction == ltcgenerator::Direction::kDirectionForward) {
        ActionForward(kSeconds);
    } else {
        ActionBackward(kSeconds);
    }

    DEBUG_EXIT();
}

void LtcGenerator::HandleRequest(char* buffer, uint32_t size) {
    if (buffer != nullptr) {
        assert(size >= 8);
        udp_buffer_ = buffer;
        bytes_received_ = size;
    }

    if (bytes_received_ < 8) {
        return;
    }

    if (__builtin_expect((memcmp("ltc!", udp_buffer_, 4) != 0), 0)) {
        return;
    }

    if (udp_buffer_[bytes_received_ - 1] == '\n') {
        DEBUG_PUTS("\'\\n\'");
        bytes_received_--;
    }

    debug::Dump(udp_buffer_, bytes_received_);

    if (memcmp(&udp_buffer_[4], kCmdStart, kStartLength) == 0) {
        if (bytes_received_ == (4 + kStartLength)) {
            ActionStart();
            return;
        }

        if (bytes_received_ == (4 + kStartLength + 1 + ltc::timecode::CODE_MAX_LENGTH)) {
            if (udp_buffer_[4 + kStartLength] == '#') {
                ActionSetStart(&udp_buffer_[(4 + kStartLength + 1)]);
                return;
            }

            if (udp_buffer_[4 + kStartLength] == '!') {
                ActionSetStart(&udp_buffer_[(4 + kStartLength + 1)]);
                ActionStop();
                ActionStart();
                return;
            }

            if (udp_buffer_[4 + kStartLength] == '@') {
                ActionGoto(&udp_buffer_[(4 + kStartLength + 1)]);
                return;
            }
        }

        DEBUG_PUTS("Invalid !start command");
        return;
    }

    if (memcmp(&udp_buffer_[4], kCmdStop, kStopLength) == 0) {
        if (bytes_received_ == (4 + kStopLength)) {
            ActionStop();
            return;
        }

        if ((bytes_received_ == (4 + kStopLength + 1 + ltc::timecode::CODE_MAX_LENGTH)) && (udp_buffer_[4 + kStopLength] == '#')) {
            ActionSetStop(&udp_buffer_[(4 + kStopLength + 1)]);
            return;
        }

        DEBUG_PUTS("Invalid !stop command");
        return;
    }

    if (memcmp(&udp_buffer_[4], kCmdResume, kResumeLength) == 0) {
        ActionResume();
        return;
    }

    if (bytes_received_ == (4 + kRateLength + ltc::timecode::RATE_MAX_LENGTH)) {
        if (memcmp(&udp_buffer_[4], kCmdRate, kRateLength) == 0) {
            ActionSetRate(&udp_buffer_[(4 + kRateLength)]);
            return;
        }
    }

    if (bytes_received_ <= (4 + kDirectionLength + 8)) {
        if (memcmp(&udp_buffer_[4], kCmdDirection, kDirectionLength) == 0) {
            ActionSetDirection(&udp_buffer_[(4 + kDirectionLength)]);
            return;
        }
    }

    if (bytes_received_ <= (4 + kPitchLength + 4)) {
        if (memcmp(&udp_buffer_[4], kCmdPitch, kPitchLength) == 0) {
            SetPitch(&udp_buffer_[(4 + kPitchLength)], bytes_received_ - (4 + kPitchLength));
            return;
        }
    }

    if (bytes_received_ <= (4 + kForwardLength + 2)) {
        if (memcmp(&udp_buffer_[4], kCmdForward, kForwardLength) == 0) {
            SetSkip(&udp_buffer_[(4 + kForwardLength)], bytes_received_ - (4 + kForwardLength), ltcgenerator::Direction::kDirectionForward);
            return;
        }
    }

    if (bytes_received_ <= (4 + kBackwardLength + 2)) {
        if (memcmp(&udp_buffer_[4], kCmdBackward, kBackwardLength) == 0) {
            SetSkip(&udp_buffer_[(4 + kBackwardLength)], bytes_received_ - (4 + kBackwardLength), ltcgenerator::Direction::kDirectionBackward);
            return;
        }
    }

    DEBUG_PUTS("Invalid command");
}

void LtcGenerator::Input(const uint8_t* buffer, uint32_t size, [[maybe_unused]] uint32_t from_ip, [[maybe_unused]] uint16_t from_port) {
    udp_buffer_ = reinterpret_cast<char*>(const_cast<uint8_t*>(buffer));
    bytes_received_ = size;

    HandleRequest();
}

void LtcGenerator::HandleButtons() {
#if !defined(CONFIG_LTC_DISABLE_GPIO_BUTTONS)
#if defined(H3)
    buttons_ = H3_PIO_PA_INT->STA & BUTTONS_MASK;
#else
#endif

    if (__builtin_expect((buttons_ != 0), 0)) {
#if defined(H3)
        H3_PIO_PA_INT->STA = BUTTONS_MASK;
#else
#endif
        DEBUG_PRINTF("%d-%d-%d", BUTTON(BUTTON0_GPIO), BUTTON(BUTTON1_GPIO), BUTTON(BUTTON2_GPIO));

        if (BUTTON_STATE(BUTTON0_GPIO)) {
            ActionStart();
        } else if (BUTTON_STATE(BUTTON1_GPIO)) {
            ActionStop();
        } else if (BUTTON_STATE(BUTTON2_GPIO)) {
            ActionResume();
        }
    }
#endif
}

void LtcGenerator::Print() {
    printf("Internal\n");
    printf(" %s\n", ltc::get_type(static_cast<ltc::Type>(m_pStartLtcTimeCode->type)));
    printf(" Start : %.2d.%.2d.%.2d:%.2d\n", m_pStartLtcTimeCode->hours, m_pStartLtcTimeCode->minutes, m_pStartLtcTimeCode->seconds, m_pStartLtcTimeCode->frames);
    printf(" Stop  : %.2d.%.2d.%.2d:%.2d\n", m_pStopLtcTimeCode->hours, m_pStopLtcTimeCode->minutes, m_pStopLtcTimeCode->seconds, m_pStopLtcTimeCode->frames);
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#endif

void LtcGenerator::Increment() {
    //    if ((!m_bIgnoreStop) && (__builtin_expect((memcmp(&g_ltc_LtcTimeCode, m_pStopLtcTimeCode, sizeof(struct ltc::TimeCode)) == 0), 0)))
    //    {
    //        if (state_ == STARTED)
    //        {
    //            state_ = LIMIT;
    //        }
    //        return;
    //    }

    // Increment frames
    g_ltc_LtcTimeCode.frames++;

    // Drop-frame timecode handling BEFORE rolling to the next second
    if (ltc::g_Type == ltc::Type::DF) {
        // Skip frames 00 and 01 except every 10th minute
        if ((g_ltc_LtcTimeCode.minutes % 10 != 0) && (g_ltc_LtcTimeCode.seconds == 0) && (g_ltc_LtcTimeCode.frames < 2)) {
            g_ltc_LtcTimeCode.frames = 2;
        }
    }

    // Handle frame rollover
    if (g_ltc_LtcTimeCode.frames >= fps_) {
        g_ltc_LtcTimeCode.frames = 0;

        // Increment seconds
        if (++g_ltc_LtcTimeCode.seconds >= 60) {
            g_ltc_LtcTimeCode.seconds = 0;

            // Increment minutes
            if (++g_ltc_LtcTimeCode.minutes >= 60) {
                g_ltc_LtcTimeCode.minutes = 0;

                // Increment hours
                if (++g_ltc_LtcTimeCode.hours >= 24) {
                    g_ltc_LtcTimeCode.hours = 0;
                }
            }
        }
    }
}

void LtcGenerator::Decrement() {
    if ((!m_bIgnoreStart) && (__builtin_expect((memcmp(&g_ltc_LtcTimeCode, m_pStartLtcTimeCode, sizeof(struct ltc::TimeCode)) == 0), 0))) {
        if (state_ == kStarted) {
            state_ = kLimit;
        }
        return;
    }

    // Decrement frames
    if (g_ltc_LtcTimeCode.frames > 0) {
        g_ltc_LtcTimeCode.frames--;
    } else {
        g_ltc_LtcTimeCode.frames = static_cast<uint8_t>(fps_ - 1);
    }

    // Handle drop-frame logic after frames decrement
    if (ltc::g_Type == ltc::Type::DF) {
        if ((g_ltc_LtcTimeCode.minutes % 10 != 0) && (g_ltc_LtcTimeCode.seconds == 0) && (g_ltc_LtcTimeCode.frames == fps_ - 1)) {
            g_ltc_LtcTimeCode.frames = 1; // Skip to frame 01
        }
    }

    // Handle seconds rollover
    if (g_ltc_LtcTimeCode.frames == fps_ - 1) {
        if (g_ltc_LtcTimeCode.seconds > 0) {
            g_ltc_LtcTimeCode.seconds--;
        } else {
            g_ltc_LtcTimeCode.seconds = 59;
        }

        // Handle minutes rollover
        if (g_ltc_LtcTimeCode.minutes > 0) {
            g_ltc_LtcTimeCode.minutes--;
        } else {
            g_ltc_LtcTimeCode.minutes = 59;
        }

        // Handle hours rollover
        if (g_ltc_LtcTimeCode.hours > 0) {
            g_ltc_LtcTimeCode.hours--;
        } else {
            g_ltc_LtcTimeCode.hours = 23;
        }
    }
}

bool LtcGenerator::PitchControl() {
    const auto kPitch = static_cast<uint32_t>(m_fPitchControl * static_cast<float>(pitch_ticker_)); // / 100;
    const auto kR = (kPitch - m_nPitchPrevious);

    m_nPitchPrevious = kPitch;
    pitch_ticker_++;

    return (kR != 0);
}

void LtcGenerator::SetTimeCode(uint32_t seconds) {
    g_ltc_LtcTimeCode.hours = static_cast<uint8_t>(seconds / 3600U);
    seconds -= g_ltc_LtcTimeCode.hours * 3600;
    g_ltc_LtcTimeCode.minutes = static_cast<uint8_t>(seconds / 60U);
    seconds -= g_ltc_LtcTimeCode.minutes * 60;
    g_ltc_LtcTimeCode.seconds = static_cast<uint8_t>(seconds);
}

void LtcGenerator::Update() {
    if (__builtin_expect((state_ == kStopped), 0)) {
        __DMB(); // Data memory barrier to ensure memory consistency
        if (gv_ltc_bTimeCodeAvailable) {
            gv_ltc_bTimeCodeAvailable = false;

            if (ltc::Destination::IsEnabled(ltc::Destination::Output::LTC)) {
                LtcSender::Get()->SetTimeCode(static_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode), false);
            }
        }

        return;
    }

    __DMB(); // Data memory barrier to ensure memory consistency
    if (gv_ltc_bTimeCodeAvailable) {
        gv_ltc_bTimeCodeAvailable = false;
        if (__builtin_expect((m_tDirection == ltcgenerator::Direction::kDirectionForward), 1)) {
            if (__builtin_expect((m_tPitch == ltcgenerator::Pitch::kPitchNormal), 1)) {
                Increment();
            } else {
                if (m_tPitch == ltcgenerator::Pitch::kPitchFaster) {
                    Increment();
                    if (PitchControl()) {
                        Increment();
                    }
                } else {
                    if (!PitchControl()) {
                        Increment();
                    }
                }
            }
        } else { // LTC_GENERATOR_BACKWARD
            if (__builtin_expect((m_tPitch == ltcgenerator::Pitch::kPitchNormal), 1)) {
                Decrement();
            } else {
                if (m_tPitch == ltcgenerator::Pitch::kPitchFaster) {
                    Decrement();
                    if (PitchControl()) {
                        Decrement();
                    }
                } else {
                    if (!PitchControl()) {
                        Decrement();
                    }
                }
            }
        }

        if (ltc::Destination::IsEnabled(ltc::Destination::Output::LTC)) {
            LtcSender::Get()->SetTimeCode(static_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode), false);
        }

        if (ltc::Destination::IsEnabled(ltc::Destination::Output::ARTNET)) {
            artnet::SendTimeCode(reinterpret_cast<const struct artnet::TimeCode*>(&g_ltc_LtcTimeCode));
        }

        if (ltc::Destination::IsEnabled(ltc::Destination::Output::ETC)) {
            LtcEtc::Get()->Send(reinterpret_cast<const struct midi::Timecode*>(&g_ltc_LtcTimeCode));
        }

        LtcOutputs::Get()->Update(static_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode));
    }
}
