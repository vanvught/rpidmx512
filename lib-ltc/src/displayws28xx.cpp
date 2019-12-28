/**
 * @file displayws28xx.cpp
 */
/*
 * Copyright (C) 2019 by hippy mailto:dmxout@gmail.com
 * Based on: displaymax7219.cpp
 * Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "displayws28xx.h"
#include "displayws28xx_font.h"

#if defined(USE_SPI_DMA)
 #include "h3/ws28xxdma.h"
#else
 #include "ws28xx.h"
#endif
#include "rgbmapping.h"

#include "hardware.h"
#include "network.h"

#include "debug.h"

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

static const char sRGB[] ALIGNED = "rgb";
#define RGB_LENGTH			(sizeof(sRGB)/sizeof(sRGB[0]) - 1)
#define RGB_SIZE_HEX		(7) // 1 byte index followed by 6 bytes hex RGB

static const char sMaster[] ALIGNED = "master";
#define MASTER_LENGTH 		(sizeof(sMaster)/sizeof(sMaster[0]) - 1)
#define MASTER_SIZE_HEX		(2)

static const char sDisplayMSG[] ALIGNED = "showmsg";
#define DMSG_LENGTH 		(sizeof(sDisplayMSG)/sizeof(sDisplayMSG[0]) - 1)
#define DMSG_SIZE			(11)

enum TUdpPort {
	UDP_PORT = 0x2812
};

DisplayWS28xx *DisplayWS28xx::s_pThis = 0;

DisplayWS28xx::DisplayWS28xx(TWS28XXType tLedType):
	m_tLedType(tLedType),
	m_tMapping(RGB_MAPPING_RGB),
	m_nHandle(-1),
	m_nMaster(255),
	m_bShowMsg(false),
	m_nSecondsPrevious(60), // Force update
	m_nColonBlinkMode(COLON_BLINK_MODE_DOWN),
	m_nMillis(0),
	m_nWsTicker(0),
	m_nMsgTimer(0),
	m_ColonBlinkMillis(0)

{
	s_pThis = this;

	m_aColour[WS28XX_COLOUR_INDEX_SEGMENT] = WS28XXDISPLAY_DEFAULT_COLOUR_SEGMENT;
	m_aColour[WS28XX_COLOUR_INDEX_COLON] = WS28XXDISPLAY_DEFAULT_COLOUR_COLON;
	m_aColour[WS28XX_COLOUR_INDEX_MESSAGE] = WS28XXDISPLAY_DEFAULT_COLOUR_MESSAGE;
}

DisplayWS28xx::~DisplayWS28xx(void) {
	if (m_pWS28xx != 0) {
		delete m_pWS28xx;
		m_pWS28xx = 0;
	}
}

void DisplayWS28xx::Init(uint8_t nIntensity) {
	assert(m_pWS28xx == 0);
#if defined(USE_SPI_DMA)
	m_pWS28xx = new WS28xxDMA(m_tLedType, WS28XX_LED_COUNT);
#else
	m_pWS28xx = new WS28xx(m_tLedType, WS28XX_LED_COUNT);
#endif
	assert(m_pWS28xx != 0);

	m_pWS28xx->Initialize();
	m_pWS28xx->SetGlobalBrightness(nIntensity);

	SetRGB(m_aColour[WS28XX_COLOUR_INDEX_SEGMENT], WS28XX_COLOUR_INDEX_SEGMENT);
	SetRGB(m_aColour[WS28XX_COLOUR_INDEX_COLON], WS28XX_COLOUR_INDEX_COLON);
	SetRGB(m_aColour[WS28XX_COLOUR_INDEX_MESSAGE], WS28XX_COLOUR_INDEX_MESSAGE);

	m_nHandle = Network::Get()->Begin(UDP_PORT);
	assert(m_nHandle != -1);
}

// set the current RGB values, remapping them to different LED strip mappings
// ToDo Move RGB mapping to lib-???
void DisplayWS28xx::SetRGB(uint8_t nRed, uint8_t nGreen, uint8_t nBlue, TWS28xxColourIndex tIndex) {
	uint8_t nRedCurrent, nGreenCurrent, nBlueCurrent;

	switch (m_tMapping) {
	case RGB_MAPPING_RGB:
		nRedCurrent = nRed;
		nGreenCurrent = nGreen;
		nBlueCurrent = nBlue;
		break;
	case RGB_MAPPING_RBG:
		nRedCurrent = nRed;
		nGreenCurrent = nBlue;
		nBlueCurrent = nGreen;
		break;
	case RGB_MAPPING_GRB:
		nRedCurrent = nGreen;
		nGreenCurrent = nRed;
		nBlueCurrent = nBlue;
		break;
	case RGB_MAPPING_GBR:
		nRedCurrent = nGreen;
		nGreenCurrent = nBlue;
		nBlueCurrent = nRed;
		break;
	case RGB_MAPPING_BRG:
		nRedCurrent = nBlue;
		nGreenCurrent = nRed;
		nBlueCurrent = nGreen;
		break;
	case RGB_MAPPING_BGR:
		nRedCurrent = nBlue;
		nGreenCurrent = nGreen;
		nBlueCurrent = nRed;
		break;
	default: // RGB
		nRedCurrent = nRed;
		nGreenCurrent = nGreen;
		nBlueCurrent = nBlue;
		break;
	}

	switch (tIndex) {
	case WS28XX_COLOUR_INDEX_SEGMENT:
		nRedSegment = nRedCurrent;
		nGreenSegment = nGreenCurrent;
		nBlueSegment = nBlueCurrent;
		break;
	case WS28XX_COLOUR_INDEX_COLON:
		nRedColon = nRedCurrent;
		nGreenColon = nGreenCurrent;
		nBlueColon = nBlueCurrent;
		break;
	case WS28XX_COLOUR_INDEX_MESSAGE:
		nRedMsg = nRedCurrent;
		nGreenMsg = nGreenCurrent;
		nBlueMsg = nBlueCurrent;
		break;
	default:
		nRedSegment = nRedCurrent;
		nGreenSegment = nGreenCurrent;
		nBlueSegment = nBlueCurrent;
		break;
	}
}

void DisplayWS28xx::SetRGB(uint32_t nRGB, TWS28xxColourIndex tIndex) {
	const uint8_t nRed = (uint8_t) ((nRGB & 0xFF0000) >> 16);
	const uint8_t nGreen = (uint8_t) ((nRGB & 0xFF00) >> 8);
	const uint8_t nBlue = (uint8_t) (nRGB & 0xFF);

	SetRGB(nRed, nGreen, nBlue, tIndex);
}

void DisplayWS28xx::SetRGB(const char *pHexString) {
	if (!isdigit((int) pHexString[0])) {
		return;
	}

	const TWS28xxColourIndex tIndex = (TWS28xxColourIndex) (pHexString[0]  - '0');

	if (tIndex >= WS28XX_COLOUR_INDEX_LAST) {
		return;
	}

	const uint32_t nRGB = hexadecimalToDecimal(pHexString + 1);

	SetRGB(nRGB, tIndex);
}

void DisplayWS28xx::Run() {
	uint32_t nIPAddressFrom;
	uint16_t nForeignPort;

	m_nMillis = Hardware::Get()->Millis(); // millis now

	if (m_bShowMsg) {
		if (m_nMillis - m_nMsgTimer >= WS82XX_MSG_TIME_MS) {
			m_bShowMsg = false;
		}
	}

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
		DEBUG_PRINTF("RX: %d  MsgLen: %d  Msg: %.*s", m_nBytesReceived, nMsgLength, nMsgLength, &m_Buffer[(5 + DMSG_LENGTH + 1)]);

		if (((nMsgLength > 0) && (nMsgLength <= DMSG_SIZE)) && (m_Buffer[5 + DMSG_LENGTH] == '#')) {
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

uint32_t DisplayWS28xx::hexadecimalToDecimal(const char *pHexValue, uint32_t nLength) {
	char *src = (char *) pHexValue;
	uint32_t ret = 0;
	uint8_t nibble;

	while (nLength-- > 0) {
		const char d = *src;

		if (isxdigit((int) d) == 0) {
			break;
		}

		nibble = d > '9' ? (d | 0x20) - 'a' + 10 : (d - '0');
		ret = (ret << 4) | nibble;
		src++;
	}

	return ret;
}

void DisplayWS28xx::Show(const char *pTimecode) {
	if (!m_bShowMsg) { // if not showing temporary message
		uint8_t outR = 0, outG = 0, outB = 0;

		WriteChar(pTimecode[0], 0, nRedSegment, nGreenSegment, nBlueSegment);
		WriteChar(pTimecode[1], 1, nRedSegment, nGreenSegment, nBlueSegment); // hh
		WriteChar(pTimecode[3], 2, nRedSegment, nGreenSegment, nBlueSegment);
		WriteChar(pTimecode[4], 3, nRedSegment, nGreenSegment, nBlueSegment); // mm
		WriteChar(pTimecode[6], 4, nRedSegment, nGreenSegment, nBlueSegment);
		WriteChar(pTimecode[7], 5, nRedSegment, nGreenSegment, nBlueSegment); // sc
		WriteChar(pTimecode[9], 6, nRedSegment, nGreenSegment, nBlueSegment);
		WriteChar(pTimecode[10], 7, nRedSegment, nGreenSegment, nBlueSegment); // fr

		// option - blink colon
		if (m_nColonBlinkMode != COLON_BLINK_MODE_OFF) {
			if (m_nSecondsPrevious != pTimecode[7]) { // seconds have changed
				m_ColonBlinkMillis = m_nMillis;
				m_nSecondsPrevious = pTimecode[7];

				outR = 0;
				outG = 0;
				outB = 0;
			} else if (m_nMillis - m_ColonBlinkMillis < 1000) {
				uint32_t nMaster;

				if (m_nColonBlinkMode == COLON_BLINK_MODE_DOWN) {
					nMaster = 255 - ((m_nMillis - m_ColonBlinkMillis) * 255 / 1000);
				} else {
					nMaster = ((m_nMillis - m_ColonBlinkMillis) * 255 / 1000);
				}

				outR = (nMaster * nRedColon) / 255;
				outG = (nMaster *  nGreenColon) / 255;
				outB = (nMaster * nBlueColon) / 255;
			}
		} else {
			// straight thru
			outR = nRedColon;
			outG = nGreenColon;
			outB = nBlueColon;
		}

		WriteColon(pTimecode[2], 0, outR, outG, outB); // 1st :
		WriteColon(pTimecode[5], 1, outR, outG, outB); // 2nd :
		WriteColon(pTimecode[8], 2, outR, outG, outB); // 3rd :

		m_pWS28xx->Update();
	} else {
		ShowMessage();
	}
}

void DisplayWS28xx::SetMessage(const char *pMessage, uint32_t nSize) {
	assert(pMessage != 0);
	assert(nSize != 0);

	memset(&m_aMessage, ' ', WS28XX_MAX_MSG_SIZE);
	memcpy(&m_aMessage, pMessage, nSize);

	m_nMsgTimer = Hardware::Get()->Millis();
	m_bShowMsg = true;
}


void DisplayWS28xx::ShowMessage(void) {
	uint8_t nRed, nGreen, nBlue;

	if (m_nMillis - m_nMsgTimer <= WS82XX_MSG_TIME_MS) {
		nRed =   ((m_nMillis - m_nMsgTimer) * nRedMsg) / WS82XX_MSG_TIME_MS;
		nGreen = ((m_nMillis - m_nMsgTimer) * nGreenMsg) / WS82XX_MSG_TIME_MS;
		nBlue =  ((m_nMillis - m_nMsgTimer) * nBlueMsg) / WS82XX_MSG_TIME_MS;
	} else {
		// straight thru
		nRed = nRedMsg;
		nGreen = nGreenMsg;
		nBlue = nBlueMsg;
	}

	for (uint32_t nCount = 0; (nCount < WS28XX_MAX_MSG_SIZE) && (nCount < WS28XX_NUM_OF_DIGITS); nCount++) {
		if (m_aMessage[nCount] != 0) {
			WriteChar(m_aMessage[nCount], nCount, nRed, nGreen, nBlue);
		} else {
			WriteChar(' ', nCount);
		}
	}

	// blank colons
	for (uint32_t nCount = 0; nCount < WS28XX_NUM_OF_COLONS; nCount++) {
		WriteColon(' ', nCount, 0, 0, 0); // 1st :
	}

	m_pWS28xx->Update();
}

void DisplayWS28xx::ShowSysTime(const char *pSystemTime) {
	if (m_bShowMsg) {
		ShowMessage();
		return;
	}

	WriteChar(' ', 0, nRedSegment, nGreenSegment, nBlueSegment);
	WriteChar(' ', 1, nRedSegment, nGreenSegment, nBlueSegment);
	WriteChar(pSystemTime[0], 2, nRedSegment, nGreenSegment, nBlueSegment);
	WriteChar(pSystemTime[1], 3, nRedSegment, nGreenSegment, nBlueSegment);
	WriteChar(pSystemTime[3], 4, nRedSegment, nGreenSegment, nBlueSegment);
	WriteChar(pSystemTime[4], 5, nRedSegment, nGreenSegment, nBlueSegment);
	WriteChar(pSystemTime[6], 6, nRedSegment, nGreenSegment, nBlueSegment);
	WriteChar(pSystemTime[7], 7, nRedSegment, nGreenSegment, nBlueSegment);

	WriteColon(' ', 0, nRedColon, nGreenColon, nBlueColon);
	WriteColon(':', 1, nRedColon, nGreenColon, nBlueColon);
	WriteColon(':', 3, nRedColon, nGreenColon, nBlueColon);

	m_pWS28xx->Update();
}

void DisplayWS28xx::RenderSegment(bool bOnOff, uint16_t cur_digit_base, uint8_t cur_segment, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	if (!(m_nMaster == 0 || m_nMaster == 255)) {
		nRed = (m_nMaster * nRed) / 255 ;
		nGreen = (m_nMaster * nGreen) / 255 ;
		nBlue = (m_nMaster * nBlue) / 255 ;
	}

	const uint32_t cur_seg_base = cur_digit_base + (cur_segment * LEDS_PER_SEGMENT);

	while (m_pWS28xx->IsUpdating()) {
		// wait for completion
	}

	for (uint32_t nIndex = cur_seg_base; nIndex < (cur_seg_base + LEDS_PER_SEGMENT); nIndex++) {
		if (bOnOff) {
			m_pWS28xx->SetLED(nIndex, nRed, nGreen, nBlue); // on
		} else {
			m_pWS28xx->SetLED(nIndex, 0, 0, 0); // off
		}
	}
}

void DisplayWS28xx::WriteChar(uint8_t nChar, uint8_t nPos, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	if (nChar > sizeof(Seg7Array)) {
		return;
	}

	const uint32_t cur_digit_base = nPos * SEGMENTS_PER_DIGIT;

	uint8_t chr;

	if (nChar & (1 << 7)) {	// use custom bitmap
		chr = nChar;
	} else {				// use displayws28xx_font
		chr = Seg7Array[nChar];
	}

	RenderSegment(chr & (1 << 6), cur_digit_base, 0, nRed, nGreen, nBlue);
	RenderSegment(chr & (1 << 5), cur_digit_base, 1, nRed, nGreen, nBlue);
	RenderSegment(chr & (1 << 4), cur_digit_base, 2, nRed, nGreen, nBlue);
	RenderSegment(chr & (1 << 3), cur_digit_base, 3, nRed, nGreen, nBlue);
	RenderSegment(chr & (1 << 2), cur_digit_base, 4, nRed, nGreen, nBlue);
	RenderSegment(chr & (1 << 1), cur_digit_base, 5, nRed, nGreen, nBlue);
	RenderSegment(chr & (1 << 0), cur_digit_base, 6, nRed, nGreen, nBlue);
}

void DisplayWS28xx::WriteColon(uint8_t nChar, uint8_t nPos, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	if (!(m_nMaster == 0 || m_nMaster == 255)) {
		nRed = (m_nMaster * nRed) / 255;
		nGreen = (m_nMaster * nGreen) / 255;
		nBlue = (m_nMaster * nBlue) / 255;
	}

	const uint32_t cur_digit_base = (WS28XX_NUM_OF_DIGITS * SEGMENTS_PER_DIGIT) + (nPos * LEDS_PER_COLON);
	const bool OnOff = (nChar == ':' || nChar == '.' || nChar == ';') ? 1 : 0;

	while (m_pWS28xx->IsUpdating()) {
		// wait for completion
	}

	for (uint32_t nIndex = cur_digit_base; nIndex < (cur_digit_base + LEDS_PER_COLON); nIndex++) {
		if (OnOff) {
			m_pWS28xx->SetLED(nIndex, nRed, nGreen, nBlue); // on
		} else {
			m_pWS28xx->SetLED(nIndex, 0, 0, 0); // off
		}
	}
}

void DisplayWS28xx::Print(void) {
	printf("Display WS28xx\n");
	printf(" %d Digit(s), %d Colons, %d LEDs\n", WS28XX_NUM_OF_DIGITS, WS28XX_NUM_OF_COLONS, WS28XX_LED_COUNT);
	printf(" Type    : %s [%d]\n", WS28xx::GetLedTypeString(m_tLedType), m_tLedType);
	printf(" Mapping : %s [%d]\n", RGBMapping::ToString(m_tMapping), m_tMapping);
	printf(" Master  : %d\n", m_nMaster);
	printf(" RGB     : Segment 0x%.6X, Colon 0x%.6X, Message 0x%.6X\n", m_aColour[WS28XX_COLOUR_INDEX_SEGMENT], m_aColour[WS28XX_COLOUR_INDEX_COLON], m_aColour[WS28XX_COLOUR_INDEX_MESSAGE]);
}
