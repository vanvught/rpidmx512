/**
 * @file remoteconfigstatic.cpp
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
#include <string.h>
#include <assert.h>

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "remoteconfig.h"

#include "debug.h"

static const char sTxtFile[TXT_FILE_LAST][12] ALIGNED =          { "rconfig.txt", "network.txt", "artnet.txt", "e131.txt", "osc.txt", "params.txt", "devices.txt", "ltc.txt", "tcnet.txt", "oscclnt.txt", "display.txt", "nextion.txt"  };
static const uint8_t sTxtFileNameLength[TXT_FILE_LAST] ALIGNED = {  11,            11,            10,           8,          7,         10,           11,           7,         9,           11,            11,             11};
static const TStore sMap[TXT_FILE_LAST] ALIGNED = 				 { STORE_RCONFIG, STORE_NETWORK, STORE_ARTNET, STORE_E131, STORE_OSC, STORE_DMXSEND, STORE_WS28XXDMX, STORE_LTC, STORE_TCNET, STORE_OSC_CLIENT, STORE_DISPLAYUDF, STORE_NEXTION};

uint32_t RemoteConfig::GetIndex(const void *p, uint32_t &nLength) {
	uint32_t i;

#ifndef NDEBUG
	debug_dump((void *)p, 16);
#endif

	for (i = 0; i < TXT_FILE_LAST; i++) {
		nLength = sTxtFileNameLength[i];
		if (memcmp(p, (const void *) sTxtFile[i], nLength) == 0) {
			break;
		}
	}

	DEBUG_PRINTF("i=%d", i);

	return i;
}

TStore RemoteConfig::GetStore(TTxtFile tTxtFile) {
	if (tTxtFile == TXT_FILE_LAST) {
		return STORE_LAST;
	}

	return sMap[tTxtFile];
}
