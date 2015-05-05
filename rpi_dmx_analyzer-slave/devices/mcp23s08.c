/**
 * @file mcp23s08.c
 *
 */
/* Copyright (C) 2014 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#ifdef DEBUG
extern int printf(const char *format, ...);
#endif
#include <tables.h>
#include <dmx_data.h>
#include <mcp23s08.h>

/**
 * @ingroup DEV
 *
 * @param dmx_device_info
 */
static void mcp23s08(dmx_device_info_t * dmx_device_info) {
	int i = 0;
	unsigned char data = 0;
	int dmx_data_index = dmx_device_info->dmx_start_address;

	for (i = 0; i < 8; i++) {

		if (dmx_data_index > DMX_UNIVERSE_SIZE)
			break;

		if (dmx_data[dmx_data_index] & 0x80) {	// 0-127 is off, 128-255 is on
			data = data | (1 << i);
		}

		dmx_data_index++;
	}

	mcp23s08_reg_write(&dmx_device_info->device_info, MCP23S08_GPIO, data);
}

INITIALIZER(devices, mcp23s08)

/**
 * @ingroup DEV
 *
 * @param dmx_device_info
 */
static void mcp23s08_init(dmx_device_info_t * dmx_device_info) {
#ifdef DEBUG
	printf("device init <mcp23s08>\n");
#endif
	mcp23s08_start(&(dmx_device_info->device_info));
	mcp23s08_reg_write(&dmx_device_info->device_info, MCP23S08_IODIR, 0x00);
	mcp23s08_reg_write(&dmx_device_info->device_info, MCP23S08_GPIO, 0x00);
}

INITIALIZER(devices_init, mcp23s08_init)
