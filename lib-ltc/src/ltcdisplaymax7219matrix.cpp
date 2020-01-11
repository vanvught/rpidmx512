/**
 * @file ltcdisplaymax7219matrix.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <time.h>

#include "ltcdisplaymax7219matrix.h"

#include "d8x8matrix.h"

LtcDisplayMax7219Matrix *LtcDisplayMax7219Matrix::s_pThis = 0;

LtcDisplayMax7219Matrix::LtcDisplayMax7219Matrix(void) {
	s_pThis = this;

	m_DeviceInfo.chip_select = SPI_CS0;
	m_DeviceInfo.speed_hz = 0;
}

LtcDisplayMax7219Matrix::~LtcDisplayMax7219Matrix(void) {
}

void LtcDisplayMax7219Matrix::Init(uint8_t nIntensity) {
	d8x8matrix_init(&m_DeviceInfo, SEGMENTS, nIntensity);
	d8x8matrix_cls(&m_DeviceInfo);
	d8x8matrix_write(&m_DeviceInfo, (const char *)"Waiting", 7);
}

void LtcDisplayMax7219Matrix::Show(const char *pTimecode) {
	m_aBuffer[0] = (uint8_t) pTimecode[0];
	m_aBuffer[1] = (uint8_t) pTimecode[1];
	m_aBuffer[2] = (uint8_t) pTimecode[3];
	m_aBuffer[3] = (uint8_t) pTimecode[4];
	m_aBuffer[4] = (uint8_t) pTimecode[6];
	m_aBuffer[5] = (uint8_t) pTimecode[7];
	m_aBuffer[6] = (uint8_t) pTimecode[9];
	m_aBuffer[7] = (uint8_t) pTimecode[10];

	d8x8matrix_write(&m_DeviceInfo, (const char *)m_aBuffer, SEGMENTS);
}

void LtcDisplayMax7219Matrix::ShowSysTime(const char *pSystemTime) {
	m_aBuffer[0] = (uint8_t) pSystemTime[0];
	m_aBuffer[1] = (uint8_t) pSystemTime[1];
	m_aBuffer[2] = (uint8_t) pSystemTime[3];
	m_aBuffer[3] = (uint8_t) pSystemTime[4];
	m_aBuffer[4] = (uint8_t) pSystemTime[6];
	m_aBuffer[5] = (uint8_t) pSystemTime[7];

	d8x8matrix_write(&m_DeviceInfo, (const char *)m_aBuffer, SEGMENTS);
}

void LtcDisplayMax7219Matrix::WriteChar(uint8_t nChar, uint8_t nPos) {
	// TODO Max7219Matrix::WriteChar
}

