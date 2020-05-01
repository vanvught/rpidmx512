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

#ifndef DMXSERIALCHANNELDATA_H_
#define DMXSERIALCHANNELDATA_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "lightset.h"

enum ParseCode {
	PARSE_FAILED,
	PARSE_SERIAL,
	PARSE_NONE,
	PARSE_EOF
};

class DmxSerialChannelData {
public:
	DmxSerialChannelData(void);
	~DmxSerialChannelData(void);

	void Clear(void);
	bool Parse(const char *pFileName);
	const uint8_t *GetData(uint8_t nChannelValue, uint32_t &nLength);

	void Dump(void);

private:
	ParseCode GetNextLine(void);
	ParseCode ParseLine(const char *pLine);
	ParseCode ParseSerialData(const char *pLine);

private:
	FILE *m_pFile;
	uint8_t m_nChannelValue;
	uint8_t m_nChannelDataLength[DMX_UNIVERSE_SIZE];
	uint8_t *m_pChannelData[DMX_UNIVERSE_SIZE];
};

#endif /* DMXSERIALCHANNELDATA_H_ */
