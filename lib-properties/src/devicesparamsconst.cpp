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

#include "devicesparamsconst.h"

#ifndef ALIGNED
 #define ALIGNED __attribute__((aligned(4)))
#endif

char DevicesParamsConst::DEVICES_TXT[] ALIGNED = "devices.txt";
char DevicesParamsConst::LED_TYPE[] ALIGNED = "led_type";
char DevicesParamsConst::LED_COUNT[] ALIGNED = "led_count";

char DevicesParamsConst::DMX_START_ADDRESS[] ALIGNED = "dmx_start_address";
char DevicesParamsConst::SPI_SPEED_HZ[] ALIGNED = "clock_speed_hz";

char DevicesParamsConst::LED_GROUPING[] ALIGNED = "led_grouping";
char DevicesParamsConst::GLOBAL_BRIGHTNESS[] ALIGNED = "global_brightness";
