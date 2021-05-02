/**
 * @file dmxserialchanneldata.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dmxserialchanneldata.h"

#include "debug.h"

static char s_buffer[2048] __attribute__ ((aligned (4)));

DmxSerialChannelData::DmxSerialChannelData() {
	DEBUG_ENTRY

	for (uint32_t i = 0; i < sizeof(m_pChannelData) / sizeof(m_pChannelData[0]); i++) {
		m_nChannelDataLength[i] = 0;
		m_pChannelData[i] = nullptr;
	}

	DEBUG_EXIT
}

DmxSerialChannelData::~DmxSerialChannelData() {
	for (uint32_t i = 0; i < sizeof(m_pChannelData) / sizeof(m_pChannelData[0]); i++) {
		if (m_pChannelData[i] != nullptr) {
			DEBUG_PRINTF("m_pChannelData[%d]", i);
			delete[] m_pChannelData[i];
		}
	}
}

void DmxSerialChannelData::Clear() {
}

bool DmxSerialChannelData::Parse(const char *pFileName) {
	DEBUG_ENTRY

	m_pFile = fopen(pFileName, "r");

	if (m_pFile == nullptr) {
		perror(const_cast<char *>(pFileName));
		DEBUG_EXIT
		return false;
	}

	DmxSerialParseCode tParseCode = DmxSerialParseCode::NONE;

	while (tParseCode != DmxSerialParseCode::EOFILE) {
		tParseCode = GetNextLine();

		if (tParseCode == DmxSerialParseCode::FAILED) {
			break;
		}
	}

	if (fclose(m_pFile) != 0) {
		perror("fclose");
	}

	DEBUG_EXIT
	return true;
}

const uint8_t *DmxSerialChannelData::GetData(uint8_t nChannelValue, uint32_t &nLength) {
	if (m_pChannelData[nChannelValue] != nullptr) {
		nLength = m_nChannelDataLength[nChannelValue];
		return m_pChannelData[nChannelValue];
	}

	nLength = 0;
	return nullptr;
}

void DmxSerialChannelData::Dump() {
#ifndef NDEBUG
	for (uint32_t i = 0; i < sizeof(m_pChannelData) /  sizeof(m_pChannelData[0]); i++) {
		if (m_pChannelData[i] != nullptr) {
			printf("[%d]:%d ", i, m_nChannelDataLength[i]);
			for (uint32_t j = 0; j <  m_nChannelDataLength[i]; j++) {
				printf("%d ", m_pChannelData[i][j]);
			}
			printf("\n");
		}
	}
#endif
}

DmxSerialParseCode DmxSerialChannelData::GetNextLine() {
	if (fgets(s_buffer, static_cast<int>(sizeof(s_buffer) - 1), m_pFile) != s_buffer) {
		return DmxSerialParseCode::EOFILE;
	}

	if (isdigit(static_cast<int>(s_buffer[0]))) {
		return ParseLine(s_buffer);
	}

	return DmxSerialParseCode::NONE;
}

DmxSerialParseCode DmxSerialChannelData::ParseLine(const char *pLine) {
	char *p = const_cast<char *>(pLine);
	int32_t k = 0;

	while (isdigit(*p) == 1) {
		k = k * 10 + *p - '0';
		p++;
	}

	if (k > static_cast<int32_t>((static_cast<uint8_t>(~0)))) {
		return DmxSerialParseCode::FAILED;
	}

	if (*p++ == ' ') {
		m_nChannelValue = k;
		m_pChannelData[m_nChannelValue] = new uint8_t[64];
		return ParseSerialData(p);
	}

	return DmxSerialParseCode::FAILED;
}

DmxSerialParseCode DmxSerialChannelData::ParseSerialData(const char *pLine) {
	char *p = const_cast<char *>(pLine);
	int64_t k = 0;
	uint32_t nLength = 0;

	while (isdigit(*p) == 1) {
		k = k * 10 + *p - '0';

		if (k > 255) {
			DEBUG1_EXIT
			return DmxSerialParseCode::FAILED;
		}

		p++;

		if (*p == ',' || (isdigit(*p) == 0)) {

			if (nLength > 512) {
				DEBUG1_EXIT
				return DmxSerialParseCode::FAILED;
			}

			m_pChannelData[m_nChannelValue][nLength] = k;

			if (nLength == 63) {
				break;
			}

			k = 0;
			nLength++;
			p++;
		}
	}

	m_nChannelDataLength[m_nChannelValue]  = nLength;

	return DmxSerialParseCode::SERIAL;
}
