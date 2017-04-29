/**
 * @file mcp23s08.c
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

#include <stdint.h>

#include "tables.h"
#include "util.h"
#include "dmx.h"

#include "mcp23s08.h"

#define MCP23S08_IODIR	0x00	///< I/O DIRECTION (IODIR) REGISTER
#define MCP23S08_GPIO	0x09	///< PORT (GPIO) REGISTER

#define DMX_FOOTPRINT	8		///<

static const char device_label[] = "mcp23s08";
static const uint8_t device_label_len = MIN(sizeof(device_label) / sizeof(device_label[0]), RDM_DEVICE_LABEL_MAX_LENGTH);

static struct _rdm_personality rdm_personality = { DMX_FOOTPRINT, "Digital output 8-lines", 22 };
static struct _rdm_sub_devices_info sub_device_info = {DMX_FOOTPRINT, 1, 1, /* start address */0, /* sensor count */0, "", 0, &rdm_personality};

/**
 * @ingroup DEV
 *
 * @param dmx_device_info
 */
static void mcp23s08(dmx_device_info_t * dmx_device_info, const uint8_t *dmx_data) {
	int i;
	uint8_t data = 0;
	uint16_t dmx_data_index = dmx_device_info->dmx_start_address;

	for (i = 0; i < DMX_FOOTPRINT; i++) {

		if (dmx_data_index > DMX_UNIVERSE_SIZE)
			break;

		if ((dmx_data[dmx_data_index] & (uint8_t)0x80) != 0) {	// 0-127 is off, 128-255 is on
			data = data | (1 << i);
		}

		dmx_data_index++;
	}

	mcp23s08_reg_write(&dmx_device_info->device_info, MCP23S08_GPIO, data);
}

INITIALIZER(devices, mcp23s08)

/**
 *
 * @param dmx_device_info
 */
static void mcp23s08_zero(dmx_device_info_t *dmx_device_info, const uint8_t *dmx_data) {
	mcp23s08_reg_write(&dmx_device_info->device_info, MCP23S08_GPIO, 0x00);
}

INITIALIZER(devices_zero, mcp23s08_zero)

/**
 * @ingroup DEV
 *
 * @param dmx_device_info
 */
static void mcp23s08_init(dmx_device_info_t * dmx_device_info, const uint8_t *dmx_data) {
	struct _rdm_sub_devices_info *rdm_sub_devices_info =  &dmx_device_info->rdm_sub_devices_info;

	(void)mcp23s08_start(&(dmx_device_info->device_info));
	mcp23s08_reg_write(&dmx_device_info->device_info, MCP23S08_IODIR, 0x00);
	mcp23s08_reg_write(&dmx_device_info->device_info, MCP23S08_GPIO, 0x00);

	(void *) memcpy(rdm_sub_devices_info, &sub_device_info, sizeof(struct _rdm_sub_devices_info));
	dmx_device_info->rdm_sub_devices_info.dmx_start_address = dmx_device_info->dmx_start_address;
	(void *) memcpy(dmx_device_info->rdm_sub_devices_info.device_label, device_label, device_label_len);
	dmx_device_info->rdm_sub_devices_info.device_label_length = device_label_len;
}

INITIALIZER(devices_init, mcp23s08_init)
