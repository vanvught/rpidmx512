/**
 * @file dmx_devices.c
 *
 */
/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#include <dmx_devices.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "bcm2835.h"
#include "bcm2835_spi.h"
#include "dmx.h"
#include "tables.h"
#include "util.h"
#include "ff.h"
#include "sscan.h"

TABLE(initializer_t, devices)
TABLE(initializer_t, devices_init)

_devices_t devices_connected;

static struct _dmx_devices_statistics dmx_devices_statistics __attribute__((aligned(4)));	///<

/**
 * @ingroup dmx
 *
 * @return
 */
const struct _dmx_devices_statistics *dmx_devices_get_statistics(void) {
	return &dmx_devices_statistics;
}

/**
 * @ingroup dmx
 *
 */
void dmx_devices_reset_statistics(void) {
	dmx_devices_statistics.function_count = 0;
	dmx_devices_statistics.dmx_available_count = 0;
}

/**
 * @ingroup dmx
 *
 * @param line
 * @return
 */
static int add_connected_device(const char *line) {
	if (devices_connected.elements_count < (uint16_t)(sizeof(devices_connected.device_entry)	/ sizeof(devices_connected.device_entry[0]))) {

		char device_name[65];
		char chip_select;
		uint8_t slave_address;
		uint16_t dmx_start_address;
		int rc;
		uint8_t len = 64;

		rc = sscan_spi(line, &chip_select, device_name, &len, &slave_address, &dmx_start_address);
#ifdef DEBUG
		printf("%s", line);
		printf("%d, {%d (%s)[%d] %d %d}\n", rc, chip_select, device_name, len, slave_address, dmx_start_address);
#endif
		if ((rc == 4) && (len != 0)) {
			int j;

			if (chip_select < (char)BCM2835_SPI_CS0 || chip_select > (char)BCM2835_SPI_CS1) {
#ifdef DEBUG
				printf("warning : invalid chip_select [skipping this line]\n");
#endif
				return DMX_DEVICE_CONFIG_INVALID_CHIP_SELECT;
			}

			if (dmx_start_address == (uint16_t)0 || dmx_start_address > (uint16_t)DMX_UNIVERSE_SIZE) {
#ifdef DEBUG
				printf("warning : invalid dmx_start_address [skipping this line]\n");
#endif
				return DMX_DEVICE_CONFIG_INVALID_START_ADDRESS;
			}

			for (j = 0; j < TABLE_LENGTH(devices); j++) {
				if (strcmp(devices_table[j].name, device_name) == 0) {
#ifdef DEBUG
					printf("device [%s] found in devices_table, entry number = %d\n", device_name, j);
#endif
					uint16_t devices_added = devices_connected.elements_count;
					devices_connected.device_entry[devices_added].devices_table_index = j;
					devices_connected.device_entry[devices_added].dmx_device_info.device_info.chip_select = (uint8_t)chip_select;
					devices_connected.device_entry[devices_added].dmx_device_info.device_info.slave_address = slave_address;
					devices_connected.device_entry[devices_added].dmx_device_info.dmx_start_address = dmx_start_address;
					devices_connected.device_entry[devices_added].dmx_device_info.rdm_sub_devices_info = NULL;
					devices_added++;
					devices_connected.elements_count = devices_added;
					return devices_connected.elements_count;
				}
			}
		} else {
			switch (rc) {
				case DMX_DEVICE_CONFIG_INVALID_PROTOCOL:
#ifdef DEBUG
					printf("warning : invalid protocol. [skipping this line]\n");
#endif
					return DMX_DEVICE_CONFIG_INVALID_PROTOCOL;
					//break;
				case DMX_DEVICE_CONFIG_INVALID_CHIP_SELECT:
#ifdef DEBUG
					printf("warning : invalid chip_select [skipping this line]\n");
#endif
					return DMX_DEVICE_CONFIG_INVALID_CHIP_SELECT;
					//break;
				case DMX_DEVICE_CONFIG_INVALID_SLAVE_ADDRESS:
#ifdef DEBUG
					printf("warning : invalid slave_address [skipping this line]\n");
#endif
					return DMX_DEVICE_CONFIG_INVALID_SLAVE_ADDRESS;
					//break;
				case DMX_DEVICE_CONFIG_INVALID_START_ADDRESS:
#ifdef DEBUG
					printf("warning : invalid dmx_start_address [skipping this line]\n");
#endif
					return DMX_DEVICE_CONFIG_INVALID_START_ADDRESS;
					//break;
				default:
					return DMX_DEVICE_CONFIG_INVALID_ENTRY;
					//break;
			}
		}


	}

	return DMX_DEVICE_CONFIG_TABLE_FULL;
}

