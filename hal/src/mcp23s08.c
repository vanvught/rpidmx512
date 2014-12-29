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

#ifdef __AVR_ARCH__
#include <util/delay.h>
#include <avr_spi.h>
#else
#include <bcm2835.h>
#ifdef BARE_METAL
#include <bcm2835_spi.h>
#endif
#endif
#include <device_info.h>
#include <mcp23s08.h>

#ifdef __AVR_ARCH__
#define FUNC_PREFIX(x) avr_##x
#else
#define FUNC_PREFIX(x) bcm2835_##x
#endif

/**
 *
 * @param device_info
 */
inline void static mcp23s08_setup(const device_info_t *device_info) {
#ifdef __AVR_ARCH__
#else
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128);	// 1.953125MHz
	bcm2835_spi_chipSelect(device_info->chip_select);
#endif
}

/**
 *
 * @param device_info
 * @return
 */
uint8_t mcp23s08_start(device_info_t *device_info) {
#if !defined(BARE_METAL) && !defined(__AVR_ARCH__)
	if (bcm2835_init() != 1)
		return MCP23S08_ERROR;
#endif
	FUNC_PREFIX(spi_begin());

	if (device_info->slave_address <= 0)
		device_info->slave_address = MCP23S08_DEFAULT_SLAVE_ADDRESS;
	else
		device_info->slave_address = device_info->slave_address & 0x03;

	mcp23s08_reg_write(device_info, MCP23S08_IOCON, MCP23S08_IOCON_HAEN);

	return MCP23S08_OK;
}

/**
 *
 */
void mcp23s08_end (void) {
	FUNC_PREFIX(spi_end());
}

/**
 *
 * @param device_info
 * @param reg
 * @return
 */
uint8_t mcp23s08_reg_read(const device_info_t *device_info, const uint8_t reg) {
	char spiData[3];
	spiData[0] = MCP23S08_CMD_READ | ((device_info->slave_address) << 1);
	spiData[1] = reg;
	mcp23s08_setup(device_info);
	FUNC_PREFIX(spi_transfern(spiData, 3));
	return spiData[2];
}

/**
 *
 * @param device_info
 * @param reg
 * @param value
 */
void mcp23s08_reg_write(const device_info_t *device_info, const uint8_t reg, const uint8_t value) {
	char spiData[3];
	spiData[0] = MCP23S08_CMD_WRITE | ((device_info->slave_address) << 1);
	spiData[1] = reg;
	spiData[2] = value;
	mcp23s08_setup(device_info);
	FUNC_PREFIX(spi_transfern(spiData, 3));
}

/**
 *
 * @param device_info
 * @param pin
 * @param mode
 */
void mcp23s08_gpio_fsel(const device_info_t *device_info, const uint8_t pin, const uint8_t mode) {
	uint8_t data = mcp23s08_reg_read(device_info, MCP23S08_IODIR);
	if (mode == MCP23S08_FSEL_OUTP) {
		data &= (~pin);
	} else {
		data |= pin;
	}
	mcp23s08_reg_write(device_info, MCP23S08_IODIR, data);
}

/**
 *
 * @param device_info
 * @param pin
 */
void mcp23s08_gpio_set(const device_info_t *device_info, const uint8_t pin) {
	uint8_t data = mcp23s08_reg_read(device_info, MCP23S08_OLAT);
	data |= pin;
	mcp23s08_reg_write(device_info, MCP23S08_GPIO, data);
}

/**
 *
 * @param device_info
 * @param pin
 */
void mcp23s08_gpio_clr(const device_info_t *device_info, const uint8_t pin) {
	uint8_t data = mcp23s08_reg_read(device_info, MCP23S08_OLAT);
	data &= (~pin);
	mcp23s08_reg_write(device_info, MCP23S08_GPIO, data);
}
