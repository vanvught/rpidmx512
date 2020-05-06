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
#include <assert.h>

#include "h3/ltcgenerator.h"
#include "ltc.h"
#include "timecodeconst.h"

#include "network.h"

#include "c/led.h"

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
#include "tcnet.h"
#include "rtpmidi.h"
#include "h3/ltcsender.h"
#include "display.h"
//
#include "h3/ltcoutputs.h"

#include "debug.h"

#define BUTTON(x)			((m_nButtons >> x) & 0x01)
#define BUTTON_STATE(x)		((m_nButtons & (1 << x)) == (1 << x))

#define BUTTON0_GPIO		GPIO_EXT_22		// PA2 Start
#define BUTTON1_GPIO		GPIO_EXT_15		// PA3 Stop
#define BUTTON2_GPIO		GPIO_EXT_7		// PA6 Resume

#define BUTTONS_MASK		((1 << BUTTON0_GPIO) |  (1 << BUTTON1_GPIO) | (1 << BUTTON2_GPIO))

constexpr char aStart[] = "start";
#define START_LENGTH		(sizeof(aStart) - 1)

constexpr char aStop[] = "stop";
#define STOP_LENGTH			(sizeof(aStop) - 1)

constexpr char aResume[] = "resume";
#define RESUME_LENGTH		(sizeof(aResume) - 1)

constexpr char aRate[] = "rate";
#define RATE_LENGTH			(sizeof(aRate) - 1)

constexpr char aDirection[] = "direction";
#define DIRECTION_LENGTH	(sizeof(aDirection) - 1)

constexpr char aPitch[] = "pitch";
#define PITCH_LENGTH		(sizeof(aPitch) - 1)

enum TUdpPort {
	UDP_PORT = 0x5443
};

// IRQ Timer0
static volatile bool bTimeCodeAvailable;
static struct TLtcDisabledOutputs* s_ptLtcDisabledOutputs;

static struct TLtcTimeCode s_tLtcTimeCode;

static void irq_timer0_handler(uint32_t clo) {
	if (!s_ptLtcDisabledOutputs->bLtc) {
		LtcSender::Get()->SetTimeCode(static_cast<const struct TLtcTimeCode*>(&s_tLtcTimeCode), false);
	}

	bTimeCodeAvailable = true;
}

