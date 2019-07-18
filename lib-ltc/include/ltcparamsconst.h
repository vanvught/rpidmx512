/**
 * @file ltcparamsconst.h
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

#ifndef LTCPARAMSCONST_H_
#define LTCPARAMSCONST_H_

#include <stdint.h>

class LtcParamsConst {
public:
	alignas(uint32_t) static const char FILE_NAME[];
	alignas(uint32_t) static const char SOURCE[];
	alignas(uint32_t) static const char MAX7219_TYPE[];
	alignas(uint32_t) static const char MAX7219_INTENSITY[];
	alignas(uint32_t) static const char DISABLE_DISPLAY[];
	alignas(uint32_t) static const char DISABLE_MAX7219[];
	alignas(uint32_t) static const char DISABLE_MIDI[];
	alignas(uint32_t) static const char DISABLE_ARTNET[];
	alignas(uint32_t) static const char DISABLE_TCNET[];
	alignas(uint32_t) static const char DISABLE_LTC[];
	alignas(uint32_t) static const char SHOW_SYSTIME[];
	alignas(uint32_t) static const char DISABLE_TIMESYNC[];
	alignas(uint32_t) static const char YEAR[];
	alignas(uint32_t) static const char MONTH[];
	alignas(uint32_t) static const char DAY[];
	alignas(uint32_t) static const char NTP_ENABLE[];
	alignas(uint32_t) static const char FPS[];
	alignas(uint32_t) static const char START_FRAME[];
	alignas(uint32_t) static const char START_SECOND[];
	alignas(uint32_t) static const char START_MINUTE[];
	alignas(uint32_t) static const char START_HOUR[];
	alignas(uint32_t) static const char STOP_FRAME[];
	alignas(uint32_t) static const char STOP_SECOND[];
	alignas(uint32_t) static const char STOP_MINUTE[];
	alignas(uint32_t) static const char STOP_HOUR[];
#if 0
	alignas(uint32_t) static const char SET_DATE[];
#endif
};

#endif /* LTCPARAMSCONST_H_ */
