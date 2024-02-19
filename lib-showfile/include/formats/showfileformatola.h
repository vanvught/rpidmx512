/**
 * @file showfileformatola.h
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef FORMATS_SHOWFILEFORMATOLA_H_
#define FORMATS_SHOWFILEFORMATOLA_H_

#include <cstdio>

#include "showfileprotocol.h"
#include "showfileconst.h"

#include "debug.h"

#define SHOWFILE_PREFIX	"show"
#define SHOWFILE_SUFFIX	".txt"

namespace showfile {
static constexpr uint32_t FILE_NAME_LENGTH = sizeof(SHOWFILE_PREFIX "NN" SHOWFILE_SUFFIX) - 1U;
static constexpr uint32_t FILE_MAX_NUMBER = 99;
}  // namespace showfile

class ShowFileFormat: public ShowFileProtocol {
public:
	ShowFileFormat() {
		DEBUG_ENTRY

		DEBUG_EXIT
	}

	void ShowFileStart() {
		DEBUG_ENTRY

		m_nDelayMillis = 0;
		m_nLastMillis = 0;

		fseek(m_pShowFile, 0L, SEEK_SET);

		m_OlaState = OlaState::IDLE;

		DEBUG_EXIT
	}

	void ShowFileStop() {
		DEBUG_ENTRY

		DEBUG_EXIT
	}

	void ShowFileResume() {
		DEBUG_ENTRY

		m_nDelayMillis = 0;
		m_nLastMillis = 0;

		DEBUG_EXIT
	}

	void ShowFilePrint() {
		puts(" Format: OLA");
	}

	void ShowFileRun();

protected:
	uint32_t m_nShowFileCurrent { showfile::FILE_MAX_NUMBER + 1 };
	bool m_bDoLoop { false };
	FILE *m_pShowFile { nullptr };

private:
	enum class OlaState {
		IDLE, PARSING_DMX, TIME_WAITING
	};

	enum class OlaParseCode {
		FAILED, TIME, DMX, NONE, EOFILE
	};

	OlaParseCode GetNextLine();
	OlaParseCode ParseLine(const char *pLine);
	OlaParseCode ParseDmxData(const char *pLine);

private:
	OlaParseCode m_OlaParseCode { OlaParseCode::FAILED };
	OlaState m_OlaState { OlaState::IDLE };
	char m_buffer[2048];
	uint32_t m_nDelayMillis { 0 };
	uint32_t m_nLastMillis { 0 };
	uint32_t m_nDmxDataLength { 0 };
	uint16_t m_nUniverse { 0 };
	uint8_t m_DmxData[512];
};

#endif /* FORMATS_SHOWFILEFORMATOLA_H_ */