static int32_t atoi(const char *pBuffer, uint32_t nSize) {
	assert(pBuffer != 0);
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

LtcGenerator *LtcGenerator::s_pThis = 0;

LtcGenerator::LtcGenerator(const struct TLtcTimeCode* pStartLtcTimeCode, const struct TLtcTimeCode* pStopLtcTimeCode, struct TLtcDisabledOutputs *pLtcDisabledOutputs):
	m_pStartLtcTimeCode(const_cast<struct TLtcTimeCode*>(pStartLtcTimeCode)),
	m_pStopLtcTimeCode(const_cast<struct TLtcTimeCode*>(pStopLtcTimeCode)),
	m_nFps(0),
	m_tDirection(LTC_GENERATOR_FORWARD),
	m_fPitchControl(0),
	m_nPitchTicker(1),
	m_nPitchPrevious(0),
	m_tPitch(LTC_GENERATOR_FASTER),
	m_nTimer0Interval(0),
	m_nButtons(0),
	m_nHandle(-1),
	m_nBytesReceived(0),
	m_bIsStarted(false)
{
	assert(pStartLtcTimeCode != 0);
	assert(pStopLtcTimeCode != 0);
	assert(pLtcDisabledOutputs != 0);

	s_pThis = this;

	s_ptLtcDisabledOutputs = pLtcDisabledOutputs;

	bTimeCodeAvailable = false;

	memset(&s_tLtcTimeCode, 0, sizeof(struct TLtcTimeCode));
	s_tLtcTimeCode.nType = pStartLtcTimeCode->nType;

	m_nFps = TimeCodeConst::FPS[pStartLtcTimeCode->nType];
	m_nTimer0Interval = TimeCodeConst::TMR_INTV[pStartLtcTimeCode->nType];

	if (m_pStartLtcTimeCode->nFrames >= m_nFps) {
		m_pStartLtcTimeCode->nFrames = m_nFps - 1;
	}

	if (m_pStopLtcTimeCode->nFrames >= m_nFps) {
		m_pStopLtcTimeCode->nFrames = m_nFps - 1;
	}
}

LtcGenerator::~LtcGenerator(void) {
	Stop();
}

void LtcGenerator::Start(void) {
	DEBUG_ENTRY

	// Buttons
	h3_gpio_fsel(BUTTON0_GPIO, GPIO_FSEL_EINT); // PA2
	h3_gpio_fsel(BUTTON1_GPIO, GPIO_FSEL_EINT);	// PA3
	h3_gpio_fsel(BUTTON2_GPIO, GPIO_FSEL_EINT);	// PA6

	uint32_t value = H3_PIO_PORTA->PUL0;
	value &= ~((GPIO_PULL_MASK << 4) | (GPIO_PULL_MASK << 6) | (GPIO_PULL_MASK << 12));
	value |= (GPIO_PULL_UP << 4) | (GPIO_PULL_UP << 6) | (GPIO_PULL_UP << 12);
	H3_PIO_PORTA->PUL0 = value;

	value = H3_PIO_PA_INT->CFG0;
	value &= ~((GPIO_INT_CFG_MASK << 8) | (GPIO_INT_CFG_MASK << 12) | (GPIO_INT_CFG_MASK << 24));
	value |= (GPIO_INT_CFG_NEG_EDGE << 8) | (GPIO_INT_CFG_NEG_EDGE << 12) | (GPIO_INT_CFG_NEG_EDGE << 24);
	H3_PIO_PA_INT->CFG0 = value;

	H3_PIO_PA_INT->CTL |= BUTTONS_MASK;
	H3_PIO_PA_INT->STA = BUTTONS_MASK;
	H3_PIO_PA_INT->DEB = (0x0 << 0) | (0x7 << 4);

	// UDP Request
	m_nHandle = Network::Get()->Begin(UDP_PORT);
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

	led_set_ticks_per_second(LED_TICKS_NO_DATA);

	DEBUG_EXIT
}

void LtcGenerator::Stop(void) {
	DEBUG_ENTRY

	__disable_irq();
	irq_timer_set(IRQ_TIMER_0, 0);

	m_nHandle = Network::Get()->End(UDP_PORT);

	DEBUG_EXIT
}

void LtcGenerator::ActionStart(void) {
	DEBUG_ENTRY

	if(m_bIsStarted) {
		return;
	}

	m_bIsStarted = true;

	memcpy(&s_tLtcTimeCode, m_pStartLtcTimeCode, sizeof(struct TLtcTimeCode));

	LtcOutputs::Get()->ResetTimeCodeTypePrevious();

	DEBUG_EXIT
}

void LtcGenerator::ActionStop(void) {
	DEBUG_ENTRY

	m_bIsStarted = false;

	DEBUG_EXIT
}

void LtcGenerator::ActionResume(void) {
	DEBUG_ENTRY

	if (!m_bIsStarted) {
		m_bIsStarted = true;
	}

	DEBUG_EXIT
}

void LtcGenerator::ActionSetStart(const char *pTimeCode) {
	DEBUG_ENTRY

	Ltc::ParseTimeCode(pTimeCode, m_nFps, m_pStartLtcTimeCode);

	DEBUG_EXIT
}

void LtcGenerator::ActionSetStop(const char *pTimeCode) {
	DEBUG_ENTRY

	Ltc::ParseTimeCode(pTimeCode, m_nFps, m_pStopLtcTimeCode);

	DEBUG_EXIT
}

void LtcGenerator::ActionSetRate(const char *pTimeCodeRate) {
	DEBUG_ENTRY

	uint8_t nFps;
	TTimecodeTypes tType;

	if ((!m_bIsStarted) && (Ltc::ParseTimeCodeRate(pTimeCodeRate, nFps, tType))) {
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

void LtcGenerator::ActionSetPitch(const char *pTimeCodePitch, uint32_t nSize) {
	DEBUG_ENTRY

	debug_dump(pTimeCodePitch, nSize);

	const float f = static_cast<float>(atoi(pTimeCodePitch, nSize)) / 100;

	DEBUG_PRINTF("f=%f", f);

	ActionSetPitch(f);

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

void LtcGenerator::HandleButtons(void) {
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

void LtcGenerator::HandleUdpRequest(void) {
	uint32_t nIPAddressFrom;
	uint16_t nForeignPort;

	m_nBytesReceived = Network::Get()->RecvFrom(m_nHandle, &m_Buffer, sizeof(m_Buffer), &nIPAddressFrom, &nForeignPort);

	if (__builtin_expect((m_nBytesReceived < 8), 1)) {
		return;
	}

	if (__builtin_expect((memcmp("ltc!", m_Buffer, 4) != 0), 0)) {
		return;
	}

	if (m_Buffer[m_nBytesReceived - 1] == '\n') {
		DEBUG_PUTS("\'\\n\'");
		m_nBytesReceived--;
	}

	if (memcmp(&m_Buffer[4], aStart, START_LENGTH) == 0) {
		if (m_nBytesReceived == (4 + START_LENGTH)) {
			ActionStart();
		} else if ((m_nBytesReceived == (4 + START_LENGTH + 1 + TC_CODE_MAX_LENGTH)) && (m_Buffer[4 + START_LENGTH] == '#')){
			ActionSetStart(&m_Buffer[(4 + START_LENGTH + 1)]);
		} else if ((m_nBytesReceived == (4 + START_LENGTH + 1 + TC_CODE_MAX_LENGTH)) && (m_Buffer[4 + START_LENGTH] == '!')){
			ActionSetStart(&m_Buffer[(4 + START_LENGTH + 1)]);
			ActionStop();
			ActionStart();
		} else if ((m_nBytesReceived == (4 + START_LENGTH + 1 + TC_CODE_MAX_LENGTH)) && (m_Buffer[4 + START_LENGTH] == '@')){
			ActionGoto(&m_Buffer[(4 + START_LENGTH + 1)]);
		} else {
			DEBUG_PUTS("Invalid !start command");
		}
	} else if (memcmp(&m_Buffer[4], aStop, STOP_LENGTH) == 0) {
		if (m_nBytesReceived == (4 + STOP_LENGTH)) {
			ActionStop();
		} else if ((m_nBytesReceived == (4 + STOP_LENGTH + 1 + TC_CODE_MAX_LENGTH))  && (m_Buffer[4 + STOP_LENGTH] == '#')) {
			ActionSetStop(&m_Buffer[(4 + STOP_LENGTH + 1)]);
		} else {
			DEBUG_PUTS("Invalid !stop command");
		}
	} else if (memcmp(&m_Buffer[4], aResume, RESUME_LENGTH) == 0) {
		ActionResume();
	} else if (memcmp(&m_Buffer[4], aRate, RATE_LENGTH) == 0) {
		if ((m_nBytesReceived == (4 + RATE_LENGTH + 1 + TC_RATE_MAX_LENGTH)) && (m_Buffer[4 + RATE_LENGTH] == '#')) {
			ActionSetRate(&m_Buffer[(4 + RATE_LENGTH + 1)]);
		}
	} else if (memcmp(&m_Buffer[4], aDirection, DIRECTION_LENGTH) == 0) {
		if ((static_cast<uint32_t>(m_nBytesReceived) <= (4 + DIRECTION_LENGTH + 1 + 8)) && (m_Buffer[4 + DIRECTION_LENGTH] == '#')) {
			ActionSetDirection(&m_Buffer[(4 + DIRECTION_LENGTH + 1)]);
		}
	} else if (memcmp(&m_Buffer[4], aPitch, PITCH_LENGTH) == 0) {
		if ((static_cast<uint32_t>(m_nBytesReceived) <= (4 + PITCH_LENGTH + 1 + 4)) && (m_Buffer[4 + PITCH_LENGTH] == '#')) {
			ActionSetPitch(&m_Buffer[(4 + PITCH_LENGTH + 1)], m_nBytesReceived - (4 + PITCH_LENGTH + 1));
		}
	} else {
		DEBUG_PUTS("Invalid command");
	}
}

void LtcGenerator::Increment(void) {

	if (__builtin_expect((memcmp(&s_tLtcTimeCode, m_pStopLtcTimeCode, sizeof(struct TLtcTimeCode)) == 0), 0)) {
		return;
	}

	if (__builtin_expect((!m_bIsStarted), 0)) {
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

void LtcGenerator::Decrement(void) {

	if (__builtin_expect((memcmp(&s_tLtcTimeCode, m_pStopLtcTimeCode, sizeof(struct TLtcTimeCode)) == 0), 0)) {
		return;
	}

	if (__builtin_expect((!m_bIsStarted), 0)) {
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

bool LtcGenerator::PitchControl(void) {
	const uint32_t p = (m_fPitchControl * m_nPitchTicker); // / 100;

	const uint32_t r = (p - m_nPitchPrevious);

	m_nPitchPrevious = p;

	m_nPitchTicker++;

	return (r != 0);
}

void LtcGenerator::Update(void) {
	if (m_bIsStarted) {
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

void LtcGenerator::Print(void) {
	printf("Internal\n");
	printf(" %s\n", Ltc::GetType(static_cast<TTimecodeTypes>(m_pStartLtcTimeCode->nType)));
	printf(" Start : %.2d.%.2d.%.2d:%.2d\n", m_pStartLtcTimeCode->nHours, m_pStartLtcTimeCode->nMinutes, m_pStartLtcTimeCode->nSeconds, m_pStartLtcTimeCode->nFrames);
	printf(" Stop  : %.2d.%.2d.%.2d:%.2d\n", m_pStopLtcTimeCode->nHours, m_pStopLtcTimeCode->nMinutes, m_pStopLtcTimeCode->nSeconds, m_pStopLtcTimeCode->nFrames);
}

void LtcGenerator::Run(void) {
	Update();

	HandleButtons();
	HandleUdpRequest();

	if (m_bIsStarted) {
		led_set_ticks_per_second(LED_TICKS_DATA);
	} else {
		led_set_ticks_per_second(LED_TICKS_NO_DATA);
	}
}
