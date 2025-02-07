/**
 * @file ltcdisplayrgb.cpp
 */
/*
 * Copyright (C) 2019-2020 by hippy mailto:dmxout@gmail.com
 * Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (DEBUG_LTCDISPLAYRGB)
# undef NDEBUG
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <cassert>

#include "ltcdisplayrgb.h"

#include "ltc.h"

#include "hardware.h"
#include "network.h"

#include "ltcdisplayws28xx7segment.h"
#include "ltcdisplayws28xxmatrix.h"

#include "ltcdisplayrgbpanel.h"

#if !defined (CONFIG_LTC_DISABLE_WS28XX)
# include "pixeltype.h"
#endif

#include "softwaretimers.h"

#include "debug.h"

namespace ltcdisplayrgb {
namespace rgb {
static constexpr char PATH[] = "rgb";
static constexpr auto LENGTH = sizeof(PATH) - 1;
static constexpr auto HEX_SIZE = 7; // 1 byte index followed by 6 bytes hex RGB
}
namespace master {
static constexpr char PATH[] = "master";
static constexpr auto LENGTH = sizeof(PATH) - 1;
static constexpr auto HEX_SIZE = 2;
}
namespace showmsg {
static constexpr char PATH[] = "showmsg";
static constexpr auto LENGTH = sizeof(PATH) - 1;
}
namespace udp {
static constexpr auto PORT = 0x2812;
}
}  // namespace ltcdisplayrgb

using namespace ltcdisplayrgb;

static TimerHandle_t s_nTimerId = TIMER_ID_NONE;
static bool m_bShowMsg;

static void message_timer([[maybe_unused]] TimerHandle_t nHandle) {
	m_bShowMsg = false;
	SoftwareTimerDelete(s_nTimerId);
}

LtcDisplayRgb::LtcDisplayRgb(Type tRgbType, WS28xxType tWS28xxType) : m_tDisplayRgbType(tRgbType), m_tDisplayRgbWS28xxType(tWS28xxType) {
	DEBUG_ENTRY
	assert(s_pThis == nullptr);
	s_pThis = this;

	m_aColour[static_cast<uint32_t>(ColourIndex::TIME)] = Defaults::COLOUR_TIME;
	m_aColour[static_cast<uint32_t>(ColourIndex::COLON)] = Defaults::COLOUR_COLON;
	m_aColour[static_cast<uint32_t>(ColourIndex::MESSAGE)] = Defaults::COLOUR_MESSAGE;
	m_aColour[static_cast<uint32_t>(ColourIndex::FPS)] = Defaults::COLOUR_FPS;
	m_aColour[static_cast<uint32_t>(ColourIndex::INFO)] = Defaults::COLOUR_INFO;
	m_aColour[static_cast<uint32_t>(ColourIndex::SOURCE)] = Defaults::COLOUR_SOURCE;

	DEBUG_EXIT
}

LtcDisplayRgb::~LtcDisplayRgb() {
	DEBUG_ENTRY

	assert(m_pLtcDisplayRgbSet != nullptr);
	delete m_pLtcDisplayRgbSet;
	m_pLtcDisplayRgbSet = nullptr;

	DEBUG_EXIT
}

void LtcDisplayRgb::Init(pixel::Type type) {
	DEBUG_ENTRY

	m_PixelType = type;

	if (m_tDisplayRgbType == Type::RGBPANEL) {
		m_pLtcDisplayRgbSet = new LtcDisplayRgbPanel;

		assert(m_pLtcDisplayRgbSet != nullptr);
		m_pLtcDisplayRgbSet->Init();
	} else {

		if (m_tDisplayRgbWS28xxType == WS28xxType::SEGMENT) {
			m_pLtcDisplayRgbSet = new LtcDisplayWS28xx7Segment(type, m_PixelMap);
		} else {
			m_pLtcDisplayRgbSet = new LtcDisplayWS28xxMatrix(type, m_PixelMap);
		}

		assert(m_pLtcDisplayRgbSet != nullptr);
	}

	for (uint32_t nIndex = 0; nIndex < static_cast<uint32_t>(ColourIndex::LAST); nIndex++) {
		SetRGB(m_aColour[nIndex], static_cast<ColourIndex>(nIndex));
	}

	assert(m_nHandle == -1);
	m_nHandle = Network::Get()->Begin(udp::PORT, StaticCallbackFunction);
	assert(m_nHandle != -1);

	DEBUG_EXIT
}

void LtcDisplayRgb::Show(const char *pTimecode) {
	if (m_pLtcDisplayRgbSet == nullptr) {
		return;
	}

	if (__builtin_expect((m_bShowMsg), 0)) {
		ShowMessage();
		return;
	}

	struct Colours tColoursColons;

	if (m_tColonBlinkMode != ColonBlinkMode::OFF) {
		const uint32_t nMillis = Hardware::Get()->Millis();

		if (m_nSecondsPrevious != pTimecode[ltc::timecode::index::SECONDS_UNITS]) { // seconds have changed
			m_nSecondsPrevious = pTimecode[ltc::timecode::index::SECONDS_UNITS];
			m_nColonBlinkMillis = nMillis;

			tColoursColons.nRed = 0;
			tColoursColons.nGreen = 0;
			tColoursColons.nBlue = 0;
		} else if (nMillis - m_nColonBlinkMillis < 1000) {
			uint32_t nMaster;

			if (m_tColonBlinkMode == ColonBlinkMode::DOWN) {
				nMaster = 255 - ((nMillis - m_nColonBlinkMillis) * 255 / 1000);
			} else {
				nMaster = ((nMillis - m_nColonBlinkMillis) * 255 / 1000);
			}

			if (!(m_nMaster == 0 || m_nMaster == 255)) {
				nMaster = (m_nMaster * nMaster) / 255 ;
			}

			tColoursColons.nRed = static_cast<uint8_t>((nMaster * m_tColoursColons.nRed) / 255);
			tColoursColons.nGreen = static_cast<uint8_t>((nMaster *  m_tColoursColons.nGreen) / 255);
			tColoursColons.nBlue = static_cast<uint8_t>((nMaster * m_tColoursColons.nBlue) / 255);
		} else {
			if (!(m_nMaster == 0 || m_nMaster == 255)) {
				tColoursColons.nRed = static_cast<uint8_t>((m_nMaster * m_tColoursColons.nRed) / 255);
				tColoursColons.nGreen = static_cast<uint8_t>((m_nMaster * m_tColoursColons.nGreen) / 255);
				tColoursColons.nBlue = static_cast<uint8_t>((m_nMaster * m_tColoursColons.nBlue) / 255);
			} else {
				tColoursColons.nRed = m_tColoursColons.nRed;
				tColoursColons.nGreen = m_tColoursColons.nGreen;
				tColoursColons.nBlue = m_tColoursColons.nBlue;
			}
		}
	} else {
		if (!(m_nMaster == 0 || m_nMaster == 255)) {
			tColoursColons.nRed = static_cast<uint8_t>((m_nMaster * m_tColoursColons.nRed) / 255);
			tColoursColons.nGreen = static_cast<uint8_t>((m_nMaster * m_tColoursColons.nGreen) / 255);
			tColoursColons.nBlue = static_cast<uint8_t>((m_nMaster * m_tColoursColons.nBlue) / 255);
		} else {
			tColoursColons.nRed = m_tColoursColons.nRed;
			tColoursColons.nGreen = m_tColoursColons.nGreen;
			tColoursColons.nBlue = m_tColoursColons.nBlue;
		}
	}

	struct Colours tColours;

	if (!(m_nMaster == 0 || m_nMaster == 255)) {
		tColours.nRed = static_cast<uint8_t>((m_nMaster * m_tColoursTime.nRed) / 255);
		tColours.nGreen = static_cast<uint8_t>((m_nMaster * m_tColoursTime.nGreen) / 255);
		tColours.nBlue = static_cast<uint8_t>((m_nMaster * m_tColoursTime.nBlue) / 255);
	} else {
		tColours.nRed = m_tColoursTime.nRed;
		tColours.nGreen = m_tColoursTime.nGreen;
		tColours.nBlue = m_tColoursTime.nBlue;
	}

	m_pLtcDisplayRgbSet->Show(pTimecode, tColours, tColoursColons);
}

void LtcDisplayRgb::ShowSysTime(const char *pSystemTime) {
	if (m_pLtcDisplayRgbSet == nullptr) {
		return;
	}

	if (__builtin_expect((m_bShowMsg), 0)) {
		ShowMessage();
		return;
	}

	struct Colours tColours;
	struct Colours tColoursColons;

	if (!(m_nMaster == 0 || m_nMaster == 255)) {
		tColours.nRed = static_cast<uint8_t>((m_nMaster * m_tColoursTime.nRed) / 255);
		tColours.nGreen = static_cast<uint8_t>((m_nMaster * m_tColoursTime.nGreen) / 255);
		tColours.nBlue = static_cast<uint8_t>((m_nMaster * m_tColoursTime.nBlue) / 255);
		//
		tColoursColons.nRed = static_cast<uint8_t>((m_nMaster * m_tColoursColons.nRed) / 255);
		tColoursColons.nGreen = static_cast<uint8_t>((m_nMaster * m_tColoursColons.nGreen) / 255);
		tColoursColons.nBlue = static_cast<uint8_t>((m_nMaster * m_tColoursColons.nBlue) / 255);
	} else {
		tColours.nRed = m_tColoursTime.nRed;
		tColours.nGreen = m_tColoursTime.nGreen;
		tColours.nBlue = m_tColoursTime.nBlue;
		//
		tColoursColons.nRed = m_tColoursColons.nRed;
		tColoursColons.nGreen = m_tColoursColons.nGreen;
		tColoursColons.nBlue = m_tColoursColons.nBlue;
	}

	m_pLtcDisplayRgbSet->ShowSysTime(pSystemTime, tColours, tColoursColons);
}

void LtcDisplayRgb::SetMessage(const char *pMessage, uint32_t nSize) {
	assert(pMessage != nullptr);

	uint32_t i;
	const char *pSrc = pMessage;
	char *pDst = m_aMessage;

	for (i = 0; i < std::min(nSize, static_cast<uint32_t>(sizeof(m_aMessage))); i++) {
		*pDst++ = *pSrc++;
	}

	for (; i < sizeof(m_aMessage); i++) {
		*pDst++ = ' ';
	}

	m_nMsgTimer = Hardware::Get()->Millis();

	if (s_nTimerId == TIMER_ID_NONE) {
		s_nTimerId = SoftwareTimerAdd(MESSAGE_TIME_MS, message_timer);
	} else {
		SoftwareTimerChange(s_nTimerId, MESSAGE_TIME_MS);
	}

	m_bShowMsg = true;
}

void LtcDisplayRgb::ShowMessage() {
	if (m_pLtcDisplayRgbSet == nullptr) {
		return;
	}

	struct Colours tColours;

	const auto nMillis = Hardware::Get()->Millis();

	tColours.nRed = static_cast<uint8_t>(((nMillis - m_nMsgTimer) * m_tColoursMessage.nRed) / MESSAGE_TIME_MS);
	tColours.nGreen = static_cast<uint8_t>(((nMillis - m_nMsgTimer) * m_tColoursMessage.nGreen) / MESSAGE_TIME_MS);
	tColours.nBlue = static_cast<uint8_t>(((nMillis - m_nMsgTimer) * m_tColoursMessage.nBlue) / MESSAGE_TIME_MS);

	m_pLtcDisplayRgbSet->ShowMessage(m_aMessage, tColours);
}

void LtcDisplayRgb::ShowFPS(ltc::Type tTimeCodeType) {
	if (m_pLtcDisplayRgbSet == nullptr) {
		return;
	}

	m_pLtcDisplayRgbSet->ShowFPS(tTimeCodeType, m_tColoursFPS);
}

void LtcDisplayRgb::ShowInfo(const char *pInfo) {
	if (m_pLtcDisplayRgbSet == nullptr) {
		return;
	}

	m_pLtcDisplayRgbSet->ShowInfo(pInfo, static_cast<uint16_t>(strlen(pInfo)), m_tColoursInfo);
}

void LtcDisplayRgb::ShowSource(ltc::Source tSource) {
	if (m_pLtcDisplayRgbSet == nullptr) {
		return;
	}

	m_pLtcDisplayRgbSet->ShowSource(tSource, m_tColoursSource);
}

void LtcDisplayRgb::WriteChar(uint8_t nChar, uint8_t nPos) {
	if (m_pLtcDisplayRgbSet == nullptr) {
		return;
	}

	m_pLtcDisplayRgbSet->WriteChar(nChar, nPos, m_tColoursInfo);
}

/**
 * @brief Processes an incoming UDP packet.
 *
 * @param pBuffer Pointer to the packet buffer.
 * @param nSize Size of the packet buffer.
 * @param nFromIp IP address of the sender.
 * @param nFromPort Port number of the sender.
 */
