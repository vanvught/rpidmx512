/**
 * @file bw_spi_dimmer.c
 *
 */
/* Copyright (C) 2016, 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "bcm2835_spi.h"
#include "bcm2835_aux_spi.h"

#include "bw.h"
#include "bw_dimmer.h"
#include "bw_spi_dimmer.h"

#include "device_info.h"

/**
 * @ingroup SPI-AO
 *
 * @param device_info
 * @return
 */
void bw_spi_dimmer_start(device_info_t *device_info) {

	if (device_info->slave_address == (uint8_t) 0) {
		device_info->slave_address = BW_DIMMER_DEFAULT_SLAVE_ADDRESS;
	}

	if (device_info->speed_hz == (uint32_t) 0) {
		device_info->speed_hz = (uint32_t) BW_DIMMER_SPI_SPEED_DEFAULT_HZ;
	} else if (device_info->speed_hz > (uint32_t) BW_DIMMER_SPI_SPEED_MAX_HZ) {
		device_info->speed_hz = (uint32_t) BW_DIMMER_SPI_SPEED_MAX_HZ;
	}

	if (device_info->chip_select == (uint8_t) 2) {
		bcm2835_aux_spi_begin();
		device_info->internal_clk_div = bcm2835_aux_spi_CalcClockDivider(device_info->speed_hz);
	} else {
		bcm2835_spi_begin();
		device_info->internal_clk_div = (uint16_t)((uint32_t) BCM2835_CORE_CLK_HZ / device_info->speed_hz);
	}
}

/**
 * @ingroup SPI-AO
 *
 * @param device_info
 * @param value The value written can be between 0 and 255, where 0 is the lowest intensity and 255 is the highest intensity.
 */
void bw_spi_dimmer_output(const device_info_t *device_info, const uint8_t value) {
	char cmd[3];

	cmd[0] = (char) device_info->slave_address;
	cmd[1] = (char) BW_PORT_WRITE_DIMMER;
	cmd[2] = (char) value;

	if (device_info->chip_select == (uint8_t) 2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal_clk_div);
		bcm2835_aux_spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0]));
	} else {
		bcm2835_spi_setClockDivider(device_info->internal_clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0]));
	}
}
