/**
 * @file rdm_device_info.c
 *
 */
/* Copyright (C) 2015-2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <string.h>

#include "rdm_device_info.h"

#include "c/read_config_file.h"
#include "c/sscan.h"

#include "dmx.h"

#include "rdm.h"

#include "c/hardware.h"

extern int sscan_char_p(const char *, const char *, char *, uint8_t *);

#ifndef ALIGNED
# define ALIGNED __attribute__ ((aligned (4)))
#endif

static const char DEVICE_LABEL[] ALIGNED = "Raspberry Pi DMX USB Pro";
static const uint8_t DEVICE_LABEL_LENGTH = sizeof(DEVICE_LABEL) / sizeof(DEVICE_LABEL[0]) - 1;

static const char DEVICE_MANUFACTURER_NAME[] ALIGNED = "www.orangepi-dmx.org";
static const uint8_t DEVICE_MANUFACTURER_NAME_LENGTH = sizeof(DEVICE_MANUFACTURER_NAME) / sizeof(DEVICE_MANUFACTURER_NAME[0]) - 1;
static const uint8_t DEVICE_MANUFACTURER_ID[] ALIGNED = { 0x00, 0x50 };

static const char RDM_DEVICE_FILE_NAME[] ALIGNED = "rdm_device.txt";
static const char RDM_DEVICE_LABEL[] ALIGNED = "device_label";
static const char RDM_DEVICE_EXTERNAL_MONITOR[] ALIGNED = "device_external_monitor";

static uint8_t uid_device[RDM_UID_SIZE] = { 0x50, 0x00, 0x00, 0x00, 0x00, 0x00 };
static char root_device_label[RDM_DEVICE_LABEL_MAX_LENGTH] ALIGNED;
static uint8_t root_device_label_length = 0;

static uint8_t device_sn[DEVICE_SN_LENGTH] ALIGNED;

static uint8_t ext_mon_level = 0;

uint8_t rdm_device_info_get_ext_mon_level(void) {
	return ext_mon_level;
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

	len = RDM_DEVICE_LABEL_MAX_LENGTH;
	if (sscan_char_p(line, RDM_DEVICE_LABEL, root_device_label, &len) == 2) {
		root_device_label_length = len;
		return;
	}
}

void rdm_device_info_get_label(const uint16_t sub_device, struct _rdm_device_info_data *info) {
	info->data = (uint8_t *)root_device_label;
	info->length = root_device_label_length;
}

void rdm_device_info_get_manufacturer_name(struct _rdm_device_info_data *info) {
	info->data = (uint8_t *)DEVICE_MANUFACTURER_NAME;
	info->length = DEVICE_MANUFACTURER_NAME_LENGTH;
}

void rdm_device_info_get_manufacturer_id(struct _rdm_device_info_data *info) {
	info->data = (uint8_t *)DEVICE_MANUFACTURER_ID;
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

	uid_device[0] = DEVICE_MANUFACTURER_ID[1];
	uid_device[1] = DEVICE_MANUFACTURER_ID[0];

	device_sn[0] = uid_device[5];
	device_sn[1] = uid_device[4];
	device_sn[2] = uid_device[3];
	device_sn[3] = uid_device[2];

	memcpy(root_device_label, DEVICE_LABEL, DEVICE_LABEL_LENGTH);
	root_device_label_length = DEVICE_LABEL_LENGTH;

	read_config_file(RDM_DEVICE_FILE_NAME, &process_line_read_string);
}
