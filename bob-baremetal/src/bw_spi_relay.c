/**
 * @file bw_spi_relay.c
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

#ifdef __AVR_ARCH__
#include "avr_spi.h"
#else
#include "bcm2835.h"
#ifdef BARE_METAL
#include "bcm2835_spi.h"
#endif
#endif
#include "device_info.h"
#include "bw.h"
#include "bw_spi_relay.h"

static void bw_spi_relay_fsel_mask(const device_info_t *, const uint8_t);

/**
 * @ingroup SPI-DO
 *
 * @param device_info
 */
inline void static relay_spi_setup(const device_info_t *device_info) {
#ifdef __AVR_ARCH__
#else
	bcm2835_spi_setClockDivider(device_info->internal_clk_div);
	bcm2835_spi_chipSelect(device_info->chip_select);
	bcm2835_spi_setChipSelectPolarity(device_info->chip_select, LOW);
#endif
}

/**
 * @ingroup SPI-DO
 *
 * @param device_info
 * @return
 */
uint8_t bw_spi_relay_start(device_info_t *device_info) {
#if !defined(BARE_METAL) && !defined(__AVR_ARCH__)
	if (bcm2835_init() != 1)
		return 1;
#endif
	FUNC_PREFIX(spi_begin());

	if (device_info->slave_address == (uint8_t) 0) {
		device_info->slave_address = BW_RELAY_DEFAULT_SLAVE_ADDRESS;
	}

	if (device_info->speed_hz == (uint32_t) 0) {
		device_info->speed_hz = (uint32_t) BW_RELAY_SPI_SPEED_DEFAULT_HZ;
	} else if (device_info->speed_hz > (uint32_t) BW_RELAY_SPI_SPEED_MAX_HZ) {
		device_info->speed_hz = (uint32_t) BW_RELAY_SPI_SPEED_MAX_HZ;
	}
#ifdef __AVR_ARCH__
#else
	device_info->internal_clk_div = (uint16_t)((uint32_t) BCM2835_CORE_CLK_HZ / device_info->speed_hz);
#endif
	bw_spi_relay_fsel_mask(device_info, 0x7F);

	return 0;
}

/**
 * @ingroup SPI-DO
 *
 */
void bw_spi_relay_end(void) {
	FUNC_PREFIX(spi_end());
}

/**
 * @ingroup SPI-DO
 *
 * @param device_info
 * @param mask
 */
inline static void bw_spi_relay_fsel_mask(const device_info_t *device_info,	const uint8_t mask) {
	char cmd[3];
	cmd[0] = (char) device_info->slave_address;
	cmd[1] = (char) BW_PORT_WRITE_IO_DIRECTION;
	cmd[2] = (char) mask;

	relay_spi_setup(device_info);
	FUNC_PREFIX(spi_writenb(cmd, 3));
#if (BW_RELAY_SPI_BYTE_WAIT_US > 0)
	udelay(BW_RELAY_SPI_BYTE_WAIT_US);
#endif
}

/**
 * @ingroup SPI-DO
 *
 * @param device_info
 * @param pins
 */
void bw_spi_relay_output(const device_info_t *device_info, const uint8_t pins) {
	char cmd[3];
	cmd[0] = (char) device_info->slave_address;
	cmd[1] = (char) BW_PORT_WRITE_SET_ALL_OUTPUTS;
	cmd[2] = (char) pins;

	relay_spi_setup(device_info);
	FUNC_PREFIX(spi_writenb(cmd, 3));
#if (BW_RELAY_SPI_BYTE_WAIT_US > 0)
	udelay(BW_RELAY_SPI_BYTE_WAIT_US);
#endif
}

