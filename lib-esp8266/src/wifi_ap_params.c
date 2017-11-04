/**
 * @file wifi_ap_params.c
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <ap_params.h>
#include <stdint.h>
#include <stdbool.h>

#include "read_config_file.h"
#include "sscan.h"

static const char PARAMS_FILE_NAME[] ALIGNED = "ap.txt";	///< Parameters file name
static const char PARAMS_PASSWORD[] ALIGNED = "password";	///<

static char ap_params_password[34] ALIGNED;					///<

/**
 *
 * @return
 */
const char *ap_params_get_password(void) {
	return ap_params_password;
}

/**
 *
 * @param line
 */
static void process_line_read(const char *line) {
	uint8_t len = 32;

	if (sscan_char_p(line, PARAMS_PASSWORD, ap_params_password, &len) == 2) {
		return;
	}


}

/**
 *
 */
const bool ap_params_init(void) {
	uint32_t i;

	for (i = 0; i < sizeof(ap_params_password) / sizeof(char); i++) {
		ap_params_password[i] = (char) 0;
	}

	return read_config_file(PARAMS_FILE_NAME, &process_line_read);
}

