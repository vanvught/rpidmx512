#if !defined(H3)
/**
 * @file sc16is740.c
 *
 */
/* This code is inspired by:
 *
 * https://developer.mbed.org/components/SC16IS750-I2C-or-SPI-to-UART-bridge/
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

#include <stdbool.h>
#include <stdint.h>

#include "sc16is7x0.h"

#if defined(__linux__) || defined(__circle__)
 #include "bcm2835.h"
#else
 #include "bcm2835_spi.h"
#endif

#include "device_info.h"

static char buffer_tx[SC16IS7X0_FIFO_TX + 1] __attribute__((aligned(4)));

/**
 *
 * @param device_info
 */
inline static void sc16is740_setup(const device_info_t *device_info) {
	bcm2835_spi_setClockDivider(device_info->internal.clk_div);
	bcm2835_spi_chipSelect(device_info->chip_select);
}

/**
 *
 * @param device_info
 * @param reg
 * @return
 */
uint8_t sc16is740_reg_read(const device_info_t *device_info, const uint8_t reg) {
	char spiData[2];
	const char SPI_DUMMY_CHAR = (char) 0xFF;	///< Used to flush slave's shift register

	spiData[0] = (char) SC16IS7X0_SPI_READ_MODE_FLAG | (char) (reg << 3);
	spiData[1] = SPI_DUMMY_CHAR;

	sc16is740_setup(device_info);
	bcm2835_spi_transfern(spiData, 2);

	return (uint8_t) spiData[1];
}

/**
 *
 * @param device_info
 * @param reg
 * @param value
 */
void sc16is740_reg_write(const device_info_t *device_info, const uint8_t reg, const uint8_t value) {
	char spiData[2];
	spiData[0] = (char) (reg << 3);
	spiData[1] = (char) value;

	sc16is740_setup(device_info);
	bcm2835_spi_writenb(spiData, 2);
}

/**
 *
 * @param device_info
 * @return
 */
bool sc16is740_is_readable(const device_info_t *device_info) {
	if ((sc16is740_reg_read(device_info, SC16IS7X0_LSR) & LSR_DR) == LSR_DR) {
		return true;
	} else {
		return false;
	}
}

/**
 *
 * @return
 */
bool sc16is740_is_writable(const device_info_t *device_info) {
	if ((sc16is740_reg_read(device_info, SC16IS7X0_LSR) & LSR_THRE) == LSR_THRE) {
		return true;
	} else {
		return false;
	}
}

/**
 *
 * @param device_info
 * @return
 */
int sc16is740_getc(const device_info_t *device_info) {
	  if (!sc16is740_is_readable(device_info)) {
	    return -1;
	  }

	  return (int) sc16is740_reg_read(device_info, SC16IS7X0_RHR);
}

/**
 *
 * @param device_info
 * @param value
 * @return
 */
int sc16is740_putc(const device_info_t *device_info, const int value) {
	while (sc16is740_reg_read(device_info, SC16IS7X0_TXLVL) == 0) {

	}

	sc16is740_reg_write(device_info, SC16IS7X0_THR, (uint8_t) value);

	return value;
}

/**
 *
 * @return
 */
bool sc16is740_is_connected(const device_info_t *device_info) {
	const uint8_t TEST_CHARACTER = (uint8_t) 'A';

	sc16is740_reg_write(device_info, SC16IS7X0_SPR, TEST_CHARACTER);

	return (sc16is740_reg_read(device_info, SC16IS7X0_SPR) == TEST_CHARACTER);
}

/**
 *
 * @param buffer
 * @param count
 * @return
 */
int sc16is740_read(const device_info_t *device_info, void *buffer, unsigned count) {
	uint8_t *p = (uint8_t *) buffer;

	int result = 0;

	while (count-- != 0) {
		int ch = sc16is740_getc(device_info);
		if (ch < 0) {
			return result;
		}

		*p++ = (uint8_t) ch;

		result++;
	}

	return result;
}

/**
 *
 * @param buffer
 * @param count
 * @return
 */
