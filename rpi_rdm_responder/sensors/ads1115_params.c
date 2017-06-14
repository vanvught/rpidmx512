/**
 * @file ads1115_params.c
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stddef.h>

#include "read_config_file.h"
#include "sscan.h"

#include "ads1115_params.h"
#include "acs71x.h"

#include "util.h"

static const char PARAMS_FILE_NAME[] ALIGNED = "ads1115.txt";	///< Parameters file name
static const char PARAMS_CALIBRATE[] ALIGNED = "calibrate";		///<
static const char PARAMS_CH_0[] ALIGNED = "ch_0";				///<
static const char PARAMS_CH_1[] ALIGNED = "ch_1";				///<
static const char PARAMS_CH_2[] ALIGNED = "ch_2";				///<
static const char PARAMS_CH_3[] ALIGNED = "ch_3";				///<

static struct _ads1115_ch_info ads1115_ch_info ALIGNED;

/**
 *
 * @param s
 * @param len
 * @param type
 * @return
 */
static bool _check_chip_name(const char *s, const size_t len, acs71x_type_t *type) {
	acs71x_info_t acs71x_info;
	char *name;

	acs71x_info.type = 0;

	while ((name = (char *) acs71x_get_chip_name(&acs71x_info)) != NULL) {
		if (strncmp(s, name, len) == 0) {
			*type = acs71x_info.type;
			return true;
		}
		acs71x_info.type++;
	}

	return false;
}

/**
 *
 * @param line
 */
static void process_line_read(const char *line) {
	uint8_t value8;
	char value[64];
	uint8_t len;
	acs71x_type_t type;

	if (sscan_uint8_t(line, PARAMS_CALIBRATE, &value8) == 2) {
		if (value8 != 0) {
			ads1115_ch_info.calibrate = true;
		}
		return;
	}

	len = sizeof(value);
	if (sscan_char_p(line, PARAMS_CH_0, value, &len) == 2) {
		if (_check_chip_name(value, (size_t) len, &type)) {
			ads1115_ch_info.ch0.type = type;
			ads1115_ch_info.ch0.is_connected = true;
		} else {
			ads1115_ch_info.ch0.is_connected = false;
		}
		return;
	}

	len = sizeof(value);
	if (sscan_char_p(line, PARAMS_CH_1, value, &len) == 2) {
		if (_check_chip_name(value, (size_t) len, &type)) {
			ads1115_ch_info.ch1.type = type;
			ads1115_ch_info.ch1.is_connected = true;
		} else {
			ads1115_ch_info.ch1.is_connected = false;
		}
		return;
	}

	len = sizeof(value);
	if (sscan_char_p(line, PARAMS_CH_2, value, &len) == 2) {
		if (_check_chip_name(value, (size_t) len, &type)) {
			ads1115_ch_info.ch2.type = type;
			ads1115_ch_info.ch2.is_connected = true;
		} else {
			ads1115_ch_info.ch2.is_connected = false;
		}
		return;
	}

	len = sizeof(value);
	if (sscan_char_p(line, PARAMS_CH_3, value, &len) == 2) {
		if (_check_chip_name(value, (size_t) len, &type)) {
			ads1115_ch_info.ch3.type = type;
			ads1115_ch_info.ch3.is_connected = true;
		} else {
			ads1115_ch_info.ch3.is_connected = false;
		}
	}
}

/**
 *
 * @return
 */
const struct _ads1115_ch_info *ads1115_params_get_ch_info(void) {
	return &ads1115_ch_info;
}

/**
 *
 */
void ads1115_params_init(void) {
	ads1115_ch_info.ch0.is_connected = false;
	ads1115_ch_info.ch1.is_connected = false;
	ads1115_ch_info.ch2.is_connected = false;
	ads1115_ch_info.ch3.is_connected = false;

	(void) read_config_file(PARAMS_FILE_NAME, &process_line_read);
}
