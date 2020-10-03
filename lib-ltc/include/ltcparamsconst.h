/**
 * @file ltcparamsconst.h
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

#ifndef LTCPARAMSCONST_H_
#define LTCPARAMSCONST_H_

struct LtcParamsConst {
	static const char FILE_NAME[];

	static const char SOURCE[];
	// System time
	static const char AUTO_START[];
	// Output options
	static const char DISABLE_DISPLAY[];
	static const char DISABLE_MAX7219[];
	static const char DISABLE_MIDI[];
	static const char DISABLE_ARTNET[];
	static const char DISABLE_LTC[];
	static const char DISABLE_RTPMIDI[];
	static const char SHOW_SYSTIME[];
	static const char DISABLE_TIMESYNC[];
	// NTP
	static const char YEAR[];
	static const char MONTH[];
	static const char DAY[];
	static const char NTP_ENABLE[];
	// Generator
	static const char FPS[];
	static const char START_FRAME[];
	static const char START_SECOND[];
	static const char START_MINUTE[];
	static const char START_HOUR[];
	static const char STOP_FRAME[];
	static const char STOP_SECOND[];
	static const char STOP_MINUTE[];
	static const char STOP_HOUR[];
	static const char ALT_FUNCTION[];
	static const char SKIP_SECONDS[];
	static const char SKIP_FREE[];
	// OSC
	static const char OSC_ENABLE[];
	static const char OSC_PORT[];
	// WS28xx Display
	static const char WS28XX_ENABLE[];
	// RGB led panel
	static const char RGBPANEL_ENABLE[];
};

#endif /* LTCPARAMSCONST_H_ */
