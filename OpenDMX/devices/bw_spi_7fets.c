/**
 * @file bw_spi_7fets.c
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

extern int printf(const char *format, ...);

#include <tables.h>
#include <dmx_data.h>
#include <bw_spi_7fets.h>

/**
 *
 * @param dmx_device_info
 */
static void bw_spi_7fets(dmx_device_info_t *dmx_device_info) {
	int i;
	unsigned char data = 0;
	int dmx_data_index = dmx_device_info->dmx_start_address - 1;

	for (i = 0; i < 7; i++) {

		if (dmx_data_index > 0x1FF)
			break;

		if (dmx_data[dmx_data_index] & 0x80) {	// 0-127 is off, 128-255 is on
			data = data | (1 << i);
		}

		dmx_data_index++;
	}

	bw_spi_7fets_output(&dmx_device_info->device_info, data);
}

INITIALIZER(devices, bw_spi_7fets)

/**
 *
 * @param dmx_device_info
 */
static void bw_spi_7fets_init(dmx_device_info_t *dmx_device_info) {
	printf("device init <bw_spi_7fets_init>\n");
	bw_spi_7fets_start(&(dmx_device_info->device_info));
	bw_spi_7fets_output(&dmx_device_info->device_info, 0);
}

INITIALIZER(devices_init, bw_spi_7fets_init)
