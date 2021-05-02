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

using namespace remoteconfig;

constexpr char s_TxtFile[static_cast<uint32_t>(TxtFile::LAST)][15] = {
		"rconfig.txt", "network.txt", "artnet.txt", "e131.txt", "osc.txt", "params.txt", "devices.txt",
		"ltc.txt", "tcnet.txt", "oscclnt.txt", "display.txt", "ldisplay.txt", "mon.txt",
#if defined(OUTPUT_STEPPER)
		"sparkfun.txt",
		"motor0.txt", "motor1.txt", "motor2.txt", "motor3.txt",
		"motor4.txt", "motor5.txt", "motor6.txt", "motor7.txt",
#endif
		"show.txt", "serial.txt", "gps.txt", "rgbpanel.txt"
};

constexpr uint8_t s_TxtFileNameLength[static_cast<uint32_t>(TxtFile::LAST)] = {
		11, 11, 10,8, 7, 10, 11,
		7, 9, 11, 11, 12, 7,
#if defined(OUTPUT_STEPPER)
		12,
		10, 10, 10, 10,
		10, 10, 10, 10,
#endif
		8, 10, 7, 12
};

#include "spiflashstore.h"

using namespace spiflashstore;

constexpr Store s_Map[static_cast<uint32_t>(TxtFile::LAST)] = {
		Store::RCONFIG, Store::NETWORK, Store::ARTNET, Store::E131, Store::OSC, Store::DMXSEND, Store::WS28XXDMX,
		Store::LTC, Store::TCNET, Store::OSC_CLIENT, Store::DISPLAYUDF, Store::LTCDISPLAY, Store::MONITOR,
#if defined(OUTPUT_STEPPER)
		Store::SPARKFUN,
		Store::MOTORS, Store::MOTORS, Store::MOTORS, Store::MOTORS,
		Store::MOTORS, Store::MOTORS, Store::MOTORS, Store::MOTORS,
#endif
		Store::SHOW, Store::SERIAL, Store::GPS, Store::RGBPANEL };

TxtFile RemoteConfig::GetIndex(const void *p, uint32_t &nLength) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nLength=%d", nLength);

#ifndef NDEBUG
	debug_dump(const_cast<void*>(p), 16);
#endif

	uint32_t i;
	for (i = 0; i < static_cast<uint32_t>(TxtFile::LAST); i++) {
		if (memcmp(p, s_TxtFile[i], std::min(static_cast<uint32_t>(s_TxtFileNameLength[i]), nLength)) == 0) {
			nLength = s_TxtFileNameLength[i];
			break;
		}
	}

	DEBUG_PRINTF("nLength=%d, i=%d", nLength, i);
	DEBUG_EXIT
	return static_cast<TxtFile>(i);
}

Store RemoteConfig::GetStore(TxtFile tTxtFile) {
	if (tTxtFile == TxtFile::LAST) {
		return Store::LAST;
	}

	return s_Map[static_cast<uint32_t>(tTxtFile)];
}
