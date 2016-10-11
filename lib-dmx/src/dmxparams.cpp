/**
 * @file dmxparams.cpp
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <dmxparams.h>

#include "read_config_file.h"
#include "sscan.h"
#include "util.h"
#include "dmx.h"

static const char PARAMS_FILE_NAME[] ALIGNED = "params.txt";				///< Parameters file name
static const char PARAMS_BREAK_TIME[] ALIGNED = "dmxsend_break_time";		///<
static const char PARAMS_MAB_TIME[] ALIGNED = "dmxsend_mab_time";			///<
static const char PARAMS_REFRESH_RATE[] ALIGNED = "dmxsend_refresh_rate";	///<

static uint8_t DMXParamsBreakTime ALIGNED = DMX_PARAMS_DEFAULT_BREAK_TIME;		///< DMX output break time in 10.67 microsecond units. Valid range is 9 to 127.
static uint8_t DMXParamsMabTime ALIGNED = DMX_PARAMS_DEFAULT_MAB_TIME;			///< DMX output Mark After Break time in 10.67 microsecond units. Valid range is 1 to 127.
static uint8_t DMXParamsRefreshRate ALIGNED = DMX_PARAMS_DEFAULT_REFRESH_RATE;	///< DMX output rate in packets per second. Valid range is 1 to 40.

/**
 *
 * @param line
 */
static void process_line_read(const char *line) {
	uint8_t value8;

	if (sscan_uint8_t(line, PARAMS_BREAK_TIME, &value8) == 2) {
		if ((value8 >= (uint8_t) DMX_PARAMS_MIN_BREAK_TIME) && (value8 <= (uint8_t) DMX_PARAMS_MAX_BREAK_TIME)) {
			DMXParamsBreakTime = value8;
		}
	} else if (sscan_uint8_t(line, PARAMS_MAB_TIME, &value8) == 2) {
		if ((value8 >= (uint8_t) DMX_PARAMS_MIN_MAB_TIME) && (value8 <= (uint8_t) DMX_PARAMS_MAX_MAB_TIME)) {
			DMXParamsMabTime = value8;
		}
	} else if (sscan_uint8_t(line, PARAMS_REFRESH_RATE, &value8) == 2) {
		DMXParamsRefreshRate = value8;
	}
}

/**
 *
 */
DMXParams::DMXParams(void) {
	DMXParamsBreakTime = DMX_PARAMS_DEFAULT_BREAK_TIME;
	DMXParamsMabTime = DMX_PARAMS_DEFAULT_MAB_TIME;
	DMXParamsRefreshRate = DMX_PARAMS_DEFAULT_REFRESH_RATE;

}

/**
 *
 */
DMXParams::~DMXParams(void) {
}

/**
 *
 * @return
 */
bool DMXParams::Load(void) {
	return read_config_file(PARAMS_FILE_NAME, &process_line_read);
}

/**
 *
 * @return
 */
const uint8_t DMXParams::GetBreakTime(void) {
	return DMXParamsBreakTime;
}

/**
 *
 * @return
 */
const uint8_t DMXParams::GetMabTime(void) {
	return DMXParamsMabTime;
}

/**
 *
 * @return
 */
const uint8_t DMXParams::GetRefreshRate(void) {
	return DMXParamsRefreshRate;
}
