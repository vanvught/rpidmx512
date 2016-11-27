/**
 * @file rdm_params.c
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
#include <stdbool.h>

#include "read_config_file.h"
#include "sscan.h"
#include "util.h"

#include "rdm.h"

static const char PARAMS_FILE_NAME[] ALIGNED = "rdm_device.txt";					///< Parameters file name
static const char PARAMS_MANUFACTURER_NAME[] ALIGNED = "manufacturer_name";			///<
static const char PARAMS_MANUFACTURER_ID[] ALIGNED = "manufacturer_id";				///<
static const char PARAMS_LABEL[] ALIGNED = "device_label";							///<
static const char PARAMS_EXTERNAL_MONITOR[] ALIGNED = "device_external_monitor";	///<

// 0x7F, 0xF0 : RESERVED FOR PROTOTYPING/EXPERIMENTAL USE ONLY
static uint8_t rdm_params_uid[RDM_UID_SIZE] ALIGNED = { 0x7F, 0xF0, 0x00, 0x00, 0x00, 0x00 };	///<
static char rdm_params_manufacturer_name[RDM_MANUFACTURER_LABEL_MAX_LENGTH] ALIGNED;			///<
static char rdm_params_device_label[RDM_DEVICE_LABEL_MAX_LENGTH] ALIGNED;						///<
static uint8_t rdm_params_manufacturer_name_length = 0;											///<
static uint8_t rdm_params_device_label_length = 0;												///<
static uint8_t rdm_params_ext_mon_level = 0;													///<

/**
 * @ingroup rdm
 *
 * Process the input , parse name=value
 *
 * @param line
 */
static void process_line_read(const char *line) {
	char value[8] ALIGNED;
	uint8_t len;

	len = 1;
	if (sscan_char_p(line, PARAMS_EXTERNAL_MONITOR, value, &len) == 2) {
		if (len == 1) {
			if (isdigit((int)value[0])) {
				rdm_params_ext_mon_level = (uint8_t)(value[0] - (char)'0');
			}
		}
		return;
	}

	len = RDM_MANUFACTURER_LABEL_MAX_LENGTH;
	if (sscan_char_p(line, PARAMS_MANUFACTURER_NAME, rdm_params_manufacturer_name, &len) == 2) {
		rdm_params_manufacturer_name_length = len;
		return;
	}

	len = RDM_DEVICE_LABEL_MAX_LENGTH;
	if (sscan_char_p(line, PARAMS_LABEL, rdm_params_device_label, &len) == 2) {
		rdm_params_device_label_length = len;
		return;
	}

	len = 4;
	if (sscan_char_p(line, PARAMS_MANUFACTURER_ID, value, &len) == 2) {
		if (len == 4) {
			if (isxdigit((int)value[0]) && isxdigit((int)value[1]) && isxdigit((int)value[2]) && isxdigit((int)value[3])) {
				uint8_t nibble_high;
				uint8_t nibble_low;

				nibble_high = (uint8_t)((value[0] > (char)'9' ? (value[0] | (char)0x20) - (char)'a' + (char)10 : value[0] - (char)'0')) << 4;
				nibble_low = (uint8_t)(value[1] > (char)'9' ? (value[1] | (char)0x20) - (char)'a' + (char)10 : value[1] - (char)'0');

				rdm_params_uid[0] = nibble_high | nibble_low;

				nibble_high = (uint8_t)((value[2] > (char)'9' ? (value[2] | (char)0x20) - (char)'a' + (char)10 : value[2] - (char)'0')) << 4;
				nibble_low = (uint8_t)(value[3] > (char)'9' ? (value[3] | (char)0x20) - (char)'a' + (char)10 : value[3] - (char)'0');

				rdm_params_uid[1] = nibble_high | nibble_low;
			}
		}
	}
}

/**
 *
 * @return
 */
const char *rdm_params_get_device_label(void) {
	return rdm_params_device_label;
}

/**
 *
 * @return
 */
const uint8_t rdm_params_get_device_label_length(void) {
	return rdm_params_device_label_length;
}

/**
 *
 * @return
 */
const char *rdm_params_get_manufacturer_name(void) {
	return rdm_params_manufacturer_name;
}

/**
 *
 * @return
 */
const uint8_t rdm_params_get_manufacturer_name_length(void) {
	return rdm_params_manufacturer_name_length;
}

/**
 *
 * @return
 */
const uint8_t rdm_params_get_ext_mon_level(void) {
	return rdm_params_ext_mon_level;
}

/**
 *
 */
const bool rdm_params_init(void) {
	return read_config_file(PARAMS_FILE_NAME, &process_line_read);
}
