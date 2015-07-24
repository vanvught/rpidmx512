/**
 * @file bw_spi_dio.c
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
#include "tables.h"
#include "dmx.h"
#include "bw_spi_dio.h"

static const struct _rdm_personality rdm_sub_device = { 7, "Digital output 7-lines", 22 };
static struct _rdm_sub_devices_info rdm_sub_devices_info = { 7, 1, 1, 0, 0, "bw_spi_dio", 10, &rdm_sub_device };

/**
 * @ingroup DEV
 *
 * @param dmx_device_info
 */
static void bw_spi_dio(dmx_device_info_t *dmx_device_info) {
	int i = 0;
	uint8_t data = 0;
	uint16_t dmx_data_index = dmx_device_info->dmx_start_address ;

	for (i = 0; i < 7; i++) {

		if (dmx_data_index > DMX_UNIVERSE_SIZE)
			break;

		if ((dmx_data[dmx_data_index] & (uint8_t)0x80) != 0) {	// 0-127 is off, 128-255 is on
			data = data | (uint8_t)(1 << i);
		}

		dmx_data_index++;
	}

	bw_spi_dio_output(&dmx_device_info->device_info, data);
}

INITIALIZER(devices, bw_spi_dio)

/**
 * @ingroup DEV
 *
 * @param dmx_device_info
 */
static void bw_spi_dio_init(dmx_device_info_t *dmx_device_info) {
#ifdef DEBUG
	printf("device init <bw_spi_dio_init>\n");
#endif
	(void)bw_spi_dio_start(&(dmx_device_info->device_info));
	bw_spi_dio_fsel_mask(&dmx_device_info->device_info, 0x7F);
	bw_spi_dio_output(&dmx_device_info->device_info, 0);

	dmx_device_info->rdm_sub_devices_info = &rdm_sub_devices_info;
	rdm_sub_devices_info.dmx_start_address = dmx_device_info->dmx_start_address;
}

INITIALIZER(devices_init, bw_spi_dio_init)
