/**
 * @file olashowfile.cpp
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

#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <cassert>

#include "olashowfile.h"
#include "showfile.h"

#include "hardware.h"

#include "debug.h"

OlaShowFile::OlaShowFile() {
	DEBUG1_ENTRY

	DEBUG1_EXIT
}

void OlaShowFile::ShowFileStart() {
	DEBUG1_ENTRY

	m_nDelayMillis = 0;
	m_nLastMillis = 0;

	fseek(m_pShowFile, 0L, SEEK_SET);

	m_tState = OlaState::IDLE;

	DEBUG1_EXIT
}

void OlaShowFile::ShowFileStop() {
	DEBUG1_ENTRY

	DEBUG1_EXIT
}

void OlaShowFile::ShowFileResume() {
	DEBUG1_ENTRY

	m_nDelayMillis = 0;
	m_nLastMillis = 0;

	DEBUG1_EXIT
}

void OlaShowFile::ShowFileRun() {
	if (m_tState != OlaState::TIME_WAITING) {
		m_tParseCode = GetNextLine();

		if (m_tParseCode == OlaParseCode::DMX) {
			if (m_nDmxDataLength != 0) {
				m_pShowFileProtocolHandler->DmxOut(m_nUniverse, m_DmxData, m_nDmxDataLength);
			}
		} else if (m_tParseCode == OlaParseCode::TIME) {
			if (m_nDelayMillis != 0) {
				if (m_nDmxDataLength != 0) {
					m_pShowFileProtocolHandler->DmxSync();
				}
			}
			m_tState = OlaState::TIME_WAITING;
		} else if (m_tParseCode == OlaParseCode::EOFILE) {
			if (m_bDoLoop) {
				fseek(m_pShowFile, 0L, SEEK_SET);
			} else {
				SetShowFileStatus(ShowFileStatus::ENDED);
			}
		}
	}

	const uint32_t nMillis = Hardware::Get()->Millis();

	if ((nMillis - m_nLastMillis) >= m_nDelayMillis) {
		m_nLastMillis = nMillis;
		m_tState = OlaState::PARSING_DMX;
	}
}

OlaParseCode OlaShowFile::ParseDmxData(const char *pLine) {
	char *p = const_cast<char *>(pLine);
	int64_t k = 0;
	uint32_t nLength = 0;

	while (isdigit(*p) == 1) {
		k = k * 10 + *p - '0';

		if (k > 255) {
			DEBUG1_EXIT
			return OlaParseCode::FAILED;
		}

		p++;

		if (*p == ',' || (isdigit(*p) == 0)) {

			if (nLength > 512) {
				DEBUG1_EXIT
				return OlaParseCode::FAILED;
			}

			m_DmxData[nLength] = k;

			k = 0;
			nLength++;
			p++;
		}
	}

	m_nDmxDataLength  = nLength;

	return OlaParseCode::DMX;
}

OlaParseCode OlaShowFile::ParseLine(const char *pLine) {
	char *p = const_cast<char*>(pLine);
	int32_t k = 0;

	while (isdigit(*p) == 1) {
		k = k * 10 + *p - '0';
		p++;
	}

	if (k > static_cast<int32_t>((static_cast<uint16_t>(~0)))) {
		return OlaParseCode::FAILED;
	}

	if (*p++ == ' ') {
		m_nDelayMillis = 0;
		m_nUniverse = static_cast<uint32_t>(k);
		return ParseDmxData(p);
	}

	m_nDelayMillis = static_cast<uint32_t>(k);

	return OlaParseCode::TIME;
}

OlaParseCode OlaShowFile::GetNextLine() {
	if (m_pShowFile != nullptr) {
		if (fgets(s_buffer, (sizeof(s_buffer) - 1), m_pShowFile) != s_buffer) {
			return OlaParseCode::EOFILE;
		}

		if (isdigit(s_buffer[0])) {
			return ParseLine(s_buffer);
		}
	}

	return  OlaParseCode::FAILED;
}