void LtcDisplayRgb::Input(const uint8_t *pBuffer, uint32_t nSize, [[maybe_unused]] uint32_t nFromIp, [[maybe_unused]] uint16_t nFromPort) {
	if (__builtin_expect((memcmp("7seg!", pBuffer, 5) != 0), 0)) {
		return;
	}

	if (pBuffer[nSize - 1] == '\n') {
		DEBUG_PUTS("\'\\n\'");
		nSize--;
	}

	if (memcmp(&pBuffer[5], showmsg::PATH, showmsg::LENGTH) == 0) {
		const uint32_t nMsgLength = nSize - (5 + showmsg::LENGTH + 1);
		DEBUG_PRINTF("m_nBytesReceived=%d, nMsgLength=%d [%.*s]", nBytesReceived, nMsgLength, nMsgLength, &m_pUdpBuffer[(5 + showmsg::LENGTH + 1)]);

		if (((nMsgLength > 0) && (nMsgLength <= MAX_MESSAGE_SIZE)) && (pBuffer[5 + showmsg::LENGTH] == '#')) {
			SetMessage(reinterpret_cast<const char *>(&pBuffer[(5 + showmsg::LENGTH + 1)]), nMsgLength);
			return;
		}

		DEBUG_PUTS("Invalid !showmsg command");
		return;
	}

	if (memcmp(&pBuffer[5], rgb::PATH, rgb::LENGTH) == 0) {
		if ((nSize == (5 + rgb::LENGTH + 1 + rgb::HEX_SIZE)) && (pBuffer[5 + rgb::LENGTH] == '#')) {
			SetRGB(reinterpret_cast<const char *>(&pBuffer[(5 + rgb::LENGTH + 1)]));
			return;
		}

		DEBUG_PUTS("Invalid !rgb command");
		return;
	}

	if (memcmp(&pBuffer[5], master::PATH, master::LENGTH) == 0) {
		if ((nSize == (5 + master::LENGTH + 1 + master::HEX_SIZE)) && (pBuffer[5 + master::LENGTH] == '#')) {
			m_nMaster = hexadecimalToDecimal(reinterpret_cast<const char *>(&pBuffer[(5 + master::LENGTH + 1)]), master::HEX_SIZE);
			return;
		}

		DEBUG_PUTS("Invalid !master command");
		return;
	}

	DEBUG_PUTS("Invalid command");
}

