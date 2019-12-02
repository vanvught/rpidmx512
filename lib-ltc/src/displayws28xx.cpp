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

enum tUdpPort {
	WS28XX_UDP_PORT = 0x2812
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
}

DisplayWS28xx::~DisplayWS28xx(void) {
	if (m_pWS28xx != 0) {
		delete m_pWS28xx;
		m_pWS28xx = 0;
	}
}

void DisplayWS28xx::Init(uint8_t nIntensity, TRGBMapping tMapping) {
	m_pWS28xx = new WS28xx(m_tLedType, WS28XX_LED_COUNT);
	assert(m_pWS28xx != 0);

	m_pWS28xx->Initialize();
	m_pWS28xx->SetGlobalBrightness(nIntensity);

	m_tMapping = tMapping;

	SetRGB(0xFF, 0, 0, 0);
	SetRGB(0xFF, 0xCC, 0, 1);
	SetRGB(0xFF, 0xFF, 0xFF, 2);

	m_nHandle = Network::Get()->Begin(WS28XX_UDP_PORT);
	assert(m_nHandle != -1);
}

// set the current RGB values, remapping them to different LED strip mappings
// TODO Move to lib-ws28xx
void DisplayWS28xx::SetRGB(uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nIndex) {
	switch (m_tMapping) {
	case RGB_MAPPING_RGB:
		curR = nRed;
		curG = nGreen;
		curB = nBlue;
		break;

	case RGB_MAPPING_RBG:
		curR = nRed;
		curG = nBlue;
		curB = nGreen;
		break;

	case RGB_MAPPING_GRB:
		curR = nGreen;
		curG = nRed;
		curB = nBlue;
		break;

	case RGB_MAPPING_GBR:
		curR = nGreen;
		curG = nBlue;
		curB = nRed;
		break;

	case RGB_MAPPING_BRG:
		curR = nBlue;
		curG = nRed;
		curB = nGreen;
		break;

	case RGB_MAPPING_BGR:
		curR = nBlue;
		curG = nGreen;
		curB = nRed;
		break;

	default: // RGB
		curR = nRed;
		curG = nGreen;
		curB = nBlue;
		break;
	}

	switch (nIndex) {
	case 0: // segment colour
		segR = curR;
		segG = curG;
		segB = curB;
		break;

	case 1: // colon colour
		colR = curR;
		colG = curG;
		colB = curB;
		break;

	case 2: // message colour
		msgR = curR;
		msgG = curG;
		msgB = curB;
		break;

	default:
		segR = curR;
		segG = curG;
		segB = curB;
		break;
	}
}

// set the current RGB values from a hex string, FFCC00
void DisplayWS28xx::SetRGB(const char *pHexString) {
	const uint8_t nIndex = pHexString[0] - '0';

	const uint32_t nRGB = hexadecimalToDecimal(pHexString + 1);

	const uint8_t nRed = (uint8_t) (nRGB >> 16);
	const uint8_t nGreen = (uint8_t) (nRGB >> 8);
	const uint8_t nBlue = (uint8_t) nRGB & 0xff;

	SetRGB(nRed, nGreen, nBlue, nIndex);
}

