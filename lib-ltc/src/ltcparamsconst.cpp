/**
 * @file ltcparamsconst.cpp
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

#include "ltcparamsconst.h"

alignas(uint32_t) const char LtcParamsConst::FILE_NAME[] = "ltc.txt";
alignas(uint32_t) const char LtcParamsConst::SOURCE[] = "source";
alignas(uint32_t) const char LtcParamsConst::MAX7219_TYPE[] = "max7219_type";
alignas(uint32_t) const char LtcParamsConst::MAX7219_INTENSITY[] = "max7219_intensity";
alignas(uint32_t) const char LtcParamsConst::DISABLE_DISPLAY[] = "disable_display";
alignas(uint32_t) const char LtcParamsConst::DISABLE_MAX7219[] = "disable_max7219";
alignas(uint32_t) const char LtcParamsConst::DISABLE_MIDI[] = "disable_midi";
alignas(uint32_t) const char LtcParamsConst::DISABLE_ARTNET[] = "disable_artnet";
alignas(uint32_t) const char LtcParamsConst::DISABLE_TCNET[] = "disable_tcnet";
alignas(uint32_t) const char LtcParamsConst::DISABLE_LTC[] = "disable_ltc";
alignas(uint32_t) const char LtcParamsConst::SHOW_SYSTIME[] = "show_systime";
alignas(uint32_t) const char LtcParamsConst::DISABLE_TIMESYNC[] = "disable_timesync";
alignas(uint32_t) const char LtcParamsConst::YEAR[] = "year";
alignas(uint32_t) const char LtcParamsConst::MONTH[] = "month";
alignas(uint32_t) const char LtcParamsConst::DAY[] = "day";
alignas(uint32_t) const char LtcParamsConst::NTP_ENABLE[] = "ntp_enable";
alignas(uint32_t) const char LtcParamsConst::FPS[] = "fps";
alignas(uint32_t) const char LtcParamsConst::START_FRAME[] = "start_frame";
alignas(uint32_t) const char LtcParamsConst::START_SECOND[] = "start_second";
alignas(uint32_t) const char LtcParamsConst::START_MINUTE[] = "start_minute";
alignas(uint32_t) const char LtcParamsConst::START_HOUR[] = "start_hour";
alignas(uint32_t) const char LtcParamsConst::STOP_FRAME[] = "stop_frame";
alignas(uint32_t) const char LtcParamsConst::STOP_SECOND[] = "stop_second";
alignas(uint32_t) const char LtcParamsConst::STOP_MINUTE[] = "stop_minute";
alignas(uint32_t) const char LtcParamsConst::STOP_HOUR[] = "stop_hour";
#if 0
alignas(uint32_t) const char LtcParamsConst::SET_DATE[] = "set_date";
#endif
