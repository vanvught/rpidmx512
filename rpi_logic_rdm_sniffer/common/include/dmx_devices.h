/**
 * @file dmx_devices.h
 *
 */
/* Copyright (C) 2014, 2015, 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef DMX_DEVICES_H_
#define DMX_DEVICES_H_

#include <stdint.h>

#include "util.h"
#include "device_info.h"
#include "rdm_sub_devices.h"

#define DMX_DEVICE_CONFIG_TABLE_FULL 				0	///<
#define DMX_DEVICE_CONFIG_INVALID_PROTOCOL 			-2	///<
#define DMX_DEVICE_CONFIG_INVALID_CHIP_SELECT 		-3	///<
#define DMX_DEVICE_CONFIG_INVALID_SLAVE_ADDRESS 	-4	///<
#define DMX_DEVICE_CONFIG_INVALID_START_ADDRESS 	-5	///<
#define DMX_DEVICE_CONFIG_INVALID_DEVICE		 	-6	///<
#define DMX_DEVICE_CONFIG_INVALID_ENTRY			 	-7	///<
#define DMX_DEVICE_CONFIG_INVALID_SPI_SPEED			-8	///<
#define DMX_DEVICE_CONFIG_INVALID_PIXELS			-9	///<

typedef struct _dmx_device_info {
	device_info_t device_info;
	uint16_t dmx_start_address;
	uint8_t pixel_count;
	struct _rdm_sub_devices_info rdm_sub_devices_info;
} dmx_device_info_t;

typedef struct _device_entry {
	int devices_table_index;
	dmx_device_info_t dmx_device_info;
} device_entry_t;

typedef struct _devices {
	uint16_t elements_count;
	device_entry_t device_entry[255];
} devices_t;

struct _dmx_devices_statistics {
	uint32_t function_count;
	uint32_t dmx_available_count;
	uint32_t dmx_missed_count;
};

extern void dmx_devices_read_config(void);
extern void dmx_devices_init(void);
extern void dmx_devices_run(void);

extern /*@shared@*/const struct _dmx_devices_statistics *dmx_devices_get_statistics(void) ASSUME_ALIGNED;
extern void dmx_devices_reset_statistics(void);

extern const uint16_t dmx_devices_get_devices_connected(void);

extern const uint16_t dmx_devices_get_footprint(const uint16_t);
extern const uint16_t dmx_devices_get_dmx_start_address(const uint16_t);
extern void dmx_devices_set_dmx_start_address(const uint16_t, const uint16_t);
extern /*@shared@*//*@null@*/const char * dmx_devices_get_label(const uint16_t);
extern void dmx_devices_set_label(const uint16_t, const uint8_t *, uint8_t);
extern const uint8_t dmx_devices_get_label_length(const uint16_t);
extern /*@shared@*//*@null@*/const struct _rdm_sub_devices_info *dmx_devices_info_get(const uint16_t);
extern void dmx_devices_info_set(const uint16_t, const struct _rdm_sub_devices_info *);
extern const uint8_t dmx_devices_get_personality_current(const uint16_t);
extern void dmx_devices_set_personality_current(const uint16_t, const uint8_t);
extern /*@shared@*//*@null@*/const char *dmx_devices_get_personality_description(const uint16_t, const uint8_t) ASSUME_ALIGNED;
extern const uint8_t dmx_devices_get_personality_description_length(const uint16_t, const uint8_t);
extern const uint16_t dmx_devices_get_personality_slots(const uint16_t, const uint8_t);

extern devices_t *dmx_devices_get_devices(void) ASSUME_ALIGNED;

#endif /* DMX_DEVICES_H_ */
