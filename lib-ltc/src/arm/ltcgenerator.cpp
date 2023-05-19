/**
 * @file ltcgenerator.cpp
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstring>
#include <cstdio>
#include <cassert>

#include "ltcgenerator.h"
#include "ltc.h"
#include "timecodeconst.h"

#include "network.h"
#include "hardware.h"

// Output
#include "artnetnode.h"
#include "rtpmidi.h"
#include "ltcetc.h"
#include "ltcsender.h"
#include "ltcoutputs.h"

#include "platform_ltc.h"

#include "debug.h"

#if !defined(CONFIG_LTC_DISABLE_GPIO_BUTTONS)
# define BUTTON(x)			((m_nButtons >> x) & 0x01)
# define BUTTON_STATE(x)	((m_nButtons & (1 << x)) == (1 << x))

# define BUTTON0_GPIO		GPIO_EXT_22		// PA2 Start
# define BUTTON1_GPIO		GPIO_EXT_15		// PA3 Stop
# define BUTTON2_GPIO		GPIO_EXT_7		// PA6 Resume

# define BUTTONS_MASK		((1 << BUTTON0_GPIO) |  (1 << BUTTON1_GPIO) | (1 << BUTTON2_GPIO))

# if defined (H3)
# else
#  error
# endif
#endif

#if defined (H3)
static void irq_timer0_handler(__attribute__((unused)) uint32_t clo) {
	if (!g_ltc_ptLtcDisabledOutputs.bLtc) {
		LtcSender::Get()->SetTimeCode(static_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode), false);
	}

	gv_ltc_bTimeCodeAvailable = true;
}
#elif defined (GD32)
	// Defined in platform_ltc.cpp
#endif

namespace cmd {
static constexpr char START[] = "start";
static constexpr char STOP[] = "stop";
static constexpr char RESUME[] = "resume";
static constexpr char RATE[] = "rate#";
static constexpr char DIRECTION[] = "direction#";
static constexpr char PITCH[] = "pitch#";
static constexpr char FORWARD[] = "forward#";
static constexpr char BACKWARD[] = "backward#";
}

namespace length {
static constexpr auto START = sizeof(cmd::START) - 1;
static constexpr auto STOP = sizeof(cmd::STOP) - 1;
static constexpr auto RESUME = sizeof(cmd::RESUME) - 1;
static constexpr auto RATE = sizeof(cmd::RATE) - 1;
static constexpr auto DIRECTION = sizeof(cmd::DIRECTION) - 1;
static constexpr auto PITCH = sizeof(cmd::PITCH) - 1;
static constexpr auto FORWARD = sizeof(cmd::FORWARD) - 1;
static constexpr auto BACKWARD = sizeof(cmd::BACKWARD) - 1;
}

namespace udp {
static constexpr auto PORT = 0x5443;
}

static int32_t atoi(const char *pBuffer, uint32_t nSize) {
	assert(pBuffer != nullptr);
	assert(nSize <= 4); // -100

	const char *p = pBuffer;
	int32_t sign = 1;
	int32_t res = 0;

	if (*p == '-') {
		sign = -1;
		nSize--;
		p++;
	}

	for (; (nSize > 0) && (*p >= '0' && *p <= '9'); nSize--) {
		res = res * 10 + *p - '0';
		p++;
	}

	DEBUG_PRINTF("sign * res = %d", sign * res);

	return sign * res;
}

LtcGenerator *LtcGenerator::s_pThis;

LtcGenerator::LtcGenerator(const struct ltc::TimeCode* pStartLtcTimeCode, const struct ltc::TimeCode* pStopLtcTimeCode, bool bSkipFree):
	m_pStartLtcTimeCode(const_cast<struct ltc::TimeCode*>(pStartLtcTimeCode)),
	m_pStopLtcTimeCode(const_cast<struct ltc::TimeCode*>(pStopLtcTimeCode)),
	m_bSkipFree(bSkipFree),
	m_nStartSeconds(GetSeconds(*m_pStartLtcTimeCode)),
	m_nStopSeconds(GetSeconds(*m_pStopLtcTimeCode))
{
	assert(pStartLtcTimeCode != nullptr);
	assert(pStopLtcTimeCode != nullptr);

	assert(s_pThis == nullptr);
	s_pThis = this;

	gv_ltc_bTimeCodeAvailable = false;

	memcpy(&g_ltc_LtcTimeCode, pStartLtcTimeCode, sizeof(struct ltc::TimeCode));

	m_nFps = TimeCodeConst::FPS[pStartLtcTimeCode->nType];
	m_nTimer0Interval = TimeCodeConst::TMR_INTV[pStartLtcTimeCode->nType];

	if (m_pStartLtcTimeCode->nFrames >= m_nFps) {
		m_pStartLtcTimeCode->nFrames = static_cast<uint8_t>(m_nFps - 1);
	}

	if (m_pStopLtcTimeCode->nFrames >= m_nFps) {
		m_pStopLtcTimeCode->nFrames = static_cast<uint8_t>(m_nFps - 1);
	}
}

void LtcGenerator::Start() {
	DEBUG_ENTRY

#if !defined(CONFIG_LTC_DISABLE_GPIO_BUTTONS)
# if defined (H3)
	h3_gpio_fsel(BUTTON0_GPIO, GPIO_FSEL_EINT); // PA2
	h3_gpio_fsel(BUTTON1_GPIO, GPIO_FSEL_EINT);	// PA3
	h3_gpio_fsel(BUTTON2_GPIO, GPIO_FSEL_EINT);	// PA6

	h3_gpio_pud(BUTTON0_GPIO, GPIO_PULL_UP);
	h3_gpio_pud(BUTTON1_GPIO, GPIO_PULL_UP);
	h3_gpio_pud(BUTTON2_GPIO, GPIO_PULL_UP);

	h3_gpio_int_cfg(BUTTON0_GPIO, GPIO_INT_CFG_NEG_EDGE);
	h3_gpio_int_cfg(BUTTON1_GPIO, GPIO_INT_CFG_NEG_EDGE);
	h3_gpio_int_cfg(BUTTON2_GPIO, GPIO_INT_CFG_NEG_EDGE);

	H3_PIO_PA_INT->CTL |= BUTTONS_MASK;
	H3_PIO_PA_INT->STA = BUTTONS_MASK;
	H3_PIO_PA_INT->DEB = (0x0 << 0) | (0x7 << 4);
# else
# endif
#endif

	m_nHandle = Network::Get()->Begin(udp::PORT);
	assert(m_nHandle != -1);

#if defined (H3)
	irq_timer_init();
	irq_timer_set(IRQ_TIMER_0, static_cast<thunk_irq_timer_t>(irq_timer0_handler));

	H3_TIMER->TMR0_INTV = m_nTimer0Interval;
	H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
	H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
#elif defined (GD32)
	platform::ltc::timer11_config();
	TIMER_CH0CV(TIMER11) = m_nTimer0Interval;
#endif

	LtcOutputs::Get()->Init();

	if (!g_ltc_ptLtcDisabledOutputs.bLtc) {
		LtcSender::Get()->SetTimeCode(const_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode), false);
	}

	if (!g_ltc_ptLtcDisabledOutputs.bArtNet) {
		ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct TArtNetTimeCode*>(&g_ltc_LtcTimeCode));
	}

	if (!g_ltc_ptLtcDisabledOutputs.bRtpMidi) {
		RtpMidi::Get()->SendTimeCode(reinterpret_cast<const struct midi::Timecode *>(&g_ltc_LtcTimeCode));
	}

	if (!g_ltc_ptLtcDisabledOutputs.bEtc) {
		LtcEtc::Get()->Send(reinterpret_cast<const struct midi::Timecode *>(&g_ltc_LtcTimeCode));
	}

	LtcOutputs::Get()->Update(const_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode));

	Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);

	DEBUG_EXIT
}

void LtcGenerator::Stop() {
	DEBUG_ENTRY

#if defined (H3)
	__disable_irq();
	irq_timer_set(IRQ_TIMER_0, nullptr);
#elif defined (GD32)
#endif

	m_nHandle = Network::Get()->End(udp::PORT);

	DEBUG_EXIT
}

void LtcGenerator::ActionStart(bool bDoReset) {
	DEBUG_ENTRY


	if (m_State == STARTED) {
		DEBUG_EXIT
		return;
	}

	m_State = STARTED;

	if (bDoReset) {
		ActionReset();
	}

	LtcOutputs::Get()->ResetTimeCodeTypePrevious();

	DEBUG_EXIT
}

void LtcGenerator::ActionStop() {
	DEBUG_ENTRY

	m_State = STOPPED;

	DEBUG_EXIT
}

void LtcGenerator::ActionResume() {
	DEBUG_ENTRY

	if (m_State != STARTED) {
		m_State = STARTED;
	}

	DEBUG_EXIT
}

void LtcGenerator::ActionSetStart(const char *pTimeCode) {
	DEBUG_ENTRY

	ltc::parse_timecode(pTimeCode, m_nFps, m_pStartLtcTimeCode);

	m_nStartSeconds = GetSeconds(*m_pStartLtcTimeCode);

	DEBUG_EXIT
}

void LtcGenerator::ActionSetStop(const char *pTimeCode) {
	DEBUG_ENTRY

	ltc::parse_timecode(pTimeCode, m_nFps, m_pStopLtcTimeCode);

	m_nStopSeconds = GetSeconds(*m_pStopLtcTimeCode);

	DEBUG_EXIT
}

void LtcGenerator::ActionGoto(const char *pTimeCode) {
	DEBUG_ENTRY

	ActionStop();
	ActionSetStart(pTimeCode);
	ActionStart();
	ActionStop();

	DEBUG_EXIT
}

void LtcGenerator::ActionSetDirection(const char *pTimeCodeDirection) {
	DEBUG_ENTRY

	if (memcmp("forward", pTimeCodeDirection, 7) == 0) {
		m_tDirection = LTC_GENERATOR_FORWARD;
	} else if (memcmp("backward", pTimeCodeDirection, 8) == 0) {
		m_tDirection = LTC_GENERATOR_BACKWARD;
	}

	DEBUG_PRINTF("m_tDirection=%d", m_tDirection);

	DEBUG_EXIT
}

void LtcGenerator::ActionSetPitch(float fTimeCodePitch) {
	DEBUG_ENTRY

	if ((fTimeCodePitch < -1) || (fTimeCodePitch > 1)) {
		return;
	}

	if (fTimeCodePitch < 0) {
		m_tPitch = LTC_GENERATOR_SLOWER;
		m_fPitchControl = -fTimeCodePitch;
	} else if (fTimeCodePitch == 0) {
		m_tPitch = LTC_GENERATOR_NORMAL;
		return;
	} else {
		m_tPitch = LTC_GENERATOR_FASTER;
		m_fPitchControl = fTimeCodePitch;
	}

	m_nPitchPrevious = 0;
	m_nPitchTicker = 1;

	DEBUG_PRINTF("m_fPitchControl=%f, m_tPitch=%d", m_fPitchControl, m_tPitch);

	DEBUG_EXIT
}

void LtcGenerator::ActionReset() {
	DEBUG_ENTRY

	memcpy(&g_ltc_LtcTimeCode, m_pStartLtcTimeCode, sizeof(struct ltc::TimeCode));

	LtcOutputs::Get()->ResetTimeCodeTypePrevious();

	DEBUG_EXIT
}

void LtcGenerator::ActionSetRate(const char *pTimeCodeRate) {
	DEBUG_ENTRY

	uint8_t nFps;
	ltc::Type type;

	if ((m_State == STOPPED) && (ltc::parse_timecode_rate(pTimeCodeRate, nFps, type))) {
		if (nFps != m_nFps) {
			const auto nType = static_cast<uint8_t>(type);
			m_nFps = nFps;
			//
			g_ltc_LtcTimeCode.nType = nType;
			//
			m_pStartLtcTimeCode->nType = nType;
			if (m_pStartLtcTimeCode->nFrames >= m_nFps) {
				m_pStartLtcTimeCode->nFrames = static_cast<uint8_t>(m_nFps - 1);
			}
			m_pStopLtcTimeCode->nType = nType;
			if (m_pStopLtcTimeCode->nFrames >= m_nFps) {
				m_pStopLtcTimeCode->nFrames = static_cast<uint8_t>(m_nFps - 1);
			}
			//
			m_nTimer0Interval = TimeCodeConst::TMR_INTV[nType];
#if defined (H3)
			H3_TIMER->TMR0_INTV = m_nTimer0Interval;
			H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
			H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
#elif defined (GD32)
			TIMER_CNT(TIMER11) = 0;
			TIMER_CH0CV(TIMER11) = m_nTimer0Interval;
#endif
			//
			if (!g_ltc_ptLtcDisabledOutputs.bLtc) {
				LtcSender::Get()->SetTimeCode(const_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode), false);
			}

			if (!g_ltc_ptLtcDisabledOutputs.bArtNet) {
				ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct TArtNetTimeCode*>(&g_ltc_LtcTimeCode));
			}

			if (!g_ltc_ptLtcDisabledOutputs.bRtpMidi) {
				RtpMidi::Get()->SendTimeCode(reinterpret_cast<const struct midi::Timecode *>(&g_ltc_LtcTimeCode));
			}

			if (!g_ltc_ptLtcDisabledOutputs.bEtc) {
				LtcEtc::Get()->Send(reinterpret_cast<const struct midi::Timecode *>(&g_ltc_LtcTimeCode));
			}

			LtcOutputs::Get()->Update(const_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode));
		}
	}

	DEBUG_EXIT
}

void LtcGenerator::ActionForward(int32_t nSeconds) {
	DEBUG_ENTRY

	if (m_State == STARTED) {
		DEBUG_EXIT
		return;
	}

	if (m_State == LIMIT) {
		m_State = STARTED;
	}

	const auto s = GetSeconds(g_ltc_LtcTimeCode) + nSeconds;
	constexpr auto nMaxSeconds = ((23 * 60) + 59) * 60 + 59;
	const auto nLimit = m_bSkipFree ? nMaxSeconds : m_nStopSeconds;

	if (s <= nLimit) {
		SetTimeCode(s);
	} else {
		memcpy(&g_ltc_LtcTimeCode, m_pStopLtcTimeCode, sizeof(struct ltc::TimeCode));
	}

	DEBUG_EXIT
}

void LtcGenerator::ActionBackward(int32_t nSeconds) {
	DEBUG_ENTRY

	if (m_State == STARTED) {
		DEBUG_EXIT
		return;
	}

	if (m_State == LIMIT) {
		m_State = STARTED;
	}

	const auto s = GetSeconds(g_ltc_LtcTimeCode) - nSeconds;
	const auto nLimit = m_bSkipFree ? 0 : m_nStartSeconds;

	if (s >= nLimit) {
		SetTimeCode(s);
	} else {
		memcpy(&g_ltc_LtcTimeCode, m_pStartLtcTimeCode, sizeof(struct ltc::TimeCode));
	}

	DEBUG_EXIT
}

void LtcGenerator::SetPitch(const char *pTimeCodePitch, uint32_t nSize) {
	DEBUG_ENTRY
	debug_dump(const_cast<char*>(pTimeCodePitch), static_cast<uint16_t>(nSize));

	const auto f = static_cast<float>(atoi(pTimeCodePitch, nSize)) / 100;

	DEBUG_PRINTF("f=%f", f);

	ActionSetPitch(f);

	DEBUG_EXIT
}

void LtcGenerator::SetSkip(const char *pSeconds, uint32_t nSize, TLtcGeneratorDirection tDirection) {
	DEBUG_ENTRY
	debug_dump(const_cast<char*>(pSeconds), static_cast<uint16_t>(nSize));

	const auto nSeconds = atoi(pSeconds, nSize);

	DEBUG_PRINTF("nSeconds=%d", nSeconds);

	if (tDirection == LTC_GENERATOR_FORWARD) {
		ActionForward(nSeconds);
	} else {
		ActionBackward(nSeconds);
	}

	DEBUG_EXIT
}

void LtcGenerator::HandleRequest(void *pBuffer, uint16_t nBufferLength) {
	if ((pBuffer != nullptr) && (nBufferLength <= sizeof(m_Buffer))) {
		memcpy(m_Buffer, pBuffer, nBufferLength);
		m_nBytesReceived = nBufferLength;
	}

	if (__builtin_expect((memcmp("ltc!", m_Buffer, 4) != 0), 0)) {
		return;
	}

	if (m_Buffer[m_nBytesReceived - 1] == '\n') {
		DEBUG_PUTS("\'\\n\'");
		m_nBytesReceived--;
	}

	debug_dump(m_Buffer, m_nBytesReceived);

	if (memcmp(&m_Buffer[4], cmd::START, length::START) == 0) {
		if (m_nBytesReceived == (4 + length::START)) {
			ActionStart();
			return;
		}

		if (m_nBytesReceived == (4 + length::START + 1 + ltc::timecode::CODE_MAX_LENGTH)) {
			if (m_Buffer[4 + length::START] == '#') {
				ActionSetStart(&m_Buffer[(4 + length::START + 1)]);
				return;
			}

			if (m_Buffer[4 + length::START] == '!') {
				ActionSetStart(&m_Buffer[(4 + length::START + 1)]);
				ActionStop();
				ActionStart();
				return;
			}

			if (m_Buffer[4 + length::START] == '@') {
				ActionGoto(&m_Buffer[(4 + length::START + 1)]);
				return;
			}
		}

		DEBUG_PUTS("Invalid !start command");
		return;
	}

	if (memcmp(&m_Buffer[4], cmd::STOP, length::STOP) == 0) {
		if (m_nBytesReceived == (4 + length::STOP)) {
			ActionStop();
			return;
		}

		if ((m_nBytesReceived == (4 + length::STOP + 1 + ltc::timecode::CODE_MAX_LENGTH))  && (m_Buffer[4 + length::STOP] == '#')) {
			ActionSetStop(&m_Buffer[(4 + length::STOP + 1)]);
			return;
		}

		DEBUG_PUTS("Invalid !stop command");
		return;
	}

	if (memcmp(&m_Buffer[4], cmd::RESUME, length::RESUME) == 0) {
		ActionResume();
		return;
	}

	if (m_nBytesReceived == (4 + length::RATE + ltc::timecode::RATE_MAX_LENGTH)) {
		if (memcmp(&m_Buffer[4], cmd::RATE, length::RATE) == 0) {
			ActionSetRate(&m_Buffer[(4 + length::RATE)]);
			return;
		}
	}

	if (m_nBytesReceived <= (4 + length::DIRECTION + 8)) {
		if (memcmp(&m_Buffer[4], cmd::DIRECTION, length::DIRECTION) == 0) {
			ActionSetDirection(&m_Buffer[(4 + length::DIRECTION)]);
			return;
		}
	}

	if (m_nBytesReceived <= (4 + length::PITCH + 4)) {
		if (memcmp(&m_Buffer[4], cmd::PITCH, length::PITCH) == 0) {
			SetPitch(&m_Buffer[(4 + length::PITCH)], m_nBytesReceived - (4 + length::PITCH));
			return;
		}
	}

	if (m_nBytesReceived <= (4 + length::FORWARD + 2)) {
		if (memcmp(&m_Buffer[4], cmd::FORWARD, length::FORWARD) == 0) {
			SetSkip(&m_Buffer[(4 + length::FORWARD)], m_nBytesReceived - (4 + length::FORWARD), LTC_GENERATOR_FORWARD);
			return;
		}
	}

	if (m_nBytesReceived <= (4 + length::BACKWARD + 2)) {
		if (memcmp(&m_Buffer[4], cmd::BACKWARD, length::BACKWARD) == 0) {
			SetSkip(&m_Buffer[(4 + length::BACKWARD)], m_nBytesReceived - (4 + length::BACKWARD), LTC_GENERATOR_BACKWARD);
			return;
		}
	}

	DEBUG_PUTS("Invalid command");
}

void LtcGenerator::HandleUdpRequest() {
	uint32_t nIPAddressFrom;
	uint16_t nForeignPort;

	m_nBytesReceived = Network::Get()->RecvFrom(m_nHandle, &m_Buffer, sizeof(m_Buffer), &nIPAddressFrom, &nForeignPort);

	if (__builtin_expect((m_nBytesReceived < 8), 1)) {
		return;
	}

	HandleRequest();
}

void LtcGenerator::HandleButtons() {
#if !defined(CONFIG_LTC_DISABLE_GPIO_BUTTONS)
# if defined (H3)
	m_nButtons = H3_PIO_PA_INT->STA & BUTTONS_MASK;
# else
# endif

	if (__builtin_expect((m_nButtons != 0), 0)) {
# if defined (H3)
		H3_PIO_PA_INT->STA = BUTTONS_MASK;
# else
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
	printf(" %s\n", ltc::get_type(static_cast<ltc::Type>(m_pStartLtcTimeCode->nType)));
	printf(" Start : %.2d.%.2d.%.2d:%.2d\n", m_pStartLtcTimeCode->nHours, m_pStartLtcTimeCode->nMinutes, m_pStartLtcTimeCode->nSeconds, m_pStartLtcTimeCode->nFrames);
	printf(" Stop  : %.2d.%.2d.%.2d:%.2d\n", m_pStopLtcTimeCode->nHours, m_pStopLtcTimeCode->nMinutes, m_pStopLtcTimeCode->nSeconds, m_pStopLtcTimeCode->nFrames);
}

void LtcGenerator::Increment() {

	if (__builtin_expect((memcmp(&g_ltc_LtcTimeCode, m_pStopLtcTimeCode, sizeof(struct ltc::TimeCode)) == 0), 0)) {
		if (m_State == STARTED) {
			m_State = LIMIT;
		}
		return;
	}

	if (__builtin_expect((m_State == STOPPED), 0)) {
		return;
	}

	g_ltc_LtcTimeCode.nFrames++;
	if (m_nFps == g_ltc_LtcTimeCode.nFrames) {
		g_ltc_LtcTimeCode.nFrames = 0;

		g_ltc_LtcTimeCode.nSeconds++;
		if (g_ltc_LtcTimeCode.nSeconds == 60) {
			g_ltc_LtcTimeCode.nSeconds = 0;

			g_ltc_LtcTimeCode.nMinutes++;
			if (g_ltc_LtcTimeCode.nMinutes == 60) {
				g_ltc_LtcTimeCode.nMinutes = 0;

				g_ltc_LtcTimeCode.nHours++;
				if (g_ltc_LtcTimeCode.nHours == 24) {
					g_ltc_LtcTimeCode.nHours = 0;
				}
			}
		}
	}

	//FIXME Add support for DF
}

void LtcGenerator::Decrement() {

	if (__builtin_expect((memcmp(&g_ltc_LtcTimeCode, m_pStartLtcTimeCode, sizeof(struct ltc::TimeCode)) == 0), 0)) {
		if (m_State == STARTED) {
			m_State = LIMIT;
		}
		return;
	}

	if (__builtin_expect((m_State == STOPPED), 0)) {
		return;
	}

	if (g_ltc_LtcTimeCode.nFrames > 0) {
		g_ltc_LtcTimeCode.nFrames--;
	} else {
		g_ltc_LtcTimeCode.nFrames = static_cast<uint8_t>(m_nFps - 1);
	}

	if (g_ltc_LtcTimeCode.nFrames == m_nFps - 1) {
		if (g_ltc_LtcTimeCode.nSeconds > 0) {
			g_ltc_LtcTimeCode.nSeconds--;
		} else {
			g_ltc_LtcTimeCode.nSeconds = 59;
		}

		if (g_ltc_LtcTimeCode.nSeconds == 59) {
			if (g_ltc_LtcTimeCode.nMinutes > 0) {
				g_ltc_LtcTimeCode.nMinutes--;
			} else {
				g_ltc_LtcTimeCode.nMinutes = 59;
			}

			if (g_ltc_LtcTimeCode.nMinutes == 59) {
				if (g_ltc_LtcTimeCode.nHours > 0) {
					g_ltc_LtcTimeCode.nHours--;
				} else {
					g_ltc_LtcTimeCode.nHours = 23;
				}
			}
		}
	}

	//FIXME Add support for DF
}

bool LtcGenerator::PitchControl() {
	const auto nPitch = static_cast<uint32_t>(m_fPitchControl * static_cast<float>(m_nPitchTicker)); // / 100;
	const auto r = (nPitch - m_nPitchPrevious);

	m_nPitchPrevious = nPitch;
	m_nPitchTicker++;

	return (r != 0);
}

void LtcGenerator::SetTimeCode(int32_t nSeconds) {
	g_ltc_LtcTimeCode.nHours = static_cast<uint8_t>(nSeconds / 3600);
	nSeconds -= g_ltc_LtcTimeCode.nHours * 3600;
	g_ltc_LtcTimeCode.nMinutes = static_cast<uint8_t>(nSeconds / 60);
	nSeconds -= g_ltc_LtcTimeCode.nMinutes * 60;
	g_ltc_LtcTimeCode.nSeconds = static_cast<uint8_t>(nSeconds);
}

void LtcGenerator::Update() {
	if (m_State != STOPPED) {
		LtcOutputs::Get()->UpdateMidiQuarterFrameMessage(const_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode));
	}

	__DMB();
	if (gv_ltc_bTimeCodeAvailable) {
		gv_ltc_bTimeCodeAvailable = false;

		if (!g_ltc_ptLtcDisabledOutputs.bArtNet) {
			ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct TArtNetTimeCode*>(&g_ltc_LtcTimeCode));
		}

		if (!g_ltc_ptLtcDisabledOutputs.bRtpMidi) {
			RtpMidi::Get()->SendTimeCode(reinterpret_cast<const struct midi::Timecode *>(&g_ltc_LtcTimeCode));
		}

		if (!g_ltc_ptLtcDisabledOutputs.bEtc) {
			LtcEtc::Get()->Send(reinterpret_cast<const struct midi::Timecode *>(&g_ltc_LtcTimeCode));
		}

		LtcOutputs::Get()->Update(static_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode));

		if (__builtin_expect((m_tDirection == LTC_GENERATOR_FORWARD), 1)) {
			if (__builtin_expect((m_tPitch == LTC_GENERATOR_NORMAL), 1)) {
				Increment();
			} else {
				if (m_tPitch == LTC_GENERATOR_FASTER) {
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
			if (__builtin_expect((m_tPitch == LTC_GENERATOR_NORMAL), 1)) {
				Decrement();
			} else {
				if (m_tPitch == LTC_GENERATOR_FASTER) {
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
	}
}

void LtcGenerator::Run() {
	Update();

	HandleButtons();
	HandleUdpRequest();

	if (m_State == STARTED) {
		Hardware::Get()->SetMode(hardware::ledblink::Mode::DATA);
	} else {
		Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
	}
}
