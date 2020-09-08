/**
 * @file gpsprint.cpp
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

#include "gps.h"

#if defined (H3)
# include "h3_board.h"
#endif

void GPS::Print() {
	printf("GPS\n");
	printf(" Module : %s [%u]\n", GetModuleName(m_tModule), m_nBaud);
	printf(" UTC offset : %d (seconds)\n", m_nUtcOffset);
	switch (m_tStatusCurrent) {
	case GPSStatus::WARNING:
		puts(" No Fix");
		break;
	case GPSStatus::VALID:
		puts(" Has Fix");
		break;
	case GPSStatus::IDLE:
		puts(" Idle");
		break;
	default:
		break;
	}
#if defined (H3)
	printf(" UART: %d\n", EXT_UART_NUMBER);
#endif
}
