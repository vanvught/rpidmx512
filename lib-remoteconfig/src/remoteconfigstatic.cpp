/**
 * @file remoteconfigstatic.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <algorithm>
#include <stdint.h>
#include <string.h>
#include <cassert>

#include "remoteconfig.h"

#include "debug.h"

constexpr char sTxtFile[TXT_FILE_LAST][15] = {
		"rconfig.txt", "network.txt", "artnet.txt", "e131.txt", "osc.txt", "params.txt", "devices.txt",
		"ltc.txt", "tcnet.txt", "oscclnt.txt", "display.txt", "ldisplay.txt", "mon.txt",
#if defined(STEPPER)
		"sparkfun.txt",
		"motor0.txt", "motor1.txt", "motor2.txt", "motor3.txt",
		"motor4.txt", "motor5.txt", "motor6.txt", "motor7.txt",
#endif
		"show.txt", "serial.txt", "gps.txt", "rgbpanel.txt"
};

constexpr uint8_t sTxtFileNameLength[TXT_FILE_LAST] = {
		11, 11, 10,8, 7, 10, 11,
		7, 9, 11, 11, 12, 7,
#if defined(STEPPER)
		12,
		10, 10, 10, 10,
		10, 10, 10, 10,
#endif
		8, 10, 7, 12
};

constexpr TStore sMap[TXT_FILE_LAST] = { STORE_RCONFIG,
		STORE_NETWORK, STORE_ARTNET, STORE_E131, STORE_OSC, STORE_DMXSEND, STORE_WS28XXDMX,
		STORE_LTC, STORE_TCNET, STORE_OSC_CLIENT, STORE_DISPLAYUDF, STORE_LTCDISPLAY, STORE_MONITOR,
#if defined(STEPPER)
		STORE_SPARKFUN,
		STORE_MOTORS, STORE_MOTORS, STORE_MOTORS, STORE_MOTORS,
		STORE_MOTORS, STORE_MOTORS, STORE_MOTORS, STORE_MOTORS,
#endif
		STORE_SHOW, STORE_SERIAL, STORE_GPS, STORE_RGBPANEL
};

uint32_t RemoteConfig::GetIndex(const void *p, uint32_t &nLength) {
	uint32_t i;
	DEBUG_ENTRY

	DEBUG_PRINTF("nLength=%d", nLength);

#ifndef NDEBUG
	debug_dump(const_cast<void*>(p), 16);
#endif

	for (i = 0; i < TXT_FILE_LAST; i++) {
		if (memcmp(p, sTxtFile[i], std::min(static_cast<uint32_t>(sTxtFileNameLength[i]), nLength)) == 0) {
			nLength = sTxtFileNameLength[i];
			break;
		}
	}

	DEBUG_PRINTF("nLength=%d, i=%d", nLength, i);
	DEBUG_EXIT
	return i;
}

TStore RemoteConfig::GetStore(TTxtFile tTxtFile) {
	if (tTxtFile == TXT_FILE_LAST) {
		return STORE_LAST;
	}

	return sMap[tTxtFile];
}
