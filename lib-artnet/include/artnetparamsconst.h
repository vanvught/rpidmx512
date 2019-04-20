/**
 * @file artnetparamsconst.h
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

#ifndef ARTNETPARAMSCONST_H_
#define ARTNETPARAMSCONST_H_

#include <stdint.h>

#include "artnet.h"

class ArtNetParamsConst {
public:
	alignas(uint32_t) static const char FILE_NAME[];
	alignas(uint32_t) static const char NET[];
	alignas(uint32_t) static const char SUBNET[];
	alignas(uint32_t) static const char TIMECODE[];
	alignas(uint32_t) static const char TIMESYNC[];
	alignas(uint32_t) static const char RDM[];
	alignas(uint32_t) static const char RDM_DISCOVERY[];
	alignas(uint32_t) static const char NODE_SHORT_NAME[];
	alignas(uint32_t) static const char NODE_LONG_NAME[];
	alignas(uint32_t) static const char NODE_MANUFACTURER_ID[];
	alignas(uint32_t) static const char NODE_OEM_VALUE[];
	alignas(uint32_t) static const char NODE_NETWORK_DATA_LOSS_TIMEOUT[];
	alignas(uint32_t) static const char NODE_DISABLE_MERGE_TIMEOUT[];
	alignas(uint32_t) static const char UNIVERSE_PORT[ARTNET_MAX_PORTS][16];
	alignas(uint32_t) static const char MERGE_MODE[];
	alignas(uint32_t) static const char MERGE_MODE_PORT[ARTNET_MAX_PORTS][18];
	alignas(uint32_t) static const char PROTOCOL[];
	alignas(uint32_t) static const char PROTOCOL_PORT[ARTNET_MAX_PORTS][16];
};

#endif /* ARTNETPARAMSCONST_H_ */
