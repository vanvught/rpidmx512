/**
 * @file ltcparamsconst.h
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef JSON_LTCPARAMSCONST_H_
#define JSON_LTCPARAMSCONST_H_

#include "json/json_key.h"

namespace json
{
struct LtcParamsConst
{
	static constexpr char kFileName[] = "ltc.json";
   
	static constexpr json::SimpleKey kSource {
	    "source",
	    6,
	    Fnv1a32("source", 6)
	};
	
	// System time
	static constexpr json::SimpleKey kAutoStart {
	    "auto_start",
	    10,
	    Fnv1a32("auto_start", 10)
	};
	
	static constexpr json::SimpleKey kGpsStart {
	    "gps_start",
	    9,
	    Fnv1a32("gps_start", 9)
	};

	static constexpr json::SimpleKey kUtcOffset {
	    "utc_offset",
	    11,
	    Fnv1a32("utc_offset", 11)
	};
	
	// Output options
	static constexpr json::SimpleKey kDisableDisplayOled {
	    "disable_display",
	    14,
	    Fnv1a32("disable_display", 14)
	};
	
	static constexpr json::SimpleKey kDisableMax7219 {
	    "disable_max7219",
	    14,
	    Fnv1a32("disable_max7219", 14)
	};
	
	static constexpr json::SimpleKey kDisableMidi {
	    "disable_midi",
	    12,
	    Fnv1a32("disable_midi", 12)
	};
	
	static constexpr json::SimpleKey kDisableArtnet {
	    "disable_artnet",
	    13,
	    Fnv1a32("disable_artnet", 13)
	};
	
	static constexpr json::SimpleKey kDisableLtc {
	    "disable_ltc",
	    11,
	    Fnv1a32("disable_ltc", 11)
	};
	
	static constexpr json::SimpleKey kDisableRtpmidi {
	    "disable_rtp-midi",
	    16,
	    Fnv1a32("disable_rtp-midi", 16)
	};
	
	static constexpr json::SimpleKey kDisableEtc {
	    "disable_etc",
	    11,
	    Fnv1a32("disable_etc", 11)
	};
	
	// System clock / RTC
	static constexpr json::SimpleKey kShowSystime {
	    "show_systime",
	    12,
	    Fnv1a32("show_systime", 12)
	};
	
	static constexpr json::SimpleKey kDisableTimesync {
	    "disable_timesync",
	    16,
	    Fnv1a32("disable_timesync", 16)
	};
	
	// NTP
	static constexpr json::SimpleKey kNtpEnable {
	    "ntp_enable",
	    10,
	    Fnv1a32("ntp_enable", 10)
	};
	
	static constexpr json::SimpleKey kNtpYear {
	    "year",
	    4,
	    Fnv1a32("year", 4)
	};
	
	static constexpr json::SimpleKey kNtpMonth {
	    "month",
	    5,
	    Fnv1a32("month", 5)
	};
	
	static constexpr json::SimpleKey kNtpDay {
	    "day",
	    3,
	    Fnv1a32("day", 3)
	};
		
	// LTC
	static constexpr json::SimpleKey kLtcVolume {
	    "volume",
	   6,
	    Fnv1a32("volume", 6)
	};
	
	// Art-Net
	static constexpr json::SimpleKey kTimecodeIp {
	    "timecode_ip",
	   11,
	    Fnv1a32("timecode_ip", 11)
	};
	
	// source=internal
	static constexpr json::SimpleKey kFps {
	    "fps",
	   3,
	    Fnv1a32("fps", 3)
	};
	
	static constexpr json::SimpleKey kStartFrame {
	    "start_frame",
	    11,
	    Fnv1a32("start_frame", 11)
	};
	
	static constexpr json::SimpleKey kStartSecond {
	    "start_second",
	    12,
	    Fnv1a32("start_second", 12)
	};
	
	static constexpr json::SimpleKey kStartMinute {
	    "start_minute",
	    12,
	    Fnv1a32("start_minute", 12)
	};	

	static constexpr json::SimpleKey kStartHour {
	    "start_hour",
	    10,
	    Fnv1a32("start_hour", 10)
	};	

	static constexpr json::SimpleKey kIgnoreStart {
	    "ignore_start",
	   12,
	    Fnv1a32("ignore_start", 12)
	};
	
	static constexpr json::SimpleKey kStopFrame {
	    "stop_frame",
	    10,
	    Fnv1a32("stop_frame", 10)
	};
	
	static constexpr json::SimpleKey kStopSecond {
	    "stop_second",
	    11,
	    Fnv1a32("stop_second", 11)
	};
	
	static constexpr json::SimpleKey kStopMinute {
	    "stop_minute",
	    11,
	    Fnv1a32("stop_minute", 11)
	};	

	static constexpr json::SimpleKey kStopHour {
	    "stop_hour",
	    9,
	    Fnv1a32("stop_hour", 9)
	};
		
	static constexpr json::SimpleKey kIgnoreStop {
	    "ignore_stop",
	   11,
	    Fnv1a32("ignore_stop", 11)
	};
	
	static constexpr json::SimpleKey kAltFunction {
	    "alt_function",
	    12,
	    Fnv1a32("alt_function", 12)
	};	

	static constexpr json::SimpleKey kSkipSeconds {
	    "skip_seconds",
	    12,
	    Fnv1a32("skip_seconds", 12)
	};
		
	static constexpr json::SimpleKey kSkipFree {
	    "skip_free",
	   9,
	    Fnv1a32("skip_free", 9)
	};
		
	// OSC
	static constexpr json::SimpleKey kOscEnable {
	    "osc_enable",
	   10,
	    Fnv1a32("osc_enable", 10)
	};
	
	static constexpr json::SimpleKey kOscPort {
	    "osc_port",
	    8,
	    Fnv1a32("osc_port", 8)
	};
			
	// WS28xx Display
	static constexpr json::SimpleKey kWS28xxEnable {
	    "ws28xx_enable",
	   13,
	    Fnv1a32("ws28xx_enable", 13)
	};
	
	// RGB panel
	static constexpr json::SimpleKey kRgbpanelEnable {
	    "rgbpanel_enable",
	   15,
	    Fnv1a32("rgbpanel_enable", 15)
	};	
};
} // namespace json

#endif  // JSON_LTCPARAMSCONST_H_
