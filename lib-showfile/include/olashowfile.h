/**
 * @file olashowfile.h
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

#ifndef OLASHOWFILE_H_
#define OLASHOWFILE_H_

#include <stdio.h>

#include "showfile.h"

enum class OlaParseCode {
	FAILED,
	TIME,
	DMX,
	NONE,
	EOFILE
};

class OlaShowFile final: public ShowFile {
public:
	OlaShowFile();

	void ShowFileStart() override;
	void ShowFileStop() override;
	void ShowFileResume() override;
	void ShowFileRun() override;
	void ShowFilePrint() override {
		puts("OlaShowFile");
	}

private:
	enum class OlaState {
		IDLE,
		PARSING_DMX,
		TIME_WAITING
	};

	OlaParseCode GetNextLine();
	OlaParseCode ParseLine(const char *pLine);
	OlaParseCode ParseDmxData(const char *pLine);

private:
	OlaParseCode m_tParseCode{OlaParseCode::FAILED};
	OlaState m_tState{OlaState::IDLE};
	char s_buffer[2048];
	uint32_t m_nDelayMillis{0};
	uint32_t m_nLastMillis{0};
	uint32_t m_nUniverse{0};
	uint8_t m_DmxData[512];
	uint32_t m_nDmxDataLength{0};
};

#endif /* OLASHOWFILE_H_ */
