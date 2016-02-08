/**
 * @file mcp4822.c
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

#include "tables.h"
#include "util.h"
#include "dmx.h"
#include "bcm2835.h"
#include "bcm2835_spi.h"
#include "mcp48x2.h"

#define DMX_FOOTPRINT	2

static const char device_label[] = "mcp4822";
static const uint8_t device_label_len = MIN(sizeof(device_label) / sizeof(device_label[0]), RDM_DEVICE_LABEL_MAX_LENGTH);

static const struct _rdm_personality rdm_personality = { DMX_FOOTPRINT, "Analog output 2-lines", 21 };
static const struct _rdm_sub_devices_info sub_device_info = {DMX_FOOTPRINT, 1, 1, /* start address */0, /* sensor count */0, "", 0, &rdm_personality};

/**
 * @ingroup DEV
 *
 * @param dmx_device_info
 */
static void mcp4822(dmx_device_info_t * dmx_device_info) {
	const uint8_t *dmx_data = dmx_get_data();
	uint16_t dmx_data_index;
	uint16_t data;

	bcm2835_spi_setClockDivider(dmx_device_info->device_info.internal_clk_div);
	bcm2835_spi_chipSelect(dmx_device_info->device_info.chip_select);
	bcm2835_spi_setChipSelectPolarity(dmx_device_info->device_info.chip_select, LOW);

	dmx_data_index = dmx_device_info->dmx_start_address;

	data = (uint16_t)((uint16_t)(dmx_data[dmx_data_index] << 4) | (uint16_t)(dmx_data[dmx_data_index] >> 4));
	bcm2835_spi_write(MCP4822_DATA(data) | 0x3000 | MCP48X2_WRITE_DAC_A);

	dmx_data_index++;

	if (dmx_data_index > DMX_UNIVERSE_SIZE)
		return;

	data = (uint16_t)((uint16_t)(dmx_data[dmx_data_index] << 4) | (uint16_t)(dmx_data[dmx_data_index] >> 4));
	bcm2835_spi_write(MCP4822_DATA(data) | 0x3000 | MCP48X2_WRITE_DAC_B);
}

INITIALIZER(devices, mcp4822)

/**
 *
 * @param dmx_device_info
 */
static void mcp4822_zero(dmx_device_info_t *dmx_device_info) {
	bcm2835_spi_setClockDivider(dmx_device_info->device_info.internal_clk_div);
	bcm2835_spi_chipSelect(dmx_device_info->device_info.chip_select);
	bcm2835_spi_setChipSelectPolarity(dmx_device_info->device_info.chip_select, LOW);

	bcm2835_spi_write(MCP4822_DATA(0) | 0x3000 | MCP48X2_WRITE_DAC_A);
	bcm2835_spi_write(MCP4822_DATA(0) | 0x3000 | MCP48X2_WRITE_DAC_B);
}

INITIALIZER(devices_zero, mcp4822_zero)

/**
 * @ingroup DEV
 *
 * @param dmx_device_info
 */
static void mcp4822_init(dmx_device_info_t * dmx_device_info) {
	struct _rdm_sub_devices_info *rdm_sub_devices_info =  &(dmx_device_info)->rdm_sub_devices_info;

	if (dmx_device_info->device_info.speed_hz == (uint32_t) 0) {
		dmx_device_info->device_info.speed_hz = (uint32_t) MCP48X2_SPI_SPEED_DEFAULT_HZ;
	} else if (dmx_device_info->device_info.speed_hz > (uint32_t) MCP48X2_SPI_SPEED_MAX_HZ) {
		dmx_device_info->device_info.speed_hz = (uint32_t) MCP48X2_SPI_SPEED_MAX_HZ;
	}

	dmx_device_info->device_info.internal_clk_div = (uint16_t) ((uint32_t) BCM2835_CORE_CLK_HZ / dmx_device_info->device_info.speed_hz);

	bcm2835_spi_begin();
	bcm2835_spi_setClockDivider(dmx_device_info->device_info.internal_clk_div);
	bcm2835_spi_chipSelect(dmx_device_info->device_info.chip_select);
	bcm2835_spi_setChipSelectPolarity(dmx_device_info->device_info.chip_select, LOW);

	bcm2835_spi_write(MCP4822_DATA(0) | 0x3000 | MCP48X2_WRITE_DAC_A);
	bcm2835_spi_write(MCP4822_DATA(0) | 0x3000 | MCP48X2_WRITE_DAC_B);

	(void *)_memcpy(rdm_sub_devices_info, &sub_device_info, sizeof(struct _rdm_sub_devices_info));
	dmx_device_info->rdm_sub_devices_info.dmx_start_address = dmx_device_info->dmx_start_address;
	(void *)_memcpy(dmx_device_info->rdm_sub_devices_info.device_label, device_label, device_label_len);
	dmx_device_info->rdm_sub_devices_info.device_label_length = device_label_len;
}

INITIALIZER(devices_init, mcp4822_init)
