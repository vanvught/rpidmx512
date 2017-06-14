/**
 * @file rdmdevice.cpp
 *
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

#include "rdm.h"
#include "rdm_e120.h"
#include "rdmdevice.h"

#include "read_config_file.h"
#include "hardware.h"
#include "util.h"
#include "sscan.h"

static const char DEVICE_MANUFACTURER_NAME[] ALIGNED = "AvV";							///<
static const uint8_t DEVICE_MANUFACTURER_NAME_LENGTH = sizeof(DEVICE_MANUFACTURER_NAME) / sizeof(DEVICE_MANUFACTURER_NAME[0]) - 1;
static const uint8_t DEVICE_MANUFACTURER_ID[] ALIGNED = { 0x7F, 0xF0 };					///< 0x7F, 0xF0 : RESERVED FOR PROTOTYPING/EXPERIMENTAL USE ONLY

static const char RDM_DEVICE_FILE_NAME[] ALIGNED = "rdm_device.txt";					///< Parameters file name
static const char RDM_DEVICE_MANUFACTURER_NAME[] ALIGNED = "manufacturer_name";			///<
static const char RDM_DEVICE_MANUFACTURER_ID[] ALIGNED = "manufacturer_id";				///<
static const char RDM_DEVICE_LABEL[] ALIGNED = "device_label";							///<
static const char RDM_DEVICE_EXTERNAL_MONITOR[] ALIGNED = "device_external_monitor";	///<
static const char RDM_DEVICE_PRODUCT_CATEGORY[] ALIGNED = "product_category";			///<
static const char RDM_DEVICE_PRODUCT_DETAIL[] ALIGNED = "product_detail";				///<

// 0x7F, 0xF0 : RESERVED FOR PROTOTYPING/EXPERIMENTAL USE ONLY
static uint8_t device_uid[RDM_UID_SIZE] = { 0x7F, 0xF0, 0x00, 0x00, 0x00, 0x00 };		///<
static uint8_t device_sn[DEVICE_SN_LENGTH] ALIGNED;										///<
static char device_root_label[RDM_DEVICE_LABEL_MAX_LENGTH] ALIGNED;						///<
static uint8_t device_root_label_length = (uint8_t) 0;									///<

static char device_manufacturer_name[RDM_MANUFACTURER_LABEL_MAX_LENGTH] ALIGNED;		///<
static uint8_t device_manufacturer_name_length = (uint8_t) 0;							///<

static uint16_t product_category = E120_PRODUCT_CATEGORY_OTHER;
static uint16_t product_detail = E120_PRODUCT_DETAIL_OTHER;

static uint8_t ext_mon_level = (uint8_t) 0;												///<

/**
 *
 * @param line
 */
static void process_line_read(const char *line) {
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
	if (sscan_char_p(line, RDM_DEVICE_LABEL, device_root_label, &len) == 2) {
		device_root_label_length = len;
		return;
	}

	len = 4;
	memset(value, 0, sizeof(value) / sizeof(char));
	if (sscan_char_p(line, RDM_DEVICE_MANUFACTURER_ID, value, &len) == 2) {
		if (len == 4) {
			const uint16_t v = (uint16_t) hex_uint32(value);
			device_uid[0] = (uint8_t) (v >> 8);
			device_uid[1] = (uint8_t) (v & 0xFF);
		}
		return;
	}

	len = 4;
	memset(value, 0, sizeof(value) / sizeof(char));
	if (sscan_char_p(line, RDM_DEVICE_PRODUCT_CATEGORY, value, &len) == 2) {
		if (len == 4) {
			product_category = (uint16_t) hex_uint32(value);
		}
		return;
	}

	len = 4;
	memset(value, 0, sizeof(value) / sizeof(char));
	if (sscan_char_p(line, RDM_DEVICE_PRODUCT_DETAIL, value, &len) == 2) {
		if (len == 4) {
			product_detail = (uint16_t) hex_uint32(value);
		}
		return;
	}
}

/**
 *
 */
RDMDevice::RDMDevice(void) {
	uint8_t mac_address[6];

	if (hardware_get_mac_address(mac_address) == 0) {
		device_uid[2] = mac_address[2];
		device_uid[3] = mac_address[3];
		device_uid[4] = mac_address[4];
		device_uid[5] = mac_address[5];
	}

	device_uid[0] = DEVICE_MANUFACTURER_ID[0];
	device_uid[1] = DEVICE_MANUFACTURER_ID[1];

	device_sn[0] = device_uid[5];
	device_sn[1] = device_uid[4];
	device_sn[2] = device_uid[3];
	device_sn[3] = device_uid[2];

	const uint8_t length = MIN(RDM_MANUFACTURER_LABEL_MAX_LENGTH, DEVICE_MANUFACTURER_NAME_LENGTH);
	(void *)memcpy(device_manufacturer_name, DEVICE_MANUFACTURER_NAME, length);
	device_manufacturer_name_length = length;
}

/**
 *
 */
RDMDevice::~RDMDevice(void) {
}

/**
 *
 * @param info
 */
void RDMDevice::GetLabel(struct _rdm_device_info_data *info) {
	info->data = (uint8_t *)device_root_label;
	info->length = device_root_label_length;
}

/**
 *
 * @param info
 */
void RDMDevice::SetLabel(struct _rdm_device_info_data *info) {
	const uint8_t length = MIN(RDM_DEVICE_LABEL_MAX_LENGTH, info->length);
	(void *)memcpy(device_root_label, info->data, length);
	device_root_label_length = length;
}

/**
 *
 * @param info
 */
void RDMDevice::GetManufacturerId(struct _rdm_device_info_data *info) {
	info->data[0] = device_uid[1];
	info->data[1] = device_uid[0];
	info->length = RDM_DEVICE_MANUFACTURER_ID_LENGTH;
}

/**
 *
 * @param info
 */
void RDMDevice::SetManufacturerId(struct _rdm_device_info_data *info) {
	if (info->length != RDM_DEVICE_MANUFACTURER_ID_LENGTH) {
		return;
	}

	device_uid[1] = info->data[0];
	device_uid[0] = info->data[1];
}

/**
 *
 * @return
 */
const uint8_t* RDMDevice::GetUID(void) {
	return (const uint8_t *)device_uid;
}

/**
 *
 * @param info
 */
void RDMDevice::GetManufacturerName(struct _rdm_device_info_data *info) {
	info->data = (uint8_t *)device_manufacturer_name;
	info->length = device_manufacturer_name_length;
}

/**
 *
 * @param info
 */
void RDMDevice::SetManufacturerName(struct _rdm_device_info_data *info) {
	const uint8_t length = MIN(RDM_MANUFACTURER_LABEL_MAX_LENGTH, info->length);
	(void *)memcpy(device_manufacturer_name, info->data, length);
	device_manufacturer_name_length = length;
}

/**
 *
 */
void RDMDevice::ReadConfigFile(void) {
	(void) read_config_file(RDM_DEVICE_FILE_NAME, &process_line_read);
}

/**
 *
 * @return
 */
const uint8_t RDMDevice::GetExtMonLevel(void) {
	return ext_mon_level;
}
