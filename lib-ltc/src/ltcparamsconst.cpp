/**
 * @file ltcparamsconst.cpp
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

#include "ltcparamsconst.h"

const char LtcParamsConst::FILE_NAME[] = "ltc.txt";

const char LtcParamsConst::SOURCE[] = "source";
// System time
const char LtcParamsConst::AUTO_START[] = "auto_start";
// Output options
const char LtcParamsConst::DISABLE_DISPLAY[] = "disable_display";
const char LtcParamsConst::DISABLE_MAX7219[] = "disable_max7219";
const char LtcParamsConst::DISABLE_MIDI[] = "disable_midi";
const char LtcParamsConst::DISABLE_ARTNET[] = "disable_artnet";
const char LtcParamsConst::DISABLE_LTC[] = "disable_ltc";
const char LtcParamsConst::DISABLE_RTPMIDI[] = "disable_rtp-midi";
const char LtcParamsConst::SHOW_SYSTIME[] = "show_systime";
const char LtcParamsConst::DISABLE_TIMESYNC[] = "disable_timesync";
// NTP
const char LtcParamsConst::YEAR[] = "year";
const char LtcParamsConst::MONTH[] = "month";
const char LtcParamsConst::DAY[] = "day";
const char LtcParamsConst::NTP_ENABLE[] = "ntp_enable";
// Generator
const char LtcParamsConst::FPS[] = "fps";
const char LtcParamsConst::START_FRAME[] = "start_frame";
const char LtcParamsConst::START_SECOND[] = "start_second";
const char LtcParamsConst::START_MINUTE[] = "start_minute";
const char LtcParamsConst::START_HOUR[] = "start_hour";
const char LtcParamsConst::STOP_FRAME[] = "stop_frame";
const char LtcParamsConst::STOP_SECOND[] = "stop_second";
const char LtcParamsConst::STOP_MINUTE[] = "stop_minute";
const char LtcParamsConst::STOP_HOUR[] = "stop_hour";
const char LtcParamsConst::ALT_FUNCTION[] = "alt_function";
const char LtcParamsConst::SKIP_SECONDS[] = "skip_seconds";
const char LtcParamsConst::SKIP_FREE[] = "skip_free";
// OSC
const char LtcParamsConst::OSC_ENABLE[] = "osc_enable";
const char LtcParamsConst::OSC_PORT[] = "osc_port";
// WS28xx Display
const char LtcParamsConst::WS28XX_ENABLE[] = "ws28xx_enable";
// RGB led panel
const char LtcParamsConst::RGBPANEL_ENABLE[] = "rgbpanel_enable";
