/**
 * @file devicesparamsconst.cpp
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

#include "devicesparamsconst.h"

alignas(uint32_t) const char DevicesParamsConst::FILE_NAME[] = "devices.txt";
alignas(uint32_t) const char DevicesParamsConst::LED_TYPE[] = "led_type";
alignas(uint32_t) const char DevicesParamsConst::LED_COUNT[] = "led_count";

alignas(uint32_t) const char DevicesParamsConst::ACTIVE_OUT[] = "active_out";
alignas(uint32_t) const char DevicesParamsConst::USE_SI5351A[] = "use_si5351A";

alignas(uint32_t) const char DevicesParamsConst::DMX_START_ADDRESS[] = "dmx_start_address";
alignas(uint32_t) const char DevicesParamsConst::SPI_SPEED_HZ[] = "clock_speed_hz";

alignas(uint32_t) const char DevicesParamsConst::LED_GROUPING[] = "led_grouping";
alignas(uint32_t) const char DevicesParamsConst::LED_GROUP_COUNT[] = "led_group_count";

alignas(uint32_t) const char DevicesParamsConst::GLOBAL_BRIGHTNESS[] = "global_brightness";
