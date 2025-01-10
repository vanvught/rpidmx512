/**
 * @file ltcparamsconst.h
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
	static inline const char FILE_NAME[] = "ltc.txt";

	static inline const char SOURCE[] = "source";
	// System time
	static inline const char AUTO_START[] = "auto_start";
	static inline const char GPS_START[] = "gps_start";
	static inline const char UTC_OFFSET[]= "utc_offset";
	// Output options
	static inline const char DISABLE_DISPLAY_OLED[] = "disable_display";
	static inline const char DISABLE_MAX7219[] = "disable_max7219";
	static inline const char DISABLE_MIDI[] = "disable_midi";
	static inline const char DISABLE_ARTNET[] = "disable_artnet";
	static inline const char DISABLE_LTC[] = "disable_ltc";
	static inline const char DISABLE_RTPMIDI[] = "disable_rtp-midi";
	static inline const char DISABLE_ETC[] = "disable_etc";
	static inline const char SHOW_SYSTIME[] = "show_systime";
	static inline const char DISABLE_TIMESYNC[] = "disable_timesync";
	// NTP
	static inline const char YEAR[] = "year";
	static inline const char MONTH[] = "month";
	static inline const char DAY[] = "day";
	static inline const char NTP_ENABLE[] = "ntp_enable";
	// LTC
	static inline const char VOLUME[] = "volume";
	// Art-Net
	static inline const char TIMECODE_IP[] = "timecode_ip";
	// Generator
	static inline const char FPS[] = "fps";
	static inline const char START_FRAME[] = "start_frame";
	static inline const char START_SECOND[] = "start_second";
	static inline const char START_MINUTE[] = "start_minute";
	static inline const char START_HOUR[] = "start_hour";
	static inline const char IGNORE_START[] = "ignore_start";
	static inline const char STOP_FRAME[] = "stop_frame";
	static inline const char STOP_SECOND[] = "stop_second";
	static inline const char STOP_MINUTE[] = "stop_minute";
	static inline const char STOP_HOUR[] = "stop_hour";
	static inline const char IGNORE_STOP[] = "ignore_stop";
	static inline const char ALT_FUNCTION[] = "alt_function";
	static inline const char SKIP_SECONDS[] = "skip_seconds";
	static inline const char SKIP_FREE[] = "skip_free";
	// OSC
	static inline const char OSC_ENABLE[] = "osc_enable";
	static inline const char OSC_PORT[] = "osc_port";
	// WS28xx Display
	static inline const char WS28XX_ENABLE[] = "ws28xx_enable";
	// RGB led panel
	static inline const char RGBPANEL_ENABLE[] = "rgbpanel_enable";
};

#endif /* LTCPARAMSCONST_H_ */