int sc16is740_write(const device_info_t *device_info, const void *buffer, unsigned count) {
	int result = 0;
	uint8_t fifo_space;
	uint8_t i;
	char *src = (char *) buffer;
	char *dst;

	while (count > 0) {
		while ((fifo_space = sc16is740_reg_read(device_info, SC16IS7X0_TXLVL)) == 0) {
		}

		if ((unsigned) fifo_space > count) {
			fifo_space = (uint8_t) count;
		}

		dst = &buffer_tx[1];

		for (i = 0; i < fifo_space; i++) {
			*dst++ = *src++;
		}

		sc16is740_setup(device_info);
		bcm2835_spi_writenb(buffer_tx, fifo_space + 1);

		count -= (unsigned) fifo_space;

	}

	return result;
}

/**
 *
 * @param device_info
 * @param baudrate
 */
void sc16is740_set_baud(const device_info_t *device_info, const int baudrate) {
	unsigned long divisor = (unsigned long) SC16IS7X0_BAUDRATE_DIVISOR(baudrate);
	uint8_t lcr;

	lcr = sc16is740_reg_read(device_info, SC16IS7X0_LCR);

	sc16is740_reg_write(device_info, SC16IS7X0_LCR, lcr | LCR_ENABLE_DIV);
	sc16is740_reg_write(device_info, SC16IS7X0_DLL, (uint8_t) (divisor & 0xFF));
	sc16is740_reg_write(device_info, SC16IS7X0_DLH, (uint8_t) ((divisor >> 8) & 0xFF));
	sc16is740_reg_write(device_info, SC16IS7X0_LCR, lcr);
}

/** Set the transmission format used by the serial port.
  *   @param bits      The number of bits in a word (5-8; default = 8)
  *   @param parity    The parity used (\ref_serial_parity)
  *   @param stop_bits The number of stop bits (1 or 2; default = 1)
  *   @return none
  */
void sc16is740_set_format(const device_info_t *device_info, int bits, _serial_parity parity,  int stop_bits) {
	uint8_t lcr = 0x00;

	switch (bits) {
	case 5:
		lcr |= LCR_BITS5;
		break;
	case 6:
		lcr |= LCR_BITS6;
		break;
	case 7:
		lcr |= LCR_BITS7;
		break;
	case 8:
		lcr |= LCR_BITS8;
		break;
	default:
		lcr |= LCR_BITS8;
	}

	switch (parity) {
	case SERIAL_PARITY_NONE:
		lcr |= LCR_NONE;
		break;
	case SERIAL_PARITY_ODD:
		lcr |= LCR_ODD;
		break;
	case SERIAL_PARITY_EVEN:
		lcr |= LCR_EVEN;
		break;
	case SERIAL_PARITY_FORCED1:
		lcr |= LCR_FORCED1;
		break;
	case SERIAL_PARITY_FORCED0:
		lcr |= LCR_FORCED0;
		break;
	default:
		lcr |= LCR_NONE;
	}

	switch (stop_bits) {
	case 1:
		lcr |= LCR_BITS1;
		break;
	case 2:
		lcr |= LCR_BITS2;
		break;
	default:
		lcr |= LCR_BITS1;
	}

	sc16is740_reg_write(device_info, SC16IS7X0_LCR, lcr);
}

/**
 *
 * @param device_info
 * @return
 */
void sc16is740_start(device_info_t *device_info) {

	bcm2835_spi_begin();

	if (device_info->speed_hz == (uint32_t) 0) {
		device_info->speed_hz = (uint32_t) SC16IS7X0_SPI_SPEED_DEFAULT_HZ;
	} else if (device_info->speed_hz > (uint32_t) SC16IS7X0_SPI_SPEED_MAX_HZ) {
		device_info->speed_hz = (uint32_t) SC16IS7X0_SPI_SPEED_MAX_HZ;
	}

	device_info->internal.clk_div = (uint16_t)((uint32_t) BCM2835_CORE_CLK_HZ / device_info->speed_hz);

	sc16is740_set_format(device_info, 8, SERIAL_PARITY_NONE, 1);
	sc16is740_set_baud(device_info, SC16IS7X0_DEFAULT_BAUDRATE);

	sc16is740_reg_write(device_info, SC16IS7X0_FCR, (uint8_t) (FCR_ENABLE_FIFO | FCR_RX_FIFO_RST | FCR_TX_FIFO_RST));

	buffer_tx[0] = (char) (SC16IS7X0_THR << 3);
}
#endif //FIXME
