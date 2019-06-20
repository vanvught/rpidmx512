/**
 * @file displaymatrix.h
 *
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
#include <stdio.h>
#include <time.h>

#include "max7219matrix.h"

#include "d8x8matrix.h"

Max7219Matrix *Max7219Matrix::s_pThis = 0;

static char systime[] __attribute__ ((aligned (4))) = "--:--:--";

static void itoa_base10(uint32_t arg, char *buf) {
	char *n = buf;

	if (arg == 0) {
		*n++ = '0';
		*n = '0';
		return;
	}

	*n++ = (char) ('0' + (arg / 10));
	*n = (char) ('0' + (arg % 10));
}

Max7219Matrix::Max7219Matrix(void): m_nSecondsPrevious(60) {
	s_pThis = this;

	m_DeviceInfo.chip_select = SPI_CS0;
	m_DeviceInfo.speed_hz = 0;
}

Max7219Matrix::~Max7219Matrix(void) {
}

void Max7219Matrix::Init(uint8_t nIntensity) {
	d8x8matrix_init(&m_DeviceInfo, SEGMENTS, nIntensity);
	d8x8matrix_cls(&m_DeviceInfo);
	d8x8matrix_write(&m_DeviceInfo, (const char *)"Waiting", 7);
}

void Max7219Matrix::Show(const char *pTimecode) {
	m_aBuffer[0] = (uint8_t) (pTimecode[0]);
	m_aBuffer[1] = (uint8_t) (pTimecode[1]);
	m_aBuffer[2] = (uint8_t) (pTimecode[3]);
	m_aBuffer[3] = (uint8_t) (pTimecode[4]);
	m_aBuffer[4] = (uint8_t) (pTimecode[6]);
	m_aBuffer[5] = (uint8_t) (pTimecode[7]);
	m_aBuffer[6] = (uint8_t) (pTimecode[9]);
	m_aBuffer[7] = (uint8_t) (pTimecode[10]);

	d8x8matrix_write(&m_DeviceInfo, (const char *)m_aBuffer, SEGMENTS);
}

void Max7219Matrix::ShowSysTime(void) {
	time_t ltime;
	struct tm *local_time;

	ltime = time(0);
	local_time = localtime(&ltime);

	if (__builtin_expect((m_nSecondsPrevious == (uint32_t) local_time->tm_sec), 1)) {
		return;
	}

	m_nSecondsPrevious = local_time->tm_sec;

	itoa_base10(local_time->tm_hour, (char *) &systime[0]);
	itoa_base10(local_time->tm_min, (char *) &systime[3]);
	itoa_base10(local_time->tm_sec, (char *) &systime[6]);

	d8x8matrix_write(&m_DeviceInfo, (const char *)systime, SEGMENTS);
}
