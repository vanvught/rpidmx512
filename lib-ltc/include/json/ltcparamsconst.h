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
#include "common/utils/utils_hash.h"

namespace json {
struct LtcParamsConst {
    static constexpr char kFileName[] = "ltc.json";

	static constexpr auto kSource = json::MakeSimpleKey("source");
    // System time
    static constexpr auto kAutoStart = json::MakeSimpleKey("auto_start");
    static constexpr auto kGpsStart = json::MakeSimpleKey("gps_start");
    static constexpr auto kUtcOffset = json::MakeSimpleKey("utc_offset");
    // Output options
    static constexpr auto kDisableDisplayOled = json::MakeSimpleKey("disable_display");
    static constexpr auto kDisableMax7219 = json::MakeSimpleKey("disable_max7219");
    static constexpr auto kDisableMidi = json::MakeSimpleKey("disable_midi");
    static constexpr auto kDisableArtnet = json::MakeSimpleKey("disable_artnet");
    static constexpr auto kDisableLtc = json::MakeSimpleKey("disable_ltc");
    static constexpr auto kDisableRtpmidi = json::MakeSimpleKey("disable_rtp-midi");
    static constexpr auto kDisableEtc = json::MakeSimpleKey("disable_etc");
    // System clock / RTC
    static constexpr auto kShowSystime = json::MakeSimpleKey("show_systime");
    static constexpr auto kDisableTimesync = json::MakeSimpleKey("disable_timesync");
    // NTP
    static constexpr auto kNtpEnable = json::MakeSimpleKey("ntp_enable");
    static constexpr auto kNtpYear = json::MakeSimpleKey("year");
    static constexpr auto kNtpMonth = json::MakeSimpleKey("month");
    static constexpr auto kNtpDay = json::MakeSimpleKey("day");
    // LTC
    static constexpr auto kLtcVolume = json::MakeSimpleKey("volume");
    // Art-Net
    static constexpr auto kTimecodeIp = json::MakeSimpleKey("timecode_ip");
    // source=internal
    static constexpr auto kFps = json::MakeSimpleKey("fps");
    static constexpr auto kStartFrame = json::MakeSimpleKey("start_frame");
    static constexpr auto kStartSecond = json::MakeSimpleKey("start_second");
    static constexpr auto kStartMinute = json::MakeSimpleKey("start_minute");
    static constexpr auto kStartHour = json::MakeSimpleKey("start_hour");
    static constexpr auto kIgnoreStart = json::MakeSimpleKey("ignore_start");
    static constexpr auto kStopFrame = json::MakeSimpleKey("stop_frame");
    static constexpr auto kStopSecond = json::MakeSimpleKey("stop_second");
    static constexpr auto kStopMinute = json::MakeSimpleKey("stop_minute");
    static constexpr auto kStopHour = json::MakeSimpleKey("stop_hour");
    static constexpr auto kIgnoreStop = json::MakeSimpleKey("ignore_stop");
    static constexpr auto kAltFunction = json::MakeSimpleKey("alt_function");
    static constexpr auto kSkipSeconds = json::MakeSimpleKey("skip_seconds");
    static constexpr auto kSkipFree = json::MakeSimpleKey("skip_free");
    // OSC
    static constexpr auto kOscEnable = json::MakeSimpleKey("osc_enable");
    static constexpr auto kOscPort = json::MakeSimpleKey("osc_port");
    // WS28xx Display
    static constexpr auto kWS28xxEnable = json::MakeSimpleKey("ws28xx_enable");
    // RGB panel
    static constexpr auto kRgbpanelEnable = json::MakeSimpleKey("rgbpanel_enable");
};
} // namespace json

#endif // JSON_LTCPARAMSCONST_H_
