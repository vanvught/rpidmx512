/**
 * @file ws2812b.c
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

#include <stdint.h>

#include "tables.h"
#include "util.h"
#include "dmx.h"

#include "bcm2835.h"
#include "bcm2835_spi.h"

#define WS2812B_HIGH_CODE			0xF8		///< b11111000
#define WS2812B_LOW_CODE			0xC0		///< b11000000
#define WS2812B_SLOTS_PER_PIXEL		3			///< RGB

#define DMX_FOOTPRINT				510			///< Overwritten by pixel count

static const char device_label[] = "ws2812";
static const uint8_t device_label_len = MIN(sizeof(device_label) / sizeof(device_label[0]), RDM_DEVICE_LABEL_MAX_LENGTH);

static struct _rdm_personality rdm_personality = { DMX_FOOTPRINT, "WS2812 RGB LED", 14 };
static struct _rdm_sub_devices_info sub_device_info = {DMX_FOOTPRINT, 1, 1, /* start address */0, /* sensor count */0, "", 0, &rdm_personality};

/**
 * @ingroup DEV
 *
 * @param dmx_device_info
 */
static void ws2812b(dmx_device_info_t * dmx_device_info, const uint8_t *dmx_data) {
	int i;
	uint8_t mask = 0x80;
	uint16_t dmx_data_index = dmx_device_info->dmx_start_address;

	bcm2835_spi_setClockDivider((uint16_t) ((uint32_t) BCM2835_CORE_CLK_HZ / (uint32_t) 6400000));
	bcm2835_spi_chipSelect(dmx_device_info->device_info.chip_select);					// Just in case we have a multiplexer
	bcm2835_spi_setChipSelectPolarity(dmx_device_info->device_info.chip_select, LOW);	// Just in case we have a multiplexer

	// Clear TX and RX fifos
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);
	// Set TA = 1
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);

	for (i = 0; i < ((int)dmx_device_info->pixel_count * (int)WS2812B_SLOTS_PER_PIXEL); i++) {
		if (dmx_data_index > DMX_UNIVERSE_SIZE) {
			break;
		}

		mask = 0x80;

		while (mask != 0) {
			// Maybe wait for TXD
			while (!(BCM2835_SPI0->CS & BCM2835_SPI0_CS_TXD))
				;

			if (dmx_data[dmx_data_index] & mask) {
				BCM2835_SPI0->FIFO = (uint32_t) WS2812B_HIGH_CODE;
			} else {
				BCM2835_SPI0->FIFO = (uint32_t) WS2812B_LOW_CODE;
			}

			while ((BCM2835_SPI0->CS & BCM2835_SPI0_CS_RXD)) {
				(void) BCM2835_SPI0->FIFO;
			}

			mask >>= 1;
		}

		dmx_data_index++;
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

INITIALIZER(devices, ws2812b)

/**
 *
 * @param dmx_device_info
 */
static void ws2812b_zero(dmx_device_info_t *dmx_device_info, const uint8_t *dmx_data) {
	int i,j;

	bcm2835_spi_setClockDivider((uint16_t) ((uint32_t) BCM2835_CORE_CLK_HZ / (uint32_t) 6400000));
	bcm2835_spi_chipSelect(dmx_device_info->device_info.chip_select);					// Just in case we have a multiplexer
	bcm2835_spi_setChipSelectPolarity(dmx_device_info->device_info.chip_select, LOW);	// Just in case we have a multiplexer
	// Clear TX and RX fifos
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);
	// Set TA = 1
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);

	for (i = 0; i < ((int) dmx_device_info->pixel_count * (int) WS2812B_SLOTS_PER_PIXEL); i++) {
		for (j = 0; j < 8; j++) {
			// Maybe wait for TXD
			while (!(BCM2835_SPI0->CS & BCM2835_SPI0_CS_TXD))
				;

			BCM2835_SPI0->FIFO = (uint32_t) WS2812B_LOW_CODE;

			while ((BCM2835_SPI0->CS & BCM2835_SPI0_CS_RXD)) {
				(void) BCM2835_SPI0->FIFO;
			}
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

INITIALIZER(devices_zero, ws2812b_zero)

/**
 * @ingroup DEV
 *
 * @param dmx_device_info
 */
static void ws2812b_init(dmx_device_info_t * dmx_device_info, const uint8_t *dmx_data) {
	struct _rdm_sub_devices_info *rdm_sub_devices_info =  &(dmx_device_info)->rdm_sub_devices_info;

	bcm2835_spi_begin();

	(void *) memcpy(rdm_sub_devices_info, &sub_device_info, sizeof(struct _rdm_sub_devices_info));
	dmx_device_info->rdm_sub_devices_info.dmx_start_address = dmx_device_info->dmx_start_address;
	(void *) memcpy(dmx_device_info->rdm_sub_devices_info.device_label, device_label, device_label_len);
	dmx_device_info->rdm_sub_devices_info.device_label_length = device_label_len;

	// WS2812B specific
	if ((dmx_device_info->pixel_count == (uint8_t)0) || ((uint16_t) dmx_device_info->pixel_count * (uint16_t) WS2812B_SLOTS_PER_PIXEL > (uint16_t) DMX_UNIVERSE_SIZE)) {
		dmx_device_info->pixel_count = (uint8_t) ((uint16_t) DMX_UNIVERSE_SIZE / (uint16_t) WS2812B_SLOTS_PER_PIXEL);
	}

	dmx_device_info->rdm_sub_devices_info.dmx_footprint = dmx_device_info->pixel_count * (uint16_t) WS2812B_SLOTS_PER_PIXEL;
	dmx_device_info->rdm_sub_devices_info.rdm_personalities->slots = dmx_device_info->pixel_count * (uint16_t) WS2812B_SLOTS_PER_PIXEL;
}

INITIALIZER(devices_init, ws2812b_init)
