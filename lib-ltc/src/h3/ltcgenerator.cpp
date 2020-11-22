/**
 * @file ltcgenerator.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <cassert>

#include "h3/ltcgenerator.h"
#include "ltc.h"
#include "timecodeconst.h"

#include "network.h"

#include "arm/arm.h"
#include "arm/synchronize.h"

#include "h3.h"
#include "h3_timer.h"
#include "irq_timer.h"

// Buttons
#include "h3_board.h"
#include "h3_gpio.h"

// Output
#include "artnetnode.h"
#include "rtpmidi.h"
#include "h3/ltcsender.h"
#include "h3/ltcoutputs.h"

#include "debug.h"

#define BUTTON(x)			((m_nButtons >> x) & 0x01)
#define BUTTON_STATE(x)		((m_nButtons & (1 << x)) == (1 << x))

#define BUTTON0_GPIO		GPIO_EXT_22		// PA2 Start
#define BUTTON1_GPIO		GPIO_EXT_15		// PA3 Stop
#define BUTTON2_GPIO		GPIO_EXT_7		// PA6 Resume

#define BUTTONS_MASK		((1 << BUTTON0_GPIO) |  (1 << BUTTON1_GPIO) | (1 << BUTTON2_GPIO))

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

// IRQ Timer0
static volatile bool bTimeCodeAvailable;
static struct TLtcDisabledOutputs* s_ptLtcDisabledOutputs;

static struct TLtcTimeCode s_tLtcTimeCode;

static void irq_timer0_handler(__attribute__((unused)) uint32_t clo) {
	if (!s_ptLtcDisabledOutputs->bLtc) {
		LtcSender::Get()->SetTimeCode(static_cast<const struct TLtcTimeCode*>(&s_tLtcTimeCode), false);
	}

	bTimeCodeAvailable = true;
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

LtcGenerator *LtcGenerator::s_pThis = nullptr;

LtcGenerator::LtcGenerator(const struct TLtcTimeCode* pStartLtcTimeCode, const struct TLtcTimeCode* pStopLtcTimeCode, struct TLtcDisabledOutputs *pLtcDisabledOutputs, bool bSkipFree):
	m_pStartLtcTimeCode(const_cast<struct TLtcTimeCode*>(pStartLtcTimeCode)),
	m_nStartSeconds(GetSeconds(*m_pStartLtcTimeCode)),
	m_pStopLtcTimeCode(const_cast<struct TLtcTimeCode*>(pStopLtcTimeCode)),
	m_nStopSeconds(GetSeconds(*m_pStopLtcTimeCode)),
	m_bSkipFree(bSkipFree)
{
	assert(pStartLtcTimeCode != nullptr);
	assert(pStopLtcTimeCode != nullptr);
	assert(pLtcDisabledOutputs != nullptr);

	s_pThis = this;

	s_ptLtcDisabledOutputs = pLtcDisabledOutputs;

	bTimeCodeAvailable = false;

	memcpy(&s_tLtcTimeCode, pStartLtcTimeCode, sizeof(struct TLtcTimeCode));

	m_nFps = TimeCodeConst::FPS[pStartLtcTimeCode->nType];
	m_nTimer0Interval = TimeCodeConst::TMR_INTV[pStartLtcTimeCode->nType];

	if (m_pStartLtcTimeCode->nFrames >= m_nFps) {
		m_pStartLtcTimeCode->nFrames = m_nFps - 1;
	}

	if (m_pStopLtcTimeCode->nFrames >= m_nFps) {
		m_pStopLtcTimeCode->nFrames = m_nFps - 1;
	}
}

LtcGenerator::~LtcGenerator() {
	Stop();
}

void LtcGenerator::Start() {
	DEBUG_ENTRY

	// Buttons
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

	// UDP Request
	m_nHandle = Network::Get()->Begin(udp::PORT);
	assert(m_nHandle != -1);

	// Generator
	irq_timer_init();
	irq_timer_set(IRQ_TIMER_0, static_cast<thunk_irq_timer_t>(irq_timer0_handler));

	H3_TIMER->TMR0_INTV = m_nTimer0Interval;
	H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
	H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

	LtcOutputs::Get()->Init();

	if (!s_ptLtcDisabledOutputs->bLtc) {
		LtcSender::Get()->SetTimeCode(const_cast<const struct TLtcTimeCode*>(&s_tLtcTimeCode), false);
	}

	if (!s_ptLtcDisabledOutputs->bArtNet) {
		ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct TArtNetTimeCode*>(&s_tLtcTimeCode));
	}

	if (!s_ptLtcDisabledOutputs->bRtpMidi) {
		RtpMidi::Get()->SendTimeCode(reinterpret_cast<const struct _midi_send_tc*>(&s_tLtcTimeCode));
	}

	LtcOutputs::Get()->Update(const_cast<const struct TLtcTimeCode*>(&s_tLtcTimeCode));

	LedBlink::Get()->SetFrequency(ltc::led_frequency::NO_DATA);

	DEBUG_EXIT
}

void LtcGenerator::Stop() {
	DEBUG_ENTRY

	__disable_irq();
	irq_timer_set(IRQ_TIMER_0, nullptr);

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

void LtcGenerator::ActionReset() {
	DEBUG_ENTRY

	memcpy(&s_tLtcTimeCode, m_pStartLtcTimeCode, sizeof(struct TLtcTimeCode));

	LtcOutputs::Get()->ResetTimeCodeTypePrevious();

	DEBUG_EXIT
}

void LtcGenerator::ActionSetStart(const char *pTimeCode) {
	DEBUG_ENTRY

	Ltc::ParseTimeCode(pTimeCode, m_nFps, m_pStartLtcTimeCode);

	m_nStartSeconds = GetSeconds(*m_pStartLtcTimeCode);

	DEBUG_EXIT
}

void LtcGenerator::ActionSetStop(const char *pTimeCode) {
	DEBUG_ENTRY

	Ltc::ParseTimeCode(pTimeCode, m_nFps, m_pStopLtcTimeCode);

	m_nStopSeconds = GetSeconds(*m_pStopLtcTimeCode);

	DEBUG_EXIT
}

void LtcGenerator::ActionSetRate(const char *pTimeCodeRate) {
	DEBUG_ENTRY

	uint8_t nFps;
	ltc::type tType;

	if ((m_State == STOPPED) && (Ltc::ParseTimeCodeRate(pTimeCodeRate, nFps, tType))) {
		if (nFps != m_nFps) {
			m_nFps = nFps;
			//
			s_tLtcTimeCode.nType = tType;
			//
			m_pStartLtcTimeCode->nType = tType;
			if (m_pStartLtcTimeCode->nFrames >= m_nFps) {
				m_pStartLtcTimeCode->nFrames = m_nFps - 1;
			}
			m_pStopLtcTimeCode->nType = tType;
			if (m_pStopLtcTimeCode->nFrames >= m_nFps) {
				m_pStopLtcTimeCode->nFrames = m_nFps - 1;
			}
			//
			//
			m_nTimer0Interval = TimeCodeConst::TMR_INTV[tType];
			H3_TIMER->TMR0_INTV = m_nTimer0Interval;
			H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
			H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
			//
			if (!s_ptLtcDisabledOutputs->bLtc) {
				LtcSender::Get()->SetTimeCode(const_cast<const struct TLtcTimeCode*>(&s_tLtcTimeCode), false);
			}

			if (!s_ptLtcDisabledOutputs->bArtNet) {
				ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct TArtNetTimeCode*>(&s_tLtcTimeCode));
			}

			if (!s_ptLtcDisabledOutputs->bRtpMidi) {
				RtpMidi::Get()->SendTimeCode(reinterpret_cast<const struct _midi_send_tc*>(&s_tLtcTimeCode));
			}

			LtcOutputs::Get()->Update(const_cast<const struct TLtcTimeCode*>(&s_tLtcTimeCode));
		}
	}

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

void LtcGenerator::ActionForward(int32_t nSeconds) {
	DEBUG_ENTRY

	if (m_State == STARTED) {
		DEBUG_EXIT
		return;
	}

	if (m_State == LIMIT) {
		m_State = STARTED;
	}

	const auto s = GetSeconds(s_tLtcTimeCode) + nSeconds;
	constexpr auto nMaxSeconds = ((23 * 60) + 59) * 60 + 59;
	const auto nLimit = m_bSkipFree ? nMaxSeconds : m_nStopSeconds;

	if (s <= nLimit) {
		SetTimeCode(s);
	} else {
		memcpy(&s_tLtcTimeCode, m_pStopLtcTimeCode, sizeof(struct TLtcTimeCode));
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

	const auto s = GetSeconds(s_tLtcTimeCode) - nSeconds;
	const auto nLimit = m_bSkipFree ? 0 : m_nStartSeconds;

	if (s >= nLimit) {
		SetTimeCode(s);
	} else {
		memcpy(&s_tLtcTimeCode, m_pStartLtcTimeCode, sizeof(struct TLtcTimeCode));
	}

	DEBUG_EXIT
}

void LtcGenerator::SetPitch(const char *pTimeCodePitch, uint32_t nSize) {
	DEBUG_ENTRY
	debug_dump(const_cast<char*>(pTimeCodePitch), nSize);

	const auto f = static_cast<float>(atoi(pTimeCodePitch, nSize)) / 100;

	DEBUG_PRINTF("f=%f", f);

	ActionSetPitch(f);

	DEBUG_EXIT
}

void LtcGenerator::SetSkip(const char *pSeconds, uint32_t nSize, TLtcGeneratorDirection tDirection) {
	DEBUG_ENTRY
	debug_dump(const_cast<char*>(pSeconds), nSize);

	const auto nSeconds = atoi(pSeconds, nSize);

	DEBUG_PRINTF("nSeconds=%d", nSeconds);

	if (tDirection == LTC_GENERATOR_FORWARD) {
		ActionForward(nSeconds);
	} else {
		ActionBackward(nSeconds);
	}

	DEBUG_EXIT
}

int32_t LtcGenerator::GetSeconds(const TLtcTimeCode &timecode) {
	int32_t nSeconds = timecode.nHours;
	nSeconds *= 60;
	nSeconds += timecode.nMinutes;
	nSeconds *= 60;
	nSeconds += timecode.nSeconds;

	return nSeconds;
}

void LtcGenerator::SetTimeCode(int32_t nSeconds) {
	s_tLtcTimeCode.nHours = static_cast<uint8_t>(nSeconds / 3600);
	nSeconds -= s_tLtcTimeCode.nHours * 3600;
	s_tLtcTimeCode.nMinutes = static_cast<uint8_t>(nSeconds / 60);
	nSeconds -= s_tLtcTimeCode.nMinutes * 60;
	s_tLtcTimeCode.nSeconds = static_cast<uint8_t>(nSeconds);
}

void LtcGenerator::HandleButtons() {
	m_nButtons = H3_PIO_PA_INT->STA & BUTTONS_MASK;

	if (__builtin_expect((m_nButtons != 0), 0)) {
		H3_PIO_PA_INT->STA = BUTTONS_MASK;

		DEBUG_PRINTF("%d-%d-%d", BUTTON(BUTTON0_GPIO), BUTTON(BUTTON1_GPIO), BUTTON(BUTTON2_GPIO));

		if (BUTTON_STATE(BUTTON0_GPIO)) {
			ActionStart();
		} else if (BUTTON_STATE(BUTTON1_GPIO)) {
			ActionStop();
		} else if (BUTTON_STATE(BUTTON2_GPIO)) {
			ActionResume();
		}
	}
}

void LtcGenerator::HandleRequest(void *pBuffer, uint32_t nBufferLength) {
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

		if (m_nBytesReceived == (4 + length::START + 1 + TC_CODE_MAX_LENGTH)) {
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

		if ((m_nBytesReceived == (4 + length::STOP + 1 + TC_CODE_MAX_LENGTH))  && (m_Buffer[4 + length::STOP] == '#')) {
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

	if (m_nBytesReceived == (4 + length::RATE + TC_RATE_MAX_LENGTH)) {
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

void LtcGenerator::Increment() {

	if (__builtin_expect((memcmp(&s_tLtcTimeCode, m_pStopLtcTimeCode, sizeof(struct TLtcTimeCode)) == 0), 0)) {
		if (m_State == STARTED) {
			m_State = LIMIT;
		}
		return;
	}

	if (__builtin_expect((m_State == STOPPED), 0)) {
		return;
	}

	s_tLtcTimeCode.nFrames++;
	if (m_nFps == s_tLtcTimeCode.nFrames) {
		s_tLtcTimeCode.nFrames = 0;

		s_tLtcTimeCode.nSeconds++;
		if (s_tLtcTimeCode.nSeconds == 60) {
			s_tLtcTimeCode.nSeconds = 0;

			s_tLtcTimeCode.nMinutes++;
			if (s_tLtcTimeCode.nMinutes == 60) {
				s_tLtcTimeCode.nMinutes = 0;

				s_tLtcTimeCode.nHours++;
				if (s_tLtcTimeCode.nHours == 24) {
					s_tLtcTimeCode.nHours = 0;
				}
			}
		}
	}

	//FIXME Add support for DF
}

void LtcGenerator::Decrement() {

	if (__builtin_expect((memcmp(&s_tLtcTimeCode, m_pStartLtcTimeCode, sizeof(struct TLtcTimeCode)) == 0), 0)) {
		if (m_State == STARTED) {
			m_State = LIMIT;
		}
		return;
	}

	if (__builtin_expect((m_State == STOPPED), 0)) {
		return;
	}

	if (s_tLtcTimeCode.nFrames > 0) {
		s_tLtcTimeCode.nFrames--;
	} else {
		s_tLtcTimeCode.nFrames = m_nFps - 1;
	}

	if (s_tLtcTimeCode.nFrames == m_nFps - 1) {
		if (s_tLtcTimeCode.nSeconds > 0) {
			s_tLtcTimeCode.nSeconds--;
		} else {
			s_tLtcTimeCode.nSeconds = 59;
		}

		if (s_tLtcTimeCode.nSeconds == 59) {
			if (s_tLtcTimeCode.nMinutes > 0) {
				s_tLtcTimeCode.nMinutes--;
			} else {
				s_tLtcTimeCode.nMinutes = 59;
			}

			if (s_tLtcTimeCode.nMinutes == 59) {
				if (s_tLtcTimeCode.nHours > 0) {
					s_tLtcTimeCode.nHours--;
				} else {
					s_tLtcTimeCode.nHours = 23;
				}
			}
		}
	}

	//FIXME Add support for DF
}

bool LtcGenerator::PitchControl() {
	const uint32_t p = (m_fPitchControl * m_nPitchTicker); // / 100;
	const uint32_t r = (p - m_nPitchPrevious);

	m_nPitchPrevious = p;
	m_nPitchTicker++;

	return (r != 0);
}

void LtcGenerator::Update() {
	if (m_State != STOPPED) {
		LtcOutputs::Get()->UpdateMidiQuarterFrameMessage(const_cast<const struct TLtcTimeCode*>(&s_tLtcTimeCode));
	}

	dmb();
	if (bTimeCodeAvailable) {
		bTimeCodeAvailable = false;

		if (!s_ptLtcDisabledOutputs->bArtNet) {
			ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct TArtNetTimeCode*>(&s_tLtcTimeCode));
		}

		if (!s_ptLtcDisabledOutputs->bRtpMidi) {
			RtpMidi::Get()->SendTimeCode(reinterpret_cast<const struct _midi_send_tc*>(&s_tLtcTimeCode));
		}

		LtcOutputs::Get()->Update(static_cast<const struct TLtcTimeCode*>(&s_tLtcTimeCode));

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

void LtcGenerator::Print() {
	printf("Internal\n");
	printf(" %s\n", Ltc::GetType(static_cast<ltc::type>(m_pStartLtcTimeCode->nType)));
	printf(" Start : %.2d.%.2d.%.2d:%.2d\n", m_pStartLtcTimeCode->nHours, m_pStartLtcTimeCode->nMinutes, m_pStartLtcTimeCode->nSeconds, m_pStartLtcTimeCode->nFrames);
	printf(" Stop  : %.2d.%.2d.%.2d:%.2d\n", m_pStopLtcTimeCode->nHours, m_pStopLtcTimeCode->nMinutes, m_pStopLtcTimeCode->nSeconds, m_pStopLtcTimeCode->nFrames);
}

void LtcGenerator::Run() {
	Update();

	HandleButtons();
	HandleUdpRequest();

	if (m_State == STARTED) {
		LedBlink::Get()->SetFrequency(ltc::led_frequency::DATA);
	} else {
		LedBlink::Get()->SetFrequency(ltc::led_frequency::NO_DATA);
	}
}
