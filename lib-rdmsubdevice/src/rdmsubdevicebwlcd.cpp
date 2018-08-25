#if defined (HAVE_SPI)
/**
 * @file rdmsubdevicebwlcd.cpp
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#ifndef NDEBUG
 #include <stdio.h>
#endif

#include "util.h"

#include "rdmsubdevicebwlcd.h"

#ifndef TO_HEX
 #define TO_HEX(i)		((i) < 10) ? (char)'0' + (char)(i) : (char)'A' + (char)((i) - 10)
#endif

#ifndef MIN
 #define MIN(a,b)		(((a) < (b)) ? (a) : (b))
#endif

#include "bw_spi_lcd.h"

#define DMX_FOOTPRINT	4

static RDMPersonality *s_RDMPersonalities[] = {new RDMPersonality("LCD 4-slots H", DMX_FOOTPRINT), new RDMPersonality("LCD 4-slots D", DMX_FOOTPRINT), new RDMPersonality("LCD 4-slots \%", DMX_FOOTPRINT)};

const char s_aLine[BW_LCD_MAX_CHARACTERS + 1] ALIGNED = "--- --- --- --- ";

RDMSubDeviceBwLcd::RDMSubDeviceBwLcd(uint16_t nDmxStartAddress, char nChipSselect, uint8_t nSlaveAddress, uint32_t nSpiSpeed):
	RDMSubDevice("bw_spi_lcd", nDmxStartAddress),
	m_IsStarted(false),
	m_nLength(0)
{
	m_tDeviceInfo.chip_select = (spi_cs_t) nChipSselect;
	m_tDeviceInfo.slave_address = nSlaveAddress;
	m_tDeviceInfo.speed_hz = nSpiSpeed;

	SetDmxFootprint(DMX_FOOTPRINT);
	SetPersonalities(s_RDMPersonalities, 3);

	for (unsigned i = 0; i < BW_LCD_MAX_CHARACTERS; i++) {
		m_aText[i] = ' ';
	}

	for (unsigned i = 0; i < sizeof(m_Data); i++) {
		m_Data[i] = 0;
	}
}

RDMSubDeviceBwLcd::~RDMSubDeviceBwLcd(void) {
}

bool RDMSubDeviceBwLcd::Initialize(void) {
	const bool IsConnected = bw_spi_lcd_start(&m_tDeviceInfo);

	if (IsConnected) {
		for (unsigned i = 0; i < BW_LCD_MAX_CHARACTERS; i++) {
			m_aText[i] = ' ';
		}
		bw_spi_lcd_text_line_1(&m_tDeviceInfo, m_aText, BW_LCD_MAX_CHARACTERS);
		bw_spi_lcd_text_line_2(&m_tDeviceInfo, m_aText, BW_LCD_MAX_CHARACTERS);
	}

#ifndef NDEBUG
	printf("%s:%s IsConnected=%d\n", __FILE__, __FUNCTION__, (int) IsConnected);
#endif

	return IsConnected;
}

void RDMSubDeviceBwLcd::Start(void) {
	if (m_IsStarted) {
		return;
	}

	m_IsStarted = true;

	DisplayChannels();

	for (unsigned i = 0; i < BW_LCD_MAX_CHARACTERS; i++) {
		m_aText[i] = ' ';
	}

	DisplayUpdatePersonality();

	bw_spi_lcd_text_line_2(&m_tDeviceInfo, m_aText, BW_LCD_MAX_CHARACTERS);
}

void RDMSubDeviceBwLcd::Stop(void) {
	if(!m_IsStarted) {
		return;
	}

	m_IsStarted = false;

	bw_spi_lcd_text_line_2(&m_tDeviceInfo, s_aLine, BW_LCD_MAX_CHARACTERS - 1); // Leave H, D, % at the end
}

void RDMSubDeviceBwLcd::Data(const uint8_t* pData, uint16_t nLength) {
	const uint16_t nDmxStartAddress = GetDmxStartAddress();
	bool IsDataChanged = false;

	nLength = MIN(nLength, DMX_FOOTPRINT);
	nLength = MIN(nLength, 513 - nDmxStartAddress);

	const uint8_t* p = &pData[nDmxStartAddress-1];

	for (unsigned i = 0; (i < sizeof(m_Data)) && (i < nLength); i++) {
		if (m_Data[i] != p[i]) {
			IsDataChanged = true;
		}
		m_Data[i] = p[i];
	}

	if (!IsDataChanged) {
		return;
	}

	m_nLength = nLength;

	switch (GetPersonalityCurrent()) {
		case 1:
			DataHex(p, nLength);
			break;
		case 2:
			DataDec(p, nLength);
			break;
		case 3:
			DataPct(p, nLength);
			break;
		default:
			break;
	}

	bw_spi_lcd_text_line_2(&m_tDeviceInfo, m_aText, BW_LCD_MAX_CHARACTERS - 1);
}

void RDMSubDeviceBwLcd::DisplayChannels(void) {
	char text[BW_LCD_MAX_CHARACTERS];
	const uint16_t nDmxStartAddress = GetDmxStartAddress();
	unsigned i;

	for (i = 0; (i < DMX_FOOTPRINT) && (nDmxStartAddress + i) <= 512; i++) {
		unsigned nOffset = i * 4;
		text[nOffset] = ' ';
		text[nOffset + 1] = ' ';
		text[nOffset + 2] = ' ';
		text[nOffset + 3] = ' ';

		itoaBase10(nDmxStartAddress + i, &text[nOffset]);
	}

	for (; i < DMX_FOOTPRINT; i++) {
		unsigned nOffset = i * 4;
		text[nOffset] = ' ';
		text[nOffset + 1] = ' ';
		text[nOffset + 2] = ' ';
		text[nOffset + 3] = ' ';
	}

	bw_spi_lcd_text_line_1(&m_tDeviceInfo, text, BW_LCD_MAX_CHARACTERS);
}

void RDMSubDeviceBwLcd::DataHex(const uint8_t* pData, uint16_t nLength) {
	unsigned j;

	for (j = 0; j < nLength ; j++) {
		unsigned nOffset = j * 4;
		const uint8_t nData = pData[j];
		m_aText[nOffset    ] = ' ';
		m_aText[nOffset + 1] = TO_HEX((nData & 0xF0) >> 4);
		m_aText[nOffset + 2] = TO_HEX(nData & 0x0F);
	}

	for (; j < DMX_FOOTPRINT; j++) {
		unsigned nOffset = j * 4;
		m_aText[nOffset] = ' ';
		m_aText[nOffset + 1] = ' ';
		m_aText[nOffset + 2] = ' ';
	}
}

void RDMSubDeviceBwLcd::DataDec(const uint8_t* pData, uint16_t nLength) {
	unsigned j;

	for (j = 0; j < nLength ; j++) {
		unsigned nOffset = j * 4;
		m_aText[nOffset] = ' ';
		m_aText[nOffset + 1] = ' ';
		itoaBase10(pData[j], &m_aText[nOffset]);
	}

	for (; j < DMX_FOOTPRINT; j++) {
		unsigned nOffset = j * 4;
		m_aText[nOffset] = ' ';
		m_aText[nOffset + 1] = ' ';
		m_aText[nOffset + 2] = ' ';
	}
}

void RDMSubDeviceBwLcd::DataPct(const uint8_t* pData, uint16_t nLength) {
	unsigned j;

	for (j = 0; j < nLength ; j++) {
		unsigned nOffset = j * 4;
		m_aText[nOffset] = ' ';
		m_aText[nOffset + 1] = ' ';
		const uint16_t pct = ((double) pData[j] / 255) * 100;
		itoaBase10(pct , &m_aText[nOffset]);
	}

	for (; j < DMX_FOOTPRINT; j++) {
		unsigned nOffset = j * 4;
		m_aText[nOffset] = ' ';
		m_aText[nOffset + 1] = ' ';
		m_aText[nOffset + 2] = ' ';
	}
}

void RDMSubDeviceBwLcd::itoaBase10(uint16_t arg, char buf[]) {
	char *n = buf + 2;

	if (arg == 0) *n = '0';

	while (arg != 0) {
		*n = (char)'0' + (char)(arg % 10);
		n--;
		arg /= 10;
	}
}

void RDMSubDeviceBwLcd::UpdateEvent(TRDMSubDeviceUpdateEvent tUpdateEvent) {
	if (tUpdateEvent == RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS) {
		DisplayChannels();
		for (unsigned i = 0; i < BW_LCD_MAX_CHARACTERS - 1; i++) { // Leave H, D, % at the end
			m_aText[i] = ' ';
		}
		bw_spi_lcd_text_line_2(&m_tDeviceInfo, m_aText, BW_LCD_MAX_CHARACTERS - 1); // Leave H, D, % at the end
		return;
	} else if (tUpdateEvent == RDM_SUBDEVICE_UPDATE_EVENT_PERSONALITY) {
		DisplayUpdatePersonality();
		if (m_aText[2] != ' ') {
			switch (GetPersonalityCurrent()) {
			case 1:
				DataHex(m_Data, m_nLength);
				break;
			case 2:
				DataDec(m_Data, m_nLength);
				break;
			case 3:
				DataPct(m_Data, m_nLength);
				break;
			default:
				break;
			}
		}

		bw_spi_lcd_text_line_2(&m_tDeviceInfo, m_aText, BW_LCD_MAX_CHARACTERS);
	}
}

void RDMSubDeviceBwLcd::DisplayUpdatePersonality(void) {
	const uint8_t PersonalityCurrent = GetPersonalityCurrent();

	switch (PersonalityCurrent) {
	case 1:
		m_aText[15] = 'H';
		break;
	case 2:
		m_aText[15] = 'D';
		break;
	case 3:
		m_aText[15] = '%';
		break;
	default:
		break;
	}
}
#endif
