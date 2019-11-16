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

#include "hardware.h"
#include "network.h"

#include "debug.h"

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

static const char sRGB[] ALIGNED = "rgb";
#define RGB_LENGTH (sizeof(sRGB)/sizeof(sRGB[0]) - 1)
#define RGB_SIZE_HEX	(7) // 1 byte index followed by 6 bytes hex RGB

static const char sDisplayMSG[] ALIGNED = "showmsg";
#define DMSG_LENGTH (sizeof(sDisplayMSG)/sizeof(sDisplayMSG[0]) - 1)
#define DMSG_SIZE	(11)

enum tUdpPort {
	WS28XX_UDP_PORT = 0x2812
};

DisplayWS28xx *DisplayWS28xx::s_pThis = 0;

DisplayWS28xx::DisplayWS28xx(TWS28XXType tLedType):
	m_tLedType(tLedType),
	m_tMapping(RGB),
	m_nHandle(-1),
	m_nMaster(255),
	m_bShowMsg(false),
	m_nSecondsPrevious(60), // Force update
	m_nColonBlinkMode(COLON_BLINK_MODE_DOWN)
{
	s_pThis = this;
	s_wsticker = 0;
}

DisplayWS28xx::~DisplayWS28xx(void)
{
	delete m_pWS28xx;
	m_pWS28xx = 0;
}

void DisplayWS28xx::Init(uint8_t nIntensity, TWS28xxMapping lMapping)
{
	m_pWS28xx = new WS28xx(m_tLedType, WS28XX_LED_COUNT);
	assert(m_pWS28xx != 0);

	m_pWS28xx->Initialize();
	m_pWS28xx->SetGlobalBrightness(nIntensity);

	m_tMapping = lMapping;

	//m_nMaster = 255;								// Already done in constructor
	//m_nColonBlinkMode = COLON_BLINK_MODE_DOWN;	// Already done in constructor

	//	curR = 255;  // default to full red
	//	curG = 0;
	//	curB = 0;
	segR = 255;
	segG = 0;
	segB = 0;
	colR = 0xff;
	colG = 0xcc;
	colB = 0x00;

	// UDP socket
	m_nHandle = Network::Get()->Begin(WS28XX_UDP_PORT);
	assert(m_nHandle != -1);
}

void DisplayWS28xx::Stop(){
	m_nHandle = Network::Get()->End(WS28XX_UDP_PORT);
}

void DisplayWS28xx::SetMaster(uint8_t value)
{
	m_nMaster = value;
}

