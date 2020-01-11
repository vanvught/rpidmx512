/**
 * @file ltcdisplayws28xx.cpp
 */
/*
 * Copyright (C) 2019 by hippy mailto:dmxout@gmail.com
 * Copyright (C) 2019-2020 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "ltcdisplayws28xx.h"

#include "ltcdisplayws28xx7segment.h"
#include "ltcdisplayws28xxmatrix.h"

#include "ltc.h"

#include "rgbmapping.h"

#include "hardware.h"
#include "network.h"

#include "debug.h"

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

static const char sRGB[] ALIGNED = "rgb";
#define RGB_LENGTH			(sizeof(sRGB)/sizeof(sRGB[0]) - 1)
#define RGB_SIZE_HEX		7 // 1 byte index followed by 6 bytes hex RGB

static const char sMaster[] ALIGNED = "master";
#define MASTER_LENGTH 		(sizeof(sMaster)/sizeof(sMaster[0]) - 1)
#define MASTER_SIZE_HEX		2

static const char sDisplayMSG[] ALIGNED = "showmsg";
#define DMSG_LENGTH 		(sizeof(sDisplayMSG)/sizeof(sDisplayMSG[0]) - 1)

enum TUdpPort {
	UDP_PORT = 0x2812
};

#define MESSAGE_TIME_MS		3000

LtcDisplayWS28xx *LtcDisplayWS28xx::s_pThis = 0;

LtcDisplayWS28xx::LtcDisplayWS28xx(TLtcDisplayWS28xxTypes tType) :
	m_tDisplayWS28xxTypes(tType),
	m_nIntensity(LTCDISPLAYWS28XX_DEFAULT_GLOBAL_BRIGHTNESS),
	m_nHandle(-1),
	m_tMapping(RGB_MAPPING_RGB),
	m_nMaster(LTCDISPLAYWS28XX_DEFAULT_MASTER),
	m_bShowMsg(false),
	m_nMsgTimer(0),
	m_nColonBlinkMillis(0),
	m_nSecondsPrevious(60),
	m_tColonBlinkMode(LTCDISPLAYWS28XX_COLON_BLINK_MODE_DOWN),
	m_pLtcDisplayWS28xxSet(0)
{
	DEBUG_ENTRY

	s_pThis = this;

	m_aColour[LTCDISPLAYWS28XX_COLOUR_INDEX_DIGIT] = LTCDISPLAYWS28XX_DEFAULT_COLOUR_DIGIT;
	m_aColour[LTCDISPLAYWS28XX_COLOUR_INDEX_COLON] = LTCDISPLAYWS28XX_DEFAULT_COLOUR_COLON;
	m_aColour[LTCDISPLAYWS28XX_COLOUR_INDEX_MESSAGE] = LTCDISPLAYWS28XX_DEFAULT_COLOUR_MESSAGE;

	DEBUG_EXIT
}

LtcDisplayWS28xx::~LtcDisplayWS28xx(void) {
	DEBUG_ENTRY

	assert(m_pLtcDisplayWS28xxSet == 0);

	delete m_pLtcDisplayWS28xxSet;
	m_pLtcDisplayWS28xxSet = 0;

	DEBUG_EXIT
}

void LtcDisplayWS28xx::Init(TWS28XXType tLedType, uint8_t nIntensity) {
	DEBUG_ENTRY

	m_tLedType = tLedType;

	if (m_tDisplayWS28xxTypes == LTCDISPLAYWS28XX_TYPE_7SEGMENT) {
		m_pLtcDisplayWS28xxSet = new LtcDisplayWS28xx7Segment;
	} else {
		m_pLtcDisplayWS28xxSet = new LtcDisplayWS28xxMatrix;
	}

	assert(m_pLtcDisplayWS28xxSet != 0);
	m_pLtcDisplayWS28xxSet->Init(tLedType);

	SetRGB(m_aColour[LTCDISPLAYWS28XX_COLOUR_INDEX_DIGIT], LTCDISPLAYWS28XX_COLOUR_INDEX_DIGIT);
	SetRGB(m_aColour[LTCDISPLAYWS28XX_COLOUR_INDEX_COLON], LTCDISPLAYWS28XX_COLOUR_INDEX_COLON);
	SetRGB(m_aColour[LTCDISPLAYWS28XX_COLOUR_INDEX_MESSAGE], LTCDISPLAYWS28XX_COLOUR_INDEX_MESSAGE);

	m_nHandle = Network::Get()->Begin(UDP_PORT);
	assert(m_nHandle != -1);

	DEBUG_EXIT
}

void LtcDisplayWS28xx::Show(const char *pTimecode) {
	if (__builtin_expect((m_bShowMsg), 0)) {
		ShowMessage();
		return;
	}

	struct TLtcDisplayRgbColours tColoursColons;

	if (m_tColonBlinkMode != LTCDISPLAYWS28XX_COLON_BLINK_MODE_OFF) {
		const uint32_t nMillis = Hardware::Get()->Millis();

		if (m_nSecondsPrevious != pTimecode[LTC_TC_INDEX_SECONDS_UNITS]) { // seconds have changed
			m_nSecondsPrevious = pTimecode[LTC_TC_INDEX_SECONDS_UNITS];
			m_nColonBlinkMillis = nMillis;

			tColoursColons.nRed = 0;
			tColoursColons.nGreen = 0;
			tColoursColons.nBlue = 0;
		} else if (nMillis - m_nColonBlinkMillis < 1000) {
			uint32_t nMaster;

			if (m_tColonBlinkMode == LTCDISPLAYWS28XX_COLON_BLINK_MODE_DOWN) {
				nMaster = 255 - ((nMillis - m_nColonBlinkMillis) * 255 / 1000);
			} else {
				nMaster = ((nMillis - m_nColonBlinkMillis) * 255 / 1000);
			}

			if (!(m_nMaster == 0 || m_nMaster == 255)) {
				nMaster = (m_nMaster * nMaster) / 255 ;
			}

			tColoursColons.nRed = (nMaster * m_tColoursColons.nRed) / 255;
			tColoursColons.nGreen = (nMaster *  m_tColoursColons.nGreen) / 255;
			tColoursColons.nBlue = (nMaster * m_tColoursColons.nBlue) / 255;
		} else {
			if (!(m_nMaster == 0 || m_nMaster == 255)) {
				tColoursColons.nRed = (m_nMaster * m_tColoursColons.nRed) / 255 ;
				tColoursColons.nGreen = (m_nMaster * m_tColoursColons.nGreen) / 255 ;
				tColoursColons.nBlue = (m_nMaster * m_tColoursColons.nBlue) / 255 ;
			} else {
				tColoursColons.nRed = m_tColoursColons.nRed;
				tColoursColons.nGreen = m_tColoursColons.nGreen;
				tColoursColons.nBlue = m_tColoursColons.nBlue;
			}
		}
	} else {
		if (!(m_nMaster == 0 || m_nMaster == 255)) {
			tColoursColons.nRed = (m_nMaster * m_tColoursColons.nRed) / 255 ;
			tColoursColons.nGreen = (m_nMaster * m_tColoursColons.nGreen) / 255 ;
			tColoursColons.nBlue = (m_nMaster * m_tColoursColons.nBlue) / 255 ;
		} else {
			tColoursColons.nRed = m_tColoursColons.nRed;
			tColoursColons.nGreen = m_tColoursColons.nGreen;
			tColoursColons.nBlue = m_tColoursColons.nBlue;
		}
	}

	struct TLtcDisplayRgbColours tColours;

	if (!(m_nMaster == 0 || m_nMaster == 255)) {
		tColours.nRed = (m_nMaster * m_tColours.nRed) / 255 ;
		tColours.nGreen = (m_nMaster * m_tColours.nGreen) / 255 ;
		tColours.nBlue = (m_nMaster * m_tColours.nBlue) / 255 ;
	} else {
		tColours.nRed = m_tColours.nRed;
		tColours.nGreen = m_tColours.nGreen;
		tColours.nBlue = m_tColours.nBlue;
	}

	m_pLtcDisplayWS28xxSet->Show(pTimecode, tColours, tColoursColons);
}

void LtcDisplayWS28xx::ShowSysTime(const char *pSystemTime) {
	if (__builtin_expect((m_bShowMsg), 0)) {
		ShowMessage();
		return;
	}

	struct TLtcDisplayRgbColours tColours;
	struct TLtcDisplayRgbColours tColoursColons;

	if (!(m_nMaster == 0 || m_nMaster == 255)) {
		tColours.nRed = (m_nMaster * m_tColours.nRed) / 255 ;
		tColours.nGreen = (m_nMaster * m_tColours.nGreen) / 255 ;
		tColours.nBlue = (m_nMaster * m_tColours.nBlue) / 255 ;
		//
		tColoursColons.nRed = (m_nMaster * m_tColoursColons.nRed) / 255 ;
		tColoursColons.nGreen = (m_nMaster * m_tColoursColons.nGreen) / 255 ;
		tColoursColons.nBlue = (m_nMaster * m_tColoursColons.nBlue) / 255 ;
	} else {
		tColours.nRed = m_tColours.nRed;
		tColours.nGreen = m_tColours.nGreen;
		tColours.nBlue = m_tColours.nBlue;
		//
		tColoursColons.nRed = m_tColoursColons.nRed;
		tColoursColons.nGreen = m_tColoursColons.nGreen;
		tColoursColons.nBlue = m_tColoursColons.nBlue;
	}

	m_pLtcDisplayWS28xxSet->ShowSysTime(pSystemTime, tColours, tColoursColons);
}

void LtcDisplayWS28xx::SetMessage(const char *pMessage, uint32_t nSize) {
	assert(pMessage != 0);

	uint32_t i;
	const char *pSrc = pMessage;
	char *pDst = (char *)m_aMessage;

	for (i = 0; i < nSize; i++) {
		*pDst++ = *pSrc++;
	}

	for (; i < sizeof(m_aMessage); i++) {
		*pDst++ = ' ';
	}

	m_nMsgTimer = Hardware::Get()->Millis();
	m_bShowMsg = true;
}

void LtcDisplayWS28xx::ShowMessage(void) {
	struct TLtcDisplayRgbColours tColours;

	const uint32_t nMillis = Hardware::Get()->Millis();

	tColours.nRed = ((nMillis - m_nMsgTimer) * m_tColoursMessage.nRed) / MESSAGE_TIME_MS;
	tColours.nGreen = ((nMillis - m_nMsgTimer) * m_tColoursMessage.nGreen) / MESSAGE_TIME_MS;
	tColours.nBlue = ((nMillis - m_nMsgTimer) * m_tColoursMessage.nBlue) / MESSAGE_TIME_MS;

	m_pLtcDisplayWS28xxSet->ShowMessage(m_aMessage, tColours);
}

void LtcDisplayWS28xx::WriteChar(uint8_t nChar, uint8_t nPos) {
	struct TLtcDisplayRgbColours tColours;

	if (!(m_nMaster == 0 || m_nMaster == 255)) {
		tColours.nRed = (m_nMaster * m_tColours.nRed) / 255 ;
		tColours.nGreen = (m_nMaster * m_tColours.nGreen) / 255 ;
		tColours.nBlue = (m_nMaster * m_tColours.nBlue) / 255 ;
	} else {
		tColours.nRed = m_tColours.nRed;
		tColours.nGreen = m_tColours.nGreen;
		tColours.nBlue = m_tColours.nBlue;
	}

	m_pLtcDisplayWS28xxSet->WriteChar(nChar, nPos, tColours);
}

void LtcDisplayWS28xx::Run(void) {
	if (__builtin_expect((m_bShowMsg), 0)) {
		if (Hardware::Get()->Millis() - m_nMsgTimer >= MESSAGE_TIME_MS) {
			m_bShowMsg = false;
		}
	}

	uint32_t nIPAddressFrom;
	uint16_t nForeignPort;
	uint16_t m_nBytesReceived = Network::Get()->RecvFrom(m_nHandle, (uint8_t *) &m_Buffer, (uint16_t) sizeof(m_Buffer), &nIPAddressFrom, &nForeignPort);

	if (__builtin_expect((m_nBytesReceived < (int) 8), 1)) {
		return;
	}

	if (__builtin_expect((memcmp("7seg!", m_Buffer, 5) != 0), 0)) {
		return;
	}

	if (m_Buffer[m_nBytesReceived - 1] == '\n') {
		DEBUG_PUTS("\'\\n\'");
		m_nBytesReceived--;
	}

	if (memcmp(&m_Buffer[5], sDisplayMSG, DMSG_LENGTH) == 0) {
		const uint32_t nMsgLength = m_nBytesReceived - (5 + DMSG_LENGTH + 1);
		DEBUG_PRINTF("m_nBytesReceived=%d, nMsgLength=%d [%.*s]", m_nBytesReceived, nMsgLength, nMsgLength, &m_Buffer[(5 + DMSG_LENGTH + 1)]);

		if (((nMsgLength > 0) && (nMsgLength <= LTCDISPLAY_MAX_MESSAGE_SIZE)) && (m_Buffer[5 + DMSG_LENGTH] == '#')) {
			SetMessage((const char *) &m_Buffer[(5 + DMSG_LENGTH + 1)], nMsgLength);
		} else {
			DEBUG_PUTS("Invalid !showmsg command");
		}
	} else if (memcmp(&m_Buffer[5], sRGB, RGB_LENGTH) == 0) {
		if ((m_nBytesReceived == (5 + RGB_LENGTH + 1 + RGB_SIZE_HEX)) && (m_Buffer[5 + RGB_LENGTH] == '#')) {
			SetRGB((const char *) &m_Buffer[(5 + RGB_LENGTH + 1)]);
		} else {
			DEBUG_PUTS("Invalid !rgb command");
		}
	} else if (memcmp(&m_Buffer[5], sMaster, MASTER_LENGTH) == 0) {
		if ((m_nBytesReceived == (5 + MASTER_LENGTH + 1 + MASTER_SIZE_HEX)) && (m_Buffer[5 + MASTER_LENGTH] == '#')) {
			m_nMaster = hexadecimalToDecimal((const char *) &m_Buffer[(5 + MASTER_LENGTH + 1)], MASTER_SIZE_HEX);
		} else {
			DEBUG_PUTS("Invalid !master command");
		}
	} else {
		DEBUG_PUTS("Invalid command");
	}
}

void LtcDisplayWS28xx::Print(void) {
	printf("Display WS28xx\n");
	printf(" Type    : %s [%d]\n", WS28xx::GetLedTypeString(m_tLedType), m_tLedType);
	printf(" Mapping : %s [%d]\n", RGBMapping::ToString(m_tMapping), m_tMapping);
	printf(" Master  : %d\n", m_nMaster);
	printf(" RGB     : Character 0x%.6X, Colon 0x%.6X, Message 0x%.6X\n", m_aColour[LTCDISPLAYWS28XX_COLOUR_INDEX_DIGIT], m_aColour[LTCDISPLAYWS28XX_COLOUR_INDEX_COLON], m_aColour[LTCDISPLAYWS28XX_COLOUR_INDEX_MESSAGE]);

	m_pLtcDisplayWS28xxSet->Print();
}
