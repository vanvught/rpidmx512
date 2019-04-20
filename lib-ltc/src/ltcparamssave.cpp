/**
 * @file ltcparamssave.h
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <assert.h>

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "ltcparams.h"
#include "ltcreader.h"

#include "propertiesbuilder.h"

#define SET_SOURCE				(1 << 0)
#define SET_MAX7219_TYPE		(1 << 1)
#define SET_MAX7219_INTENSITY	(1 << 2)

static const char PARAMS_FILE_NAME[] ALIGNED = "ltc.txt";
static const char PARAMS_SOURCE[] ALIGNED = "source";
static const char PARAMS_MAX7219_TYPE[] ALIGNED = "max7219_type";
static const char PARAMS_MAX7219_INTENSITY[] ALIGNED = "max7219_intensity";

bool LtcParams::Save(uint8_t* pBuffer, uint32_t nLength, uint32_t& nSize) {
	if (m_pLTcParamsStore) {
		nSize = 0;
		return false;
	}

	m_pLTcParamsStore->Copy(&m_tLtcParams);

	PropertiesBuilder builder(PARAMS_FILE_NAME, pBuffer, nLength);

	bool isAdded = builder.Add(PARAMS_SOURCE, m_tLtcParams.tSource == (uint8_t) LTC_READER_SOURCE_ARTNET ? "artnet" : (m_tLtcParams.tSource == (uint8_t) LTC_READER_SOURCE_MIDI ? "midi" : "ltc"), isMaskSet(SET_SOURCE));
	isAdded &= builder.Add(PARAMS_MAX7219_TYPE, m_tLtcParams.tMax7219Type == 1 ? "7segment" : "matrix" , isMaskSet(SET_MAX7219_TYPE));
	isAdded &= builder.Add(PARAMS_MAX7219_INTENSITY, (uint32_t) m_tLtcParams.nMax7219Intensity, isMaskSet(SET_MAX7219_INTENSITY));

	nSize = builder.GetSize();

	return isAdded;
}