// set the current RGB values, remapping them to different LED strip mappings
// TODO move remapping functionality to lib-ws28xx
void DisplayWS28xx::SetRGB(uint8_t red, uint8_t green, uint8_t blue, uint8_t idx)
{
	switch (m_tMapping)
	{
	case RGB:
		curR = red;
		curG = green;
		curB = blue;
		break;

	case RBG:
		curR = red;
		curG = blue;
		curB = green;
		break;

	case BGR:
		curR = blue;
		curG = green;
		curB = red;
		break;

	default:
		curR = red;
		curG = green;
		curB = blue;
		break;
	}

	switch (idx)
	{
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
void DisplayWS28xx::SetRGB(const char *hexstr, uint8_t idx)
{
	uint8_t type = hexstr[0] - '0';
	uint32_t rgb = hexadecimalToDecimal(hexstr + 1);
	uint8_t r = (uint8_t)(rgb >> 16);
	uint8_t g = (uint8_t)(rgb >> 8);
	uint8_t b = (uint8_t)rgb & 0xff;
	SetRGB(r, g, b, type);
}

void DisplayWS28xx::Run() {
	uint32_t nIPAddressFrom;
	uint16_t nForeignPort;
	uint16_t m_nBytesReceived;

	m_nMillis = Hardware::Get()->Millis(); // millis now

	if (m_nMillis >= s_wsticker) {
		s_wsticker = m_nMillis + WS28XX_UPDATE_MS;

		// temporary messages.
		if (m_nMillis > s_msgTimer)
			m_bShowMsg = 0;
		else if (m_bShowMsg)
			ShowMessage(m_aMessage);
	}

	// UDP Socket
	m_nBytesReceived = Network::Get()->RecvFrom(m_nHandle, (uint8_t *) &m_Buffer, (uint16_t) sizeof(m_Buffer), &nIPAddressFrom, &nForeignPort);

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
			int nMlength = m_nBytesReceived - (5 + DMSG_LENGTH + 1);
			printf("RX: %d  MsgLen: %d  Msg: %.*s\n",m_nBytesReceived, nMlength, nMlength, &m_Buffer[(5 + DMSG_LENGTH + 1)]);
			if ( ((nMlength > 0) && (nMlength <= DMSG_SIZE)) && (m_Buffer[5 + DMSG_LENGTH] == '#')) {
				SetMessage((const char *)&m_Buffer[(5 + DMSG_LENGTH + 1)],nMlength);
			}

	} else if (memcmp(&m_Buffer[5], sRGB, RGB_LENGTH) == 0) {
			if ((m_nBytesReceived == (5 + RGB_LENGTH + 1 + RGB_SIZE_HEX))  && (m_Buffer[5 + RGB_LENGTH] == '#')) {
				SetRGB((const char *)&m_Buffer[(5 + RGB_LENGTH + 1)]);
			} else {
				DEBUG_PUTS("Invalid !rgb command");
			}
		}

	 else {
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

			if (m_nSecondsPrevious != pTimecode[7]) // seconds have changed
			{
				ms_colon_blink = m_nMillis + 1000;
				m_nSecondsPrevious = pTimecode[7];
				outR = 0;				outG = 0;				outB = 0;
			}
			else if (m_nMillis < ms_colon_blink)
			{
				outR = (((float)(ms_colon_blink - m_nMillis) / 1000) * abs(mColonBlinkOffset - colR));
				outG = (((float)(ms_colon_blink - m_nMillis) / 1000) * abs(mColonBlinkOffset - colG));
				outB = (((float)(ms_colon_blink - m_nMillis) / 1000) * abs(mColonBlinkOffset - colB));
			}
		}
		else
		{
			// straight thru
			outR = colR; 			outG = colG;			outB = colB;
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

	s_msgTimer = Hardware::Get()->Millis() + 3000; // 3 seconds from now
	m_bShowMsg = true;
}


void DisplayWS28xx::ShowMessage(const char *pMessage) {
	assert(pMessage != 0);

	uint8_t oR = 0, oG = 0, oB = 0;

	if (m_nMillis <= s_msgTimer)
	{
		{
			oR = (((float)(s_msgTimer - m_nMillis) / WS82XX_MSG_TIME_MS) * msgR);
			oG = (((float)(s_msgTimer - m_nMillis) / WS82XX_MSG_TIME_MS) * msgG);
			oB = (((float)(s_msgTimer - m_nMillis) / WS82XX_MSG_TIME_MS) * msgB);
		}
	}
	else
	{
		// straight thru
		oR = msgR; 		oG = msgG;		oB = msgB;
	}

	for (int cnt = 0; (cnt < WS28XX_MAX_MSG_SIZE) && (cnt < WS28XX_NUM_OF_DIGITS); cnt++)
	{
		if (pMessage[cnt] != 0)
			WriteChar(pMessage[cnt], cnt, oR, oG, oB);
		else
			WriteChar(' ', cnt);
	}

	// blank colons
	for (int cnt = 0; cnt < WS28XX_NUM_OF_COLONS; cnt++)
		WriteColon(' ', cnt, 0, 0, 0); // 1st :

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
	printf(" Type  : %s [%d]\n", WS28xx::GetLedTypeString(m_tLedType), m_tLedType);
}
