/**
 * @file ws2801.c
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#define WS2801_SPI_SPEED_MAX_HZ		25000000	///< 25 MHz
#define WS2801_SPI_SPEED_DEFAULT_HZ	4000000		///< 4 MHz
#define WS2801_SLOTS_PER_PIXEL		3			///< RGB

#define DMX_FOOTPRINT				510			///< Overwritten by pixel count

static const char device_label[] = "ws2801";
static const uint8_t device_label_len = MIN(sizeof(device_label) / sizeof(device_label[0]), RDM_DEVICE_LABEL_MAX_LENGTH);

static struct _rdm_personality rdm_personality = { DMX_FOOTPRINT, "WS2801 RGB LED", 14 };
static struct _rdm_sub_devices_info sub_device_info = {DMX_FOOTPRINT, 1, 1, /* start address */0, /* sensor count */0, "", 0, &rdm_personality};

/**
 * @ingroup DEV
 *
 * @param dmx_device_info
 */
static void ws2801(dmx_device_info_t * dmx_device_info, const uint8_t *dmx_data) {
	const char *data = (char* )&dmx_data[dmx_device_info->dmx_start_address];

	if ((((uint16_t) dmx_device_info->pixel_count * (uint16_t) WS2801_SLOTS_PER_PIXEL) + dmx_device_info->dmx_start_address - (uint16_t) 1) > (uint16_t) DMX_UNIVERSE_SIZE) {
		return;
	}

	bcm2835_spi_setClockDivider(dmx_device_info->device_info.internal_clk_div);
	bcm2835_spi_chipSelect(dmx_device_info->device_info.chip_select);					// Just in case we have a multiplexer
	bcm2835_spi_setChipSelectPolarity(dmx_device_info->device_info.chip_select, LOW);	// Just in case we have a multiplexer

	bcm2835_spi_writenb(data, (uint32_t) dmx_device_info->pixel_count * (uint32_t) WS2801_SLOTS_PER_PIXEL);
}

INITIALIZER(devices, ws2801)

/**
 *
 * @param dmx_device_info
 */
static void ws2801_zero(dmx_device_info_t *dmx_device_info, const uint8_t *dmx_data) {
	int i;

	if ((((uint16_t) dmx_device_info->pixel_count * (uint16_t) WS2801_SLOTS_PER_PIXEL) + dmx_device_info->dmx_start_address - (uint16_t) 1) > (uint16_t) DMX_UNIVERSE_SIZE) {
		return;
	}

	bcm2835_spi_setClockDivider(dmx_device_info->device_info.internal_clk_div);
	bcm2835_spi_chipSelect(dmx_device_info->device_info.chip_select);					// Just in case we have a multiplexer
	bcm2835_spi_setChipSelectPolarity(dmx_device_info->device_info.chip_select, LOW);	// Just in case we have a multiplexer

	// Clear TX and RX fifos
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);
	// Set TA = 1
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);

	for (i = 0; i < ((int) dmx_device_info->pixel_count * (int) WS2801_SLOTS_PER_PIXEL); i++) {
		// Maybe wait for TXD
		while (!(BCM2835_SPI0->CS & BCM2835_SPI0_CS_TXD))
			;
		// Write to FIFO
		BCM2835_SPI0->FIFO = (uint32_t) 0;

		while ((BCM2835_SPI0->CS & BCM2835_SPI0_CS_RXD)) {
			(void) BCM2835_SPI0->FIFO;
		}
	}

	// Wait for DONE to be set
	while (!(BCM2835_SPI0->CS & BCM2835_SPI0_CS_DONE)) {
		while ((BCM2835_SPI0->CS & BCM2835_SPI0_CS_RXD)) {
			(void) BCM2835_SPI0->FIFO;
		}
	}

	// Set TA = 0
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, 0, BCM2835_SPI0_CS_TA);
}

INITIALIZER(devices_zero, ws2801_zero)

/**
 * @ingroup DEV
 *
 * @param dmx_device_info
 */
static void ws2801_init(dmx_device_info_t * dmx_device_info, const uint8_t *dmx_data) {
	struct _rdm_sub_devices_info *rdm_sub_devices_info =  &(dmx_device_info)->rdm_sub_devices_info;

	if (dmx_device_info->device_info.speed_hz == (uint32_t) 0) {
		dmx_device_info->device_info.speed_hz = (uint32_t) WS2801_SPI_SPEED_DEFAULT_HZ;
	} else if (dmx_device_info->device_info.speed_hz > (uint32_t) WS2801_SPI_SPEED_MAX_HZ) {
		dmx_device_info->device_info.speed_hz = (uint32_t) WS2801_SPI_SPEED_MAX_HZ;
	}

	dmx_device_info->device_info.internal_clk_div = (uint16_t) ((uint32_t) BCM2835_CORE_CLK_HZ / dmx_device_info->device_info.speed_hz);

	bcm2835_spi_begin();
	bcm2835_spi_setClockDivider(dmx_device_info->device_info.internal_clk_div);
	bcm2835_spi_chipSelect(dmx_device_info->device_info.chip_select);
	bcm2835_spi_setChipSelectPolarity(dmx_device_info->device_info.chip_select, LOW);

	(void *)_memcpy(rdm_sub_devices_info, &sub_device_info, sizeof(struct _rdm_sub_devices_info));
	dmx_device_info->rdm_sub_devices_info.dmx_start_address = dmx_device_info->dmx_start_address;
	(void *)_memcpy(dmx_device_info->rdm_sub_devices_info.device_label, device_label, device_label_len);
	dmx_device_info->rdm_sub_devices_info.device_label_length = device_label_len;

	// WS2801 specific
	if ((dmx_device_info->pixel_count == (uint8_t)0) || ((uint16_t) dmx_device_info->pixel_count * (uint16_t) WS2801_SLOTS_PER_PIXEL > (uint16_t) DMX_UNIVERSE_SIZE)) {
		dmx_device_info->pixel_count = (uint8_t) ((uint16_t) DMX_UNIVERSE_SIZE / (uint16_t) WS2801_SLOTS_PER_PIXEL);
	}

	dmx_device_info->rdm_sub_devices_info.dmx_footprint = dmx_device_info->pixel_count * (uint16_t) WS2801_SLOTS_PER_PIXEL;
	dmx_device_info->rdm_sub_devices_info.rdm_personalities->slots = dmx_device_info->pixel_count * (uint16_t) WS2801_SLOTS_PER_PIXEL;
}

INITIALIZER(devices_init, ws2801_init)