void LtcDisplayRgb::Print() {
	if (m_tDisplayRgbType == Type::RGBPANEL) {
#if !defined (CONFIG_LTC_DISABLE_RGB_PANEL)
		puts("Display RGB panel");
#else
		puts("Display RGB panel disabled");
#endif
	} else {
#if !defined (CONFIG_LTC_DISABLE_WS28XX)
		puts("Display WS28xx");
		printf(" Type    : %s [%d]\n", pixel::pixel_get_type(m_PixelType), static_cast<int>(m_PixelType));
		printf(" Mapping : %s [%d]\n", pixel::pixel_get_map(m_PixelMap), static_cast<int>(m_PixelMap));
#else
		puts("Display WS28xx disabled");
#endif
	}

	printf(" Master  : %d\n", m_nMaster);
	printf(" RGB     : Character 0x%.6X, Colon 0x%.6X, Message 0x%.6X\n", m_aColour[static_cast<uint32_t>(ColourIndex::TIME)], m_aColour[static_cast<uint32_t>(ColourIndex::COLON)], m_aColour[static_cast<uint32_t>(ColourIndex::MESSAGE)]);

	if (m_pLtcDisplayRgbSet == nullptr) {
		puts(" No Init()!");
	} else {
		m_pLtcDisplayRgbSet->Print();
	}
}