/**
 * @ingroup dmx
 *
 */
void dmx_devices_read_config(void) {
	int rc = -1;

	FATFS fat_fs;
	FIL file_object;

	f_mount(0, &fat_fs);		// Register volume work area (never fails)

	devices_connected.elements_count = 0;

	rc = f_open(&file_object, "devices.txt", FA_READ);

	if (rc == FR_OK) {
		TCHAR buffer[196];
		for(;;) {
			if (f_gets(buffer, sizeof buffer, &file_object) == NULL)
				break; // Error or end of file
			if (add_connected_device((const char *)buffer) == DMX_DEVICE_CONFIG_TABLE_FULL)
				break;
		}
		f_close(&file_object);
	} else {

	}
#ifdef DEBUG_ANALYZER
	// Do NOT init any devices
	devices_connected.elements_count = 0;
#endif
#ifdef DEBUG
	printf("\ndevices_connected.elements_count = %d\n\n", devices_connected.elements_count);
#endif
}

/**
 * @ingroup dmx
 *
 */
void dmx_devices_init(void) {
	uint16_t i;
	for (i = 0; i < devices_connected.elements_count; i++) {
		devices_init_table[devices_connected.device_entry[i].devices_table_index].f(&(devices_connected.device_entry[i].dmx_device_info));
	}
}

/**
 * @ingroup dmx
 *
 */
void dmx_devices_run() {
	uint16_t i;

	dmx_devices_statistics.function_count++;

	if (!dmx_get_available()) {
		return;
	}

	dmx_set_available_false();

	dmx_devices_statistics.dmx_available_count++;

	for (i = 0; i < devices_connected.elements_count; i++) {
		devices_table[devices_connected.device_entry[i].devices_table_index].f(&(devices_connected.device_entry[i].dmx_device_info));
	}
}

/**
 * @ingroup dmx
 *
 * @return
 */
const uint16_t dmx_devices_get_devices_connected(void) {
	return devices_connected.elements_count;
}

/**
 * @ingroup dmx
 *
 * @param sub_device
 * @return
 */
const uint16_t dmx_devices_get_footprint(const uint16_t sub_device) {
	if ((sub_device != 0) || (sub_device < devices_connected.elements_count)) {
		return devices_connected.device_entry[sub_device - 1].dmx_device_info.rdm_sub_devices_info->dmx_footprint;
	}

	return 0;
}

/**
 * @ingroup dmx
 *
 * @param sub_device
 * @return
 */
const uint16_t dmx_devices_get_dmx_start_address(const uint16_t sub_device) {
	if ((sub_device != 0) || (sub_device < devices_connected.elements_count)) {
		return devices_connected.device_entry[sub_device - 1].dmx_device_info.dmx_start_address;
	}

	return 0;
}

/**
 * @ingroup dmx
 *
 * @param sub_device
 * @param dmx_start_address
 */
void dmx_devices_set_dmx_start_address(const uint16_t sub_device, const uint16_t dmx_start_address) {
	if ((sub_device != 0) || (sub_device < devices_connected.elements_count)) {
		devices_connected.device_entry[sub_device - 1].dmx_device_info.dmx_start_address = dmx_start_address;
	}
}

/**
 * @ingroup dmx
 *
 * @param sub_device
 * @return
 */
