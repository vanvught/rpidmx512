/**
 * @file bw_spi_relay.c
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdbool.h>

#include "bob.h"

#include "bw.h"
#include "bw_spi.h"
#include "bw_spi_relay.h"

inline static void bw_spi_relay_fsel_mask(const device_info_t *device_info,	uint8_t mask) {
	char cmd[3];

	cmd[0] = (char) device_info->slave_address;
	cmd[1] = (char) BW_PORT_WRITE_IO_DIRECTION;
	cmd[2] = (char) mask;

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0]));
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
		bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0]));
	}
}

bool bw_spi_relay_start(device_info_t *device_info) {

	if (device_info->slave_address == (uint8_t) 0) {
		device_info->slave_address = BW_RELAY_DEFAULT_SLAVE_ADDRESS;
	}

	if (device_info->speed_hz == (uint32_t) 0) {
		device_info->speed_hz = (uint32_t) BW_RELAY_SPI_SPEED_DEFAULT_HZ;
	} else if (device_info->speed_hz > (uint32_t) BW_RELAY_SPI_SPEED_MAX_HZ) {
		device_info->speed_hz = (uint32_t) BW_RELAY_SPI_SPEED_MAX_HZ;
	}

	if (device_info->chip_select >= SPI_CS2) {
		device_info->chip_select = SPI_CS2;
		bcm2835_aux_spi_begin();
		device_info->internal.clk_div = bcm2835_aux_spi_CalcClockDivider(device_info->speed_hz);
	} else {
		bcm2835_spi_begin();
		device_info->internal.clk_div = (uint16_t)((uint32_t) BCM2835_CORE_CLK_HZ / device_info->speed_hz);
	}

	char id[BW_ID_STRING_LENGTH+1];
	bw_spi_read_id(device_info, id);

	if (memcmp(id, "spi_relay", 9) == 0) {
		bw_spi_relay_fsel_mask(device_info, 0x7F);
		return true;
	}

	return false;
}

void bw_spi_relay_output(const device_info_t *device_info, uint8_t pins) {
	char cmd[3];

	cmd[0] = (char) device_info->slave_address;
	cmd[1] = (char) BW_PORT_WRITE_SET_ALL_OUTPUTS;
	cmd[2] = (char) pins;

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0]));
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
		bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0]));
	}
}

