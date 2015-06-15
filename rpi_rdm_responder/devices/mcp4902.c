/**
 * @file mcp4902.c
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
#include "bcm2835.h"
#include "bcm2835_spi.h"
#include "mcp49x2.h"

static const struct _rdm_personality rdm_sub_device[] = {
		{ 2, "Analog output 2-lines" }
		};

static struct _rdm_sub_devices_info rdm_sub_devices_info = { 2, 1, 1, 0, 0, "mcp4902", 7, &rdm_sub_device[0] };

/**
 * @ingroup DEV
 *
 * @param dmx_device_info
 */
static void mcp4902(dmx_device_info_t * dmx_device_info) {
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_16);	// 15.625MHz
	bcm2835_spi_chipSelect(dmx_device_info->device_info.chip_select);
	bcm2835_spi_setChipSelectPolarity(dmx_device_info->device_info.chip_select, LOW);

	int dmx_data_index = dmx_device_info->dmx_start_address;

	bcm2835_spi_write(MCP4902_DATA(dmx_data[dmx_data_index]) | 0x3000 | MCP49X2_WRITE_DAC_A);

	dmx_data_index++;

	if (dmx_data_index > DMX_UNIVERSE_SIZE)
		return;

	bcm2835_spi_write(MCP4902_DATA(dmx_data[dmx_data_index]) | 0x3000 | MCP49X2_WRITE_DAC_B);
}

INITIALIZER(devices, mcp4902)

/**
 * @ingroup DEV
 *
 * @param dmx_device_info
 */
static void mcp4902_init(dmx_device_info_t * dmx_device_info) {
#ifdef DEBUG
	printf("device init <mcp4902>\n");
#endif
	bcm2835_spi_begin();
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_16);	// 15.625MHz
	bcm2835_spi_chipSelect(dmx_device_info->device_info.chip_select);
	bcm2835_spi_setChipSelectPolarity(dmx_device_info->device_info.chip_select, LOW);
	bcm2835_spi_write(MCP4902_DATA(0) | 0x3000 | MCP49X2_WRITE_DAC_A);
	bcm2835_spi_write(MCP4902_DATA(0) | 0x3000 | MCP49X2_WRITE_DAC_B);

	dmx_device_info->rdm_sub_devices_info = &rdm_sub_devices_info;
	rdm_sub_devices_info.dmx_start_address = dmx_device_info->dmx_start_address;
}

INITIALIZER(devices_init, mcp4902_init)