const char *dmx_devices_get_label(const uint16_t sub_device) {
	if ((sub_device != 0) || (sub_device < devices_connected.elements_count)) {
		return devices_connected.device_entry[sub_device - 1].dmx_device_info.rdm_sub_devices_info->device_label;
	}

	return NULL;
}

/**
 * @ingroup dmx
 *
 * @param sub_device
 * @param label
 * @param label_length
 */
void dmx_devices_set_label(const uint16_t sub_device, const uint8_t *label, uint8_t label_length) {
	if ((sub_device != 0) || (sub_device < devices_connected.elements_count)) {
		memcpy(devices_connected.device_entry[sub_device - 1].dmx_device_info.rdm_sub_devices_info->device_label, label, label_length);
		devices_connected.device_entry[sub_device - 1].dmx_device_info.rdm_sub_devices_info->device_label_length = label_length;
	}
}

/**
 * @ingroup dmx
 *
 * @param sub_device
 * @return
 */
const uint8_t dmx_devices_get_label_length(const uint16_t sub_device) {
	if ((sub_device != 0) || (sub_device < devices_connected.elements_count)) {
		return devices_connected.device_entry[sub_device - 1].dmx_device_info.rdm_sub_devices_info->device_label_length;
	}

	return 0;
}

/**
 * @ingroup dmx
 *
 * @param sub_device
 * @return
 */
const uint8_t dmx_devices_get_personality_current(const uint16_t sub_device) {
	if ((sub_device != 0) || (sub_device < devices_connected.elements_count)) {
		return devices_connected.device_entry[sub_device - 1].dmx_device_info.rdm_sub_devices_info->current_personality;
	}

	return 0;
}

/**
 * @ingroup dmx
 *
 * @param sub_device
 * @param personality
 */
void dmx_devices_set_personality_current(const uint16_t sub_device, const uint8_t personality) {
	if ((sub_device != 0) || (sub_device < devices_connected.elements_count)) {
		devices_connected.device_entry[sub_device - 1].dmx_device_info.rdm_sub_devices_info->current_personality = personality;
	}
}

/**
 * @ingroup dmx
 *
 * @param sub_device
 * @param personality
 * @return
 */
const char *dmx_devices_get_personality_description(const uint16_t sub_device, /*@unused@*/const uint8_t personality) {
	if ((sub_device != 0) || (sub_device < devices_connected.elements_count)) {
		return devices_connected.device_entry[sub_device - 1].dmx_device_info.rdm_sub_devices_info->rdm_personalities->description;
	}

	return NULL;
}

/**
 * @ingroup dmx
 *
 * @param sub_device
 * @param personality
 * @return
 */
const uint16_t dmx_devices_get_personality_slots(const uint16_t sub_device, /*@unused@*/const uint8_t personality) {
	if ((sub_device != 0) || (sub_device < devices_connected.elements_count)) {
		return devices_connected.device_entry[sub_device - 1].dmx_device_info.rdm_sub_devices_info->rdm_personalities->slots;
	}

	return 0;
}

/**
 * @ingroup dmx
 *
 * @param sub_device
 * @return
 */
const struct _rdm_sub_devices_info *dmx_devices_info_get(const uint16_t sub_device) {
	if ((sub_device != 0) || (sub_device < devices_connected.elements_count)) {
		return devices_connected.device_entry[sub_device - 1].dmx_device_info.rdm_sub_devices_info;
	}

	return NULL;
}

/**
 * @ingroup dmx
 *
 * @param sub_device
 * @param sub_devices_info
 */
void dmx_devices_info_set(const uint16_t sub_device, const struct _rdm_sub_devices_info *sub_devices_info) {
	if ((sub_device != 0) || (sub_device < devices_connected.elements_count)) {
		memcpy(devices_connected.device_entry[sub_device - 1].dmx_device_info.rdm_sub_devices_info, sub_devices_info, sizeof(struct _rdm_sub_devices_info));
	}
}

