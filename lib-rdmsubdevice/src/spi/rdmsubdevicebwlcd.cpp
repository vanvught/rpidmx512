/**
 * @file rdmsubdevicebwlcd.cpp
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <algorithm>
#include <cstdint>
#include <cstring>

#include "cstdio"

#include "spi/rdmsubdevicebwlcd.h"

#include "bwspilcd.h"

#ifndef TO_HEX
# define TO_HEX(i)	static_cast<char>(((i) < 10) ? '0' + (i) : 'A' + ((i) - 10))
#endif

static constexpr uint32_t DMX_FOOTPRINT = 4;
static RDMPersonality *s_RDMPersonalities[] = {new RDMPersonality("LCD 4-slots H", DMX_FOOTPRINT), new RDMPersonality("LCD 4-slots D", DMX_FOOTPRINT), new RDMPersonality("LCD 4-slots %%", DMX_FOOTPRINT)};
static constexpr char s_aLine[bw::lcd::max_characters + 1] = "--- --- --- --- ";

RDMSubDeviceBwLcd::RDMSubDeviceBwLcd(uint16_t nDmxStartAddress, char nChipSselect, uint8_t nSlaveAddress, [[maybe_unused]] uint32_t nSpiSpeed):
	RDMSubDevice("bw_spi_lcd", nDmxStartAddress), m_BwSpiLcd(nChipSselect, nSlaveAddress)
{
	SetDmxFootprint(DMX_FOOTPRINT);
	SetPersonalities(s_RDMPersonalities, 3);

	memset(m_aText, ' ', sizeof(m_aText));
	memset(m_Data, 0, sizeof(m_Data));
}

bool RDMSubDeviceBwLcd::Initialize() {
	if (m_BwSpiLcd.IsConnected()) {
		memset(m_aText, ' ', sizeof(m_aText));
		m_BwSpiLcd.TextLine(0, m_aText, sizeof(m_aText));
		m_BwSpiLcd.TextLine(1, m_aText, sizeof(m_aText));
		return true;
	}
	return false;
}

void RDMSubDeviceBwLcd::Start() {
	if (m_IsStarted) {
		return;
	}

	m_IsStarted = true;

	DisplayChannels();

	memset(m_aText, ' ', sizeof(m_aText));

	DisplayUpdatePersonality();

	m_BwSpiLcd.TextLine(1, m_aText, bw::lcd::max_characters);
}

void RDMSubDeviceBwLcd::Stop() {
	if(!m_IsStarted) {
		return;
	}

	m_IsStarted = false;
	m_BwSpiLcd.TextLine(1, s_aLine, bw::lcd::max_characters - 1); // Leave H, D, % at the end
}

void RDMSubDeviceBwLcd::Data(const uint8_t* pData, uint32_t nLength) {
	const uint16_t nDmxStartAddress = GetDmxStartAddress();
	bool IsDataChanged = false;

	nLength = std::min(nLength, DMX_FOOTPRINT);
	nLength = std::min(nLength, static_cast<uint32_t>(513U - nDmxStartAddress));

	const auto* p = &pData[nDmxStartAddress-1];

	for (uint32_t i = 0; (i < sizeof(m_Data)) && (i < nLength); i++) {
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

	m_BwSpiLcd.TextLine(1, m_aText, bw::lcd::max_characters - 1);
}

void RDMSubDeviceBwLcd::DisplayChannels() {
	char text[bw::lcd::max_characters];
	const uint16_t nDmxStartAddress = GetDmxStartAddress();
	unsigned i;

	for (i = 0; (i < DMX_FOOTPRINT) && (nDmxStartAddress + i) <= 512; i++) {
		unsigned nOffset = i * 4;
		text[nOffset] = ' ';
		text[nOffset + 1] = ' ';
		text[nOffset + 2] = ' ';
		text[nOffset + 3] = ' ';

		itoaBase10(static_cast<uint16_t>(nDmxStartAddress + i), &text[nOffset]);
	}

	for (; i < DMX_FOOTPRINT; i++) {
		unsigned nOffset = i * 4;
		text[nOffset] = ' ';
		text[nOffset + 1] = ' ';
		text[nOffset + 2] = ' ';
		text[nOffset + 3] = ' ';
	}

	m_BwSpiLcd.TextLine(0, text, bw::lcd::max_characters);
}

void RDMSubDeviceBwLcd::DataHex(const uint8_t* pData, uint32_t nLength) {
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

void RDMSubDeviceBwLcd::DataDec(const uint8_t* pData, uint32_t nLength) {
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

void RDMSubDeviceBwLcd::DataPct(const uint8_t* pData, uint32_t nLength) {
	unsigned j;

	for (j = 0; j < nLength ; j++) {
		unsigned nOffset = j * 4;
		m_aText[nOffset] = ' ';
		m_aText[nOffset + 1] = ' ';
		const auto pct = static_cast<uint16_t>((pData[j] / 255) * 100);
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
		*n = static_cast<char>('0' + (arg % 10));
		n--;
		arg /= 10;
	}
}

void RDMSubDeviceBwLcd::UpdateEvent(TRDMSubDeviceUpdateEvent tUpdateEvent) {
	if (tUpdateEvent == RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS) {
		DisplayChannels();

		for (uint32_t i = 0; i < bw::lcd::max_characters - 1; i++) { // Leave H, D, % at the end
			m_aText[i] = ' ';
		}

		m_BwSpiLcd.TextLine(1, m_aText, bw::lcd::max_characters - 1); // Leave H, D, % at the end
		return;
	}

	if (tUpdateEvent == RDM_SUBDEVICE_UPDATE_EVENT_PERSONALITY) {
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

		m_BwSpiLcd.TextLine(1, m_aText, bw::lcd::max_characters);
	}
}

void RDMSubDeviceBwLcd::DisplayUpdatePersonality() {
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
