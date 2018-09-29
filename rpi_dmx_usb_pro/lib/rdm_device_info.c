/**
 * @file rdm_device_info.c
 *
 */
/* Copyright (C) 2015-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "rdm_device_info.h"

#include "read_config_file.h"
#include "sscan.h"

#include "dmx.h"

#include "rdm.h"
#include "rdm_device_const.h"

#include "c/hardware.h"

#include "util.h"

static const uint8_t DEVICE_LABEL_LENGTH = sizeof(DEVICE_LABEL) / sizeof(DEVICE_LABEL[0]) - 1;
static const uint8_t DEVICE_MANUFACTURER_NAME_LENGTH = sizeof(DEVICE_MANUFACTURER_NAME) / sizeof(DEVICE_MANUFACTURER_NAME[0]) - 1;

static const char RDM_DEVICE_FILE_NAME[] ALIGNED = "rdm_device.txt";					///< Parameters file name
static const char RDM_DEVICE_MANUFACTURER_NAME[] ALIGNED = "manufacturer_name";			///<
static const char RDM_DEVICE_MANUFACTURER_ID[] ALIGNED = "manufacturer_id";				///<
static const char RDM_DEVICE_LABEL[] ALIGNED = "device_label";							///<
static const char RDM_DEVICE_EXTERNAL_MONITOR[] ALIGNED = "device_external_monitor";	///<

// 0x7F, 0xF0 : RESERVED FOR PROTOTYPING/EXPERIMENTAL USE ONLY
static uint8_t uid_device[RDM_UID_SIZE] = { 0x7F, 0xF0, 0x00, 0x00, 0x00, 0x00 };	///<
static char root_device_label[RDM_DEVICE_LABEL_MAX_LENGTH] ALIGNED;					///<
static uint8_t root_device_label_length = (uint8_t) 0;								///<

static char device_manufacturer_name[RDM_MANUFACTURER_LABEL_MAX_LENGTH] ALIGNED;	///<
static uint8_t device_manufacturer_name_length = (uint8_t) 0;						///<

static uint8_t manufacturer_id[RDM_DEVICE_MANUFACTURER_ID_LENGTH] ALIGNED;			///<

static uint8_t device_sn[DEVICE_SN_LENGTH] ALIGNED;									///<

static uint8_t ext_mon_level = (uint8_t) 0;											///<

uint8_t rdm_device_info_get_ext_mon_level(void) {
	return ext_mon_level;
}

static uint16_t hex_uint16(const char *s) {
	uint16_t ret = 0;
	uint8_t nibble;

	while (*s != '\0') {
		char d = *s;

		if (isxdigit((int) d) == 0) {
			break;
		}

		nibble = d > '9' ? ((uint8_t) d | (uint8_t) 0x20) - (uint8_t) 'a' + (uint8_t) 10 : (uint8_t) (d - '0');
		ret = (ret << 4) | nibble;
		s++;
	}

	return ret;
}

static void process_line_read_string(const char *line) {
	char value[8];
	uint8_t len;

	len = 1;
	if (sscan_char_p(line, RDM_DEVICE_EXTERNAL_MONITOR, value, &len) == 2) {
		if (len == 1) {
			if (isdigit((int)value[0])) {
				ext_mon_level = (uint8_t)(value[0] - (char)'0');
			}
		}
		return;
	}

	len = RDM_MANUFACTURER_LABEL_MAX_LENGTH;
	if (sscan_char_p(line, RDM_DEVICE_MANUFACTURER_NAME, device_manufacturer_name, &len) == 2) {
		device_manufacturer_name_length = len;
		return;
	}

	len = RDM_DEVICE_LABEL_MAX_LENGTH;
	if (sscan_char_p(line, RDM_DEVICE_LABEL, root_device_label, &len) == 2) {
		root_device_label_length = len;
		return;
	}

	len = 4;
	memset(value, 0, sizeof(value) / sizeof(char));
	if (sscan_char_p(line, RDM_DEVICE_MANUFACTURER_ID, value, &len) == 2) {
		if (len == 4) {
			const uint16_t v = hex_uint16(value);
			uid_device[0] = (uint8_t) (v >> 8);
			uid_device[1] = (uint8_t) (v & 0xFF);
		}
		return;
	}
}

void rdm_device_info_get_label(const uint16_t sub_device, struct _rdm_device_info_data *info) {
	info->data = (uint8_t *)root_device_label;
	info->length = root_device_label_length;
}

void rdm_device_info_get_manufacturer_name(struct _rdm_device_info_data *info) {
	info->data = (uint8_t *)device_manufacturer_name;
	info->length = device_manufacturer_name_length;
}

void rdm_device_info_get_manufacturer_id(struct _rdm_device_info_data *info) {
	manufacturer_id[0] = uid_device[1];
	manufacturer_id[1] = uid_device[0];

	info->data = (uint8_t *)manufacturer_id;
	info->length = RDM_DEVICE_MANUFACTURER_ID_LENGTH;
}

void rdm_device_info_get_sn(struct _rdm_device_info_data *info) {
	info->data = (uint8_t *)device_sn;
	info->length = DEVICE_SN_LENGTH;
}

const uint8_t * rdm_device_info_get_uuid(void) {
	return uid_device;
}

void rdm_device_info_init(void) {
	uint8_t mac_address[6];

	if (hardware_get_mac_address(mac_address) == 0) {
		uid_device[2] = mac_address[2];
		uid_device[3] = mac_address[3];
		uid_device[4] = mac_address[4];
		uid_device[5] = mac_address[5];
	}

	uid_device[0] = DEVICE_MANUFACTURER_ID[0];
	uid_device[1] = DEVICE_MANUFACTURER_ID[1];

	device_sn[0] = uid_device[5];
	device_sn[1] = uid_device[4];
	device_sn[2] = uid_device[3];
	device_sn[3] = uid_device[2];

	(void *)memcpy(root_device_label, DEVICE_LABEL, DEVICE_LABEL_LENGTH);
	root_device_label_length = DEVICE_LABEL_LENGTH;

	(void *)memcpy(device_manufacturer_name, DEVICE_MANUFACTURER_NAME, DEVICE_MANUFACTURER_NAME_LENGTH);
	device_manufacturer_name_length = DEVICE_MANUFACTURER_NAME_LENGTH;

	read_config_file(RDM_DEVICE_FILE_NAME, &process_line_read_string);
}
