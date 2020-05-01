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
#include <assert.h>

#include "olashowfile.h"
#include "showfile.h"

#include "hardware.h"

#include "debug.h"

OlaShowFile::OlaShowFile(void) :
	m_tParseCode(PARSE_FAILED),
	m_tState(STATE_IDLE),
	m_nDelayMillis(0),
	m_nLastMillis(0),
	m_nUniverse(0),
	m_nDmxDataLength(0)
{
	DEBUG1_ENTRY

	DEBUG1_EXIT
}

OlaShowFile::~OlaShowFile(void) {
	DEBUG1_ENTRY

	DEBUG1_EXIT
}

void OlaShowFile::ShowFileStart(void) {
	DEBUG1_ENTRY

	m_nDelayMillis = 0;
	m_nLastMillis = 0;

	static_cast<void>(fseek(m_pShowFile, 0L, SEEK_SET));

	m_tState = STATE_IDLE;

	DEBUG1_EXIT
}

void OlaShowFile::ShowFileStop(void) {
	DEBUG1_ENTRY

	DEBUG1_EXIT
}

void OlaShowFile::ShowFileResume(void) {
	DEBUG1_ENTRY

	m_nDelayMillis = 0;
	m_nLastMillis = 0;

	DEBUG1_EXIT
}

void OlaShowFile::ShowFileRun(void) {
	if (m_tState != STATE_TIME_WAITING) {
		m_tParseCode = GetNextLine();

		if (m_tParseCode == PARSE_DMX) {
			if (m_nDmxDataLength != 0) {
				m_pShowFileProtocolHandler->DmxOut(m_nUniverse, m_DmxData, m_nDmxDataLength);
			}
		} else if (m_tParseCode == PARSE_TIME) {
			if (m_nDelayMillis != 0) {
				if (m_nDmxDataLength != 0) {
					m_pShowFileProtocolHandler->DmxSync();
				}
			}
			m_tState = STATE_TIME_WAITING;
		} else if (m_tParseCode == PARSE_EOF) {
			if (m_bDoLoop) {
				static_cast<void>(fseek(m_pShowFile, 0L, SEEK_SET));
			} else {
				SetShowFileStatus(SHOWFILE_STATUS_ENDED);
			}
		}
	}

	const uint32_t nMillis = Hardware::Get()->Millis();

	if ((nMillis - m_nLastMillis) >= m_nDelayMillis) {
		m_nLastMillis = nMillis;
		m_tState = STATE_PARSING_DMX;
	}
}

ParseCode OlaShowFile::ParseDmxData(const char *pLine) {
	char *p = const_cast<char *>(pLine);
	int64_t k = 0;
	uint32_t nLength = 0;

	while (isdigit(*p) == 1) {
		k = k * 10 + *p - '0';

		if (k > 255) {
			DEBUG1_EXIT
			return PARSE_FAILED;
		}

		p++;

		if (*p == ',' || (isdigit(*p) == 0)) {

			if (nLength > 512) {
				DEBUG1_EXIT
				return PARSE_FAILED;
			}

			m_DmxData[nLength] = k;

			k = 0;
			nLength++;
			p++;
		}
	}

	m_nDmxDataLength  = nLength;

	return PARSE_DMX;
}

enum ParseCode OlaShowFile::ParseLine(const char *pLine) {
	char *p = const_cast<char *>(pLine);
	int32_t k = 0;

	while (isdigit(*p) == 1) {
		k = k * 10 + *p - '0';
		p++;
	}

	if (k > static_cast<int32_t>((static_cast<uint16_t>(~0)))) {
		return PARSE_FAILED;
	}

	if (*p++ == ' ') {
		m_nDelayMillis = 0;
		m_nUniverse = k;
		return ParseDmxData(p);
	}

	m_nDelayMillis = k;

	return PARSE_TIME;
}

ParseCode OlaShowFile::GetNextLine(void) {
	if (m_pShowFile != NULL) {
		if (fgets(s_buffer, (sizeof(s_buffer) - 1), m_pShowFile) != s_buffer) {
			return PARSE_EOF;
		}

		if (isdigit(s_buffer[0])) {
			return ParseLine(s_buffer);
		}
	}

	return PARSE_FAILED;
}
