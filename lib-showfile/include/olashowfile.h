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

enum TState {
	STATE_IDLE,
	STATE_PARSING_DMX,
	STATE_TIME_WAITING
};

enum ParseCode {
	PARSE_FAILED,
	PARSE_TIME,
	PARSE_DMX,
	PARSE_NONE,
	PARSE_EOF
};

class OlaShowFile: public ShowFile {
public:
	OlaShowFile(void);
	~OlaShowFile(void);

	void Start(void);
	void Stop(void);
	void Resume(void);

	void Process(void);

	void Print(void) {
		ShowFile::Print();
		puts("OlaShowFile");
	}

private:
	ParseCode GetNextLine(void);
	ParseCode ParseLine(const char *pLine);
	ParseCode ParseDmxData(const char *pLine);

private:
	ParseCode m_tParseCode;
	TState m_tState;
	char s_buffer[2048];
	uint32_t m_nDelayMillis;
	uint32_t m_nLastMillis;
	uint32_t m_nUniverse;
	uint8_t m_DmxData[512];
	uint32_t m_nDmxDataLength;
};

#endif /* OLASHOWFILE_H_ */
