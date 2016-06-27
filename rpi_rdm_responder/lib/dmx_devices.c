/**
 * @file dmx_devices.c
 *
 */
/* Copyright (C) 2015, 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdio.h>
#include <stdbool.h>

#include "bcm2835.h"
#include "bcm2835_spi.h"
#include "util.h"
#include "tables.h"
#include "read_config_file.h"
#include "sscan.h"
#include "dmx_devices.h"
#include "dmx.h"

extern int sscan_spi(const char *, char *, char *, uint8_t *, uint8_t *, uint16_t *, uint32_t *, uint8_t *);


TABLE(initializer_t, devices)
TABLE(initializer_t, devices_init)
TABLE(initializer_t, devices_zero)

devices_t devices_connected ALIGNED;											///<
volatile static struct _dmx_devices_statistics dmx_devices_statistics ALIGNED;	///<

static bool dmx_devices_is_zero  = false;										///<

/**
 * @ingroup dmx
 *
 */
devices_t *dmx_devices_get_devices(void) {
	return &devices_connected;
}

/**
 * @ingroup dmx
 *
 * @return
 */
volatile const struct _dmx_devices_statistics *dmx_devices_get_statistics(void) {
	return &dmx_devices_statistics;
}

/**
 * @ingroup dmx
 *
 */
void dmx_devices_reset_statistics(void) {
	dmx_devices_statistics.function_count = 0;
	dmx_devices_statistics.run_count = 0;
}

/**
 * @ingroup dmx
 *
 * @param line
 * @return
 */
static void add_connected_device(const char *line) {
#ifdef DEBUG
	printf("%d < %d\n", devices_connected.elements_count, ((uint16_t)(sizeof(devices_connected.device_entry) / sizeof(devices_connected.device_entry[0]))));
#endif
	if (devices_connected.elements_count < (uint16_t)(sizeof(devices_connected.device_entry) / sizeof(devices_connected.device_entry[0]))) {

		char device_name[65];
		uint8_t len = sizeof(device_name) - 1;
		char chip_select = (char) BCM2835_SPI_CS_NONE;
		uint8_t slave_address = 0;
		uint16_t dmx_start_address = 0;
		uint32_t spi_speed = 0;
		uint8_t pixel_count = 0;
		int rc = DMX_DEVICE_CONFIG_ERROR;

		rc = sscan_spi(line, &chip_select, device_name, &len, &slave_address, &dmx_start_address, &spi_speed, &pixel_count);
#ifdef DEBUG
		printf("%s", line);
		printf("%d, {%d (%s)[%d] %d %d %ld:%d}\n", rc, chip_select, device_name, len, slave_address, dmx_start_address, spi_speed, pixel_count);
#endif
		if ((rc >= 5) && (len != 0)) {
			int j;

			if (chip_select < (char)BCM2835_SPI_CS0 || chip_select > (char)BCM2835_SPI_CS1) {
#ifdef DEBUG
				printf("warning : invalid chip_select [skipping this line]\n");
#endif
				//return DMX_DEVICE_CONFIG_INVALID_CHIP_SELECT;
			}

			if (dmx_start_address == (uint16_t)0 || dmx_start_address > (uint16_t)DMX_UNIVERSE_SIZE) {
#ifdef DEBUG
				printf("warning : invalid dmx_start_address [skipping this line]\n");
#endif
				//return DMX_DEVICE_CONFIG_INVALID_START_ADDRESS;
			}

			if ((spi_speed != 0) && (spi_speed < BCM2835_SPI_CLOCK_MIN || spi_speed > BCM2835_SPI_CLOCK_MAX)) {
#ifdef DEBUG
				printf("warning : invalid spi_speed [skipping this line]\n");
#endif
				//return DMX_DEVICE_CONFIG_INVALID_SPI_SPEED;
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
					devices_connected.device_entry[devices_added].dmx_device_info.device_info.speed_hz = spi_speed;
					devices_connected.device_entry[devices_added].dmx_device_info.dmx_start_address = dmx_start_address;
					devices_connected.device_entry[devices_added].dmx_device_info.pixel_count = pixel_count;
					devices_added++;
					devices_connected.elements_count = devices_added;
					//return (int)devices_connected.elements_count;
				}
			}
		} else {
			switch (rc) {
				case DMX_DEVICE_CONFIG_INVALID_PROTOCOL:
#ifdef DEBUG
					printf("warning : invalid protocol. [skipping this line]\n");
#endif
					//return DMX_DEVICE_CONFIG_INVALID_PROTOCOL;
					break;
				case DMX_DEVICE_CONFIG_INVALID_CHIP_SELECT:
#ifdef DEBUG
					printf("warning : invalid chip_select [skipping this line]\n");
#endif
					//return DMX_DEVICE_CONFIG_INVALID_CHIP_SELECT;
					break;
				case DMX_DEVICE_CONFIG_INVALID_SLAVE_ADDRESS:
#ifdef DEBUG
					printf("warning : invalid slave_address [skipping this line]\n");
#endif
					//return DMX_DEVICE_CONFIG_INVALID_SLAVE_ADDRESS;
					break;
				case DMX_DEVICE_CONFIG_INVALID_START_ADDRESS:
#ifdef DEBUG
					printf("warning : invalid dmx_start_address [skipping this line]\n");
#endif
					//return DMX_DEVICE_CONFIG_INVALID_START_ADDRESS;
					break;
				case DMX_DEVICE_CONFIG_INVALID_SPI_SPEED:
#ifdef DEBUG
					printf("warning : invalid spi_speed [skipping this line]\n");
#endif
					// DMX_DEVICE_CONFIG_INVALID_SPI_SPEED;
					break;
				default:
					//return DMX_DEVICE_CONFIG_INVALID_ENTRY;
					break;
			}
		}
	}

	//return DMX_DEVICE_CONFIG_TABLE_FULL;
}

