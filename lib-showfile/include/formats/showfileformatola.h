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
#include <cassert>

#include "showfileprotocol.h"
#include "showfileconst.h"

#include "debug.h"

#define SHOWFILE_PREFIX	"show"
#define SHOWFILE_SUFFIX	".txt"

namespace showfile {
static constexpr uint32_t FILE_NAME_LENGTH = sizeof(SHOWFILE_PREFIX "NN" SHOWFILE_SUFFIX) - 1U;
static constexpr uint32_t FILE_MAX_NUMBER = 99;
}  // namespace showfile

class ShowFileFormat: ShowFileProtocol {
public:
	ShowFileFormat() {
		DEBUG_ENTRY

		assert(s_pThis == nullptr);
		s_pThis = this;

		DEBUG_EXIT
	}

	void ShowFileStart() {
		DEBUG_ENTRY

		m_nDelayMillis = 0;
		m_nLastMillis = 0;

		fseek(m_pShowFile, 0L, SEEK_SET);

		m_OlaState = OlaState::IDLE;

		ShowFileProtocol::Start();

		DEBUG_EXIT
	}

	void ShowFileStop() {
		DEBUG_ENTRY

		ShowFileProtocol::Stop();

		DEBUG_EXIT
	}

	void ShowFileResume() {
		DEBUG_ENTRY

		m_nDelayMillis = 0;
		m_nLastMillis = 0;

		DEBUG_EXIT
	}

	void ShowFileRecord() {
		DEBUG_ENTRY

		if (m_pShowFile != nullptr) {
			fputs("OLA Show\n", m_pShowFile);
			m_OlaState = OlaState::RECORD_FIRST;
		} else {
			m_OlaState = OlaState::IDLE;
		}

		ShowFileProtocol::Record();

		DEBUG_EXIT
	}

	void ShowFilePrint() {
		puts(" Format: OLA");
		ShowFileProtocol::Print();
	}

	void ShowFileRun();

	void DoRunCleanupProcess(const bool bDoRun) {
		ShowFileProtocol::DoRunCleanupProcess(bDoRun);
	}

	void ShowfileWrite([[maybe_unused]] const uint8_t *pDmxData,[[maybe_unused]] const uint32_t nSize, const uint32_t nUniverse, const uint32_t nMillis) {
		fputs(itoa_with_linefeed(nUniverse), m_pShowFile);

		if (m_OlaState == OlaState::RECORD_FIRST) {
			m_OlaState = OlaState::RECORDING;
		} else {
			fputs(itoa_with_linefeed(m_nLastMillis - nMillis), m_pShowFile);
		}

		m_nLastMillis = nMillis;
	}

	void BlackOut() {
#if defined (SHOWFILE_ENABLE_DMX_MASTER)
		ShowFileProtocol::DmxBlackout();
#endif
	}

	void SetMaster([[maybe_unused]] const uint32_t nMaster) {
#if defined (SHOWFILE_ENABLE_DMX_MASTER)
		ShowFileProtocol::DmxMaster(nMaster);
#endif
	}

	bool IsSyncDisabled() {
		return ShowFileProtocol::IsSyncDisabled();
	}

	static ShowFileFormat *Get() {
		return s_pThis;
	}

private:
#define UINT_DIGITS 12
	char *itoa_with_linefeed(uint32_t i) {
		m_buffer[UINT_DIGITS + 1] = '\n';
		m_buffer[UINT_DIGITS + 2] = '\0';
		char *p = m_buffer + UINT_DIGITS;
		do {
			*--p = '0' + static_cast<char>(i % 10);
			i /= 10;
		} while (i != 0);
		return p;
	}

protected:
	uint32_t m_nShowFileCurrent { showfile::FILE_MAX_NUMBER + 1 };
	bool m_bDoLoop { false };
	FILE *m_pShowFile { nullptr };

private:
	enum class OlaState {
		IDLE, PARSING_DMX, TIME_WAITING, RECORD_FIRST, RECORDING
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

	static ShowFileFormat *s_pThis;
};

#endif /* FORMATS_SHOWFILEFORMATOLA_H_ */