void DisplayWS28xx::Run() {
	uint32_t nIPAddressFrom;
	uint16_t nForeignPort;

	m_nMillis = Hardware::Get()->Millis(); // millis now

	if (m_bShowMsg) {
		if (m_nMillis - m_nMsgTimer >= WS82XX_MSG_TIME_MS) {
			m_bShowMsg = false;
		} else if (m_nMillis - m_nWsTicker >= WS28XX_UPDATE_MS) {
			m_nWsTicker = m_nMillis;
			ShowMessage(m_aMessage);
		}
	}

	uint16_t m_nBytesReceived = Network::Get()->RecvFrom(m_nHandle, (uint8_t *) &m_Buffer, (uint16_t) sizeof(m_Buffer), &nIPAddressFrom, &nForeignPort);

	if (__builtin_expect((m_nBytesReceived < (int) 8), 1)) {
		return;
	}

	if (__builtin_expect((memcmp("7seg", m_Buffer, 4) != 0), 0)) {
		return;
	}

	if (m_Buffer[m_nBytesReceived - 1] == '\n') {
		DEBUG_PUTS("\'\\n\'");
		m_nBytesReceived--;
	}

	if (m_Buffer[4] != '!') {
		DEBUG_PUTS("Invalid command");
		return;
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

void DisplayWS28xx::Show(const char *pTimecode)
{
	if (!m_bShowMsg) // if not showing temporary message
	{
		uint8_t outR = 0, outG = 0, outB = 0;

		WriteChar(pTimecode[0], 0, segR, segG, segB);
		WriteChar(pTimecode[1], 1, segR, segG, segB); // hh
		WriteChar(pTimecode[3], 2, segR, segG, segB);
		WriteChar(pTimecode[4], 3, segR, segG, segB); // mm
		WriteChar(pTimecode[6], 4, segR, segG, segB);
		WriteChar(pTimecode[7], 5, segR, segG, segB); // sc
		WriteChar(pTimecode[9], 6, segR, segG, segB);
		WriteChar(pTimecode[10], 7, segR, segG, segB); // fr

		// option - blink colon
		if (m_nColonBlinkMode != COLON_BLINK_MODE_OFF)
		{
			uint8_t mColonBlinkOffset = 0;

			switch (m_nColonBlinkMode) {
			case COLON_BLINK_MODE_DOWN:
				mColonBlinkOffset = 0;
				break;
			case COLON_BLINK_MODE_UP:
				mColonBlinkOffset = 255;
				break;
			default:
				mColonBlinkOffset = 0;
				break;
			}

			if (m_nSecondsPrevious != pTimecode[7]) { // seconds have changed
				m_ColonBlinkMillis = m_nMillis;
				m_nSecondsPrevious = pTimecode[7];
				outR = 0;
				outG = 0;
				outB = 0;
			} else if (m_nMillis - m_ColonBlinkMillis < 1000) {
				outR = (((m_nMillis - m_ColonBlinkMillis) * abs(mColonBlinkOffset - colR)) / 1000);
				outG = (((m_nMillis - m_ColonBlinkMillis) * abs(mColonBlinkOffset - colG)) / 1000);
				outB = (((m_nMillis - m_ColonBlinkMillis) * abs(mColonBlinkOffset - colB)) / 1000);
			}
		} else {
			// straight thru
			outR = colR;
			outG = colG;
			outB = colB;
		}

		WriteColon(pTimecode[2], 0, outR, outG, outB); // 1st :
		WriteColon(pTimecode[5], 1, outR, outG, outB); // 2nd :
		WriteColon(pTimecode[8], 2, outR, outG, outB); // 3rd :

		m_pWS28xx->Update();
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


void DisplayWS28xx::ShowMessage(const char *pMessage) {
	assert(pMessage != 0);

	uint8_t nRed = 0, nGreen = 0, nBlue = 0;

	if (m_nMillis - m_nMsgTimer <= WS82XX_MSG_TIME_MS) {
		nRed =   ((m_nMillis - m_nMsgTimer) * msgR) / WS82XX_MSG_TIME_MS;
		nGreen = ((m_nMillis - m_nMsgTimer) * msgG) / WS82XX_MSG_TIME_MS;
		nBlue =  ((m_nMillis - m_nMsgTimer) * msgB) / WS82XX_MSG_TIME_MS;
	} else {
		// straight thru
		nRed = msgR;
		nGreen = msgG;
		nBlue = msgB;
	}

	for (uint32_t nCount = 0; (nCount < WS28XX_MAX_MSG_SIZE) && (nCount < WS28XX_NUM_OF_DIGITS); nCount++) {
		if (pMessage[nCount] != 0) {
			WriteChar(pMessage[nCount], nCount, nRed, nGreen, nBlue);
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

void DisplayWS28xx::ShowSysTime(void) {
	const time_t ltime = time(0);
	const struct tm *local_time = localtime(&ltime);

	if (__builtin_expect(m_nSecondsPrevious == (uint32_t) local_time->tm_sec, 1)) {
		return;
	}

	m_nSecondsPrevious = local_time->tm_sec;

	WriteChar(' ', 0, segR, segG, segB);
	WriteChar(' ', 1, segR, segG, segB);
	WriteChar((local_time->tm_hour / 10), 2, segR, segG, segB);
	WriteChar((local_time->tm_hour % 10) | 0x80, 3, segR, segG, segB);
	WriteChar((local_time->tm_min / 10), 4, segR, segG, segB);
	WriteChar((local_time->tm_min % 10) | 0x80, 5, segR, segG, segB);
	WriteChar((local_time->tm_sec / 10), 6, segR, segG, segB);
	WriteChar((local_time->tm_sec % 10) | 0x80, 7, segR, segG, segB);

	WriteColon(' ', 0, colR, colG, colB);
	WriteColon(':', 0, colR, colG, colB);
	WriteColon(':', 0, colR, colG, colB);

	m_pWS28xx->Update();
}

void DisplayWS28xx::RenderSegment(bool bOnOff, uint16_t cur_digit_base, uint8_t cur_segment, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	if (m_nMaster != 0 || m_nMaster != 255) {
		nRed = (m_nMaster * nRed) / 255;
		nGreen = (m_nMaster * nGreen) / 255;
		nBlue = (m_nMaster * nBlue) / 255;
	}

	const uint32_t cur_seg_base = cur_digit_base + (cur_segment * LEDS_PER_SEGMENT);

	for (uint32_t nCount = cur_seg_base; nCount < (cur_seg_base + LEDS_PER_SEGMENT); nCount++) {
		if (bOnOff) {
			m_pWS28xx->SetLED(nCount, nRed, nGreen, nBlue); // on
		} else {
			m_pWS28xx->SetLED(nCount, 0, 0, 0); // off
		}
	}
}

void DisplayWS28xx::WriteChar(uint8_t nChar, uint8_t nPos, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	if (nChar > sizeof(Seg7Array)) {
		return;
	}

	const uint32_t cur_digit_base = nPos * SEGMENTS_PER_DIGIT;

	uint8_t chr;

	if (nChar & (1 << 7)) // use custom bitmap
		chr = nChar;
	else
		chr = Seg7Array[nChar]; // use displayws28xx_font

	RenderSegment(chr & (1 << 6), cur_digit_base, 0, nRed, nGreen, nBlue);
	RenderSegment(chr & (1 << 5), cur_digit_base, 1, nRed, nGreen, nBlue);
	RenderSegment(chr & (1 << 4), cur_digit_base, 2, nRed, nGreen, nBlue);
	RenderSegment(chr & (1 << 3), cur_digit_base, 3, nRed, nGreen, nBlue);
	RenderSegment(chr & (1 << 2), cur_digit_base, 4, nRed, nGreen, nBlue);
	RenderSegment(chr & (1 << 1), cur_digit_base, 5, nRed, nGreen, nBlue);
	RenderSegment(chr & (1 << 0), cur_digit_base, 6, nRed, nGreen, nBlue);
}

void DisplayWS28xx::WriteColon(uint8_t nChar, uint8_t nPos, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	if (m_nMaster != 0 || m_nMaster != 255) {
		nRed = (m_nMaster * nRed) / 255;
		nGreen = (m_nMaster * nGreen) / 255;
		nBlue = (m_nMaster * nBlue) / 255;
	}

	const uint32_t cur_digit_base = (WS28XX_NUM_OF_DIGITS * SEGMENTS_PER_DIGIT) + (nPos * LEDS_PER_COLON);
	const bool OnOff = (nChar == ':' || nChar == '.' || nChar == ';') ? 1 : 0;

	for (uint32_t cnt = cur_digit_base; cnt < (cur_digit_base + LEDS_PER_COLON); cnt++) {
		if (OnOff) {
			m_pWS28xx->SetLED(cnt, nRed, nGreen, nBlue); // on
		} else {
			m_pWS28xx->SetLED(cnt, 0, 0, 0); // off
		}
	}
}

void DisplayWS28xx::Print(void) {
	printf("Display WS28xx\n");
	printf(" %d Digit(s), %d Colons, %d LED(S)\n", WS28XX_NUM_OF_DIGITS, WS28XX_NUM_OF_COLONS, WS28XX_LED_COUNT);
	printf(" Type    : %s [%d]\n", WS28xx::GetLedTypeString(m_tLedType), m_tLedType);
	printf(" Mapping : %s [%d]\n", RGBMapping::ToString(m_tMapping), m_tMapping);
	printf(" Master  : %d\n", m_nMaster);
}