/**
 * @ingroup dmx
 *
 */
/*
static void dmx_devices_read_config(void) {
	FIL file_object;

	devices_connected.elements_count = 0;

	if (FR_OK == f_open(&file_object, "devices.txt", FA_READ)) {
		TCHAR buffer[196];
		for(;;) {
			if (f_gets(buffer, sizeof(buffer), &file_object) == NULL)
				break; // Error or end of file
			if (add_connected_device((const char *)buffer) == DMX_DEVICE_CONFIG_TABLE_FULL)
				break;
		}
		(void)f_close(&file_object);
	} else {
#ifdef DEBUG
	printf("Cannot read file \"devices.txt\"\n");
#endif
	}
#ifdef DEBUG_ANALYZER
	// Do NOT init any devices
	devices_connected.elements_count = 0;
#endif
#ifdef DEBUG
	printf("devices_connected.elements_count = %d\n", devices_connected.elements_count);
#endif
}
*/

/**
 * @ingroup dmx
 *
 */
void dmx_devices_init(void) {
	uint16_t i;

	read_config_file("devices.txt", &add_connected_device);

	for (i = 0; i < devices_connected.elements_count; i++) {
		devices_init_table[devices_connected.device_entry[i].devices_table_index].f(&(devices_connected.device_entry[i].dmx_device_info), NULL);
	}
}

/**
 * @ingroup dmx
 *
 */
void dmx_devices_zero(void) {
	uint16_t i;

	for (i = 0; i < devices_connected.elements_count; i++) {
		devices_zero_table[devices_connected.device_entry[i].devices_table_index].f(&(devices_connected.device_entry[i].dmx_device_info), NULL);
	}

	dmx_devices_is_zero = true;
}

/**
 * @ingroup dmx
 *
 */
void dmx_devices_run() {
	uint16_t i;
	volatile uint32_t dmx_updates_per_seconde;
	const uint8_t *dmx_data = dmx_get_available();

	dmx_devices_statistics.function_count++;

	if (dmx_data == NULL) {
		dmx_updates_per_seconde = dmx_get_updates_per_seconde();

		if (dmx_updates_per_seconde == 0) {
			if (!dmx_devices_is_zero) {
				dmx_devices_zero();
			}
		}

		return;
	}

	dmx_devices_is_zero = false;

	dmx_devices_statistics.run_count++;

	for (i = 0; i < devices_connected.elements_count; i++) {
		devices_table[devices_connected.device_entry[i].devices_table_index].f(&(devices_connected.device_entry[i].dmx_device_info), dmx_data);
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
		return devices_connected.device_entry[sub_device - 1].dmx_device_info.rdm_sub_devices_info.dmx_footprint;
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
		return devices_connected.device_entry[sub_device - 1].dmx_device_info.rdm_sub_devices_info.device_label;
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
		(void *)_memcpy(devices_connected.device_entry[sub_device - 1].dmx_device_info.rdm_sub_devices_info.device_label, label, label_length);
		devices_connected.device_entry[sub_device - 1].dmx_device_info.rdm_sub_devices_info.device_label_length = label_length;
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
		return devices_connected.device_entry[sub_device - 1].dmx_device_info.rdm_sub_devices_info.device_label_length;
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
		return devices_connected.device_entry[sub_device - 1].dmx_device_info.rdm_sub_devices_info.current_personality;
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
		devices_connected.device_entry[sub_device - 1].dmx_device_info.rdm_sub_devices_info.current_personality = personality;
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
		return devices_connected.device_entry[sub_device - 1].dmx_device_info.rdm_sub_devices_info.rdm_personalities->description;
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
const uint8_t dmx_devices_get_personality_description_length(const uint16_t sub_device, /*@unused@*/const uint8_t personality) {
	if ((sub_device != 0) || (sub_device < devices_connected.elements_count)) {
		return devices_connected.device_entry[sub_device - 1].dmx_device_info.rdm_sub_devices_info.rdm_personalities->description_len;
	}

	return 0;
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
		return devices_connected.device_entry[sub_device - 1].dmx_device_info.rdm_sub_devices_info.rdm_personalities->slots;
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
		return &(devices_connected.device_entry[sub_device - 1].dmx_device_info).rdm_sub_devices_info;
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
		(void *)_memcpy(&(devices_connected.device_entry[sub_device - 1].dmx_device_info).rdm_sub_devices_info, sub_devices_info, sizeof(struct _rdm_sub_devices_info));
	}
}


