/**
 * @file spi_flash.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#include "debug.h"

#include "../spi_flash_internal.h"

#include "h3.h"
#include "h3_spi.h"
#include "h3_spi_internal.h"
#include "h3_gpio.h"
#include "h3_ccu.h"

struct spi0_status {
	bool		transfer_active;
	uint8_t		*rxbuf;
	uint32_t	rxcnt;
	uint8_t		*txbuf;
	uint32_t	txcnt;
	uint32_t	txlen;
	uint32_t	rxlen;
} static s_spi0_status;

static void spi0_set_chip_select(void) {
	uint32_t value = H3_SPI0->TC;

	value |= TC_SS_OWNER;	// Software controlled

	H3_SPI0->TC = value;
}

static void spi0_set_data_mode(h3_spi_mode_t mode) {
	uint32_t value = H3_SPI0->TC;

	value &= ~TC_CPHA;
	value &= ~TC_CPOL;
	value |= (mode & 0x3);

	H3_SPI0->TC = value;
}

static void spi0_begin(void) {
	h3_gpio_fsel(H3_PORT_TO_GPIO(H3_GPIO_PORTC, 3), H3_PC3_SELECT_SPI0_CS);
	h3_gpio_fsel(H3_PORT_TO_GPIO(H3_GPIO_PORTC, 2), H3_PC2_SELECT_SPI0_CLK);
	h3_gpio_fsel(H3_PORT_TO_GPIO(H3_GPIO_PORTC, 1), H3_PC1_SELECT_SPI0_MISO);
	h3_gpio_fsel(H3_PORT_TO_GPIO(H3_GPIO_PORTC, 0), H3_PC0_SELECT_SPI0_MOSI);

	H3_CCU->BUS_SOFT_RESET0 |= CCU_BUS_SOFT_RESET0_SPI0;
	udelay(1000); // 1ms
	H3_CCU->BUS_CLK_GATING0 &= ~CCU_BUS_CLK_GATING0_SPI0;
	udelay(1000); // 1ms
	H3_CCU->BUS_CLK_GATING0 |= CCU_BUS_CLK_GATING0_SPI0;
	H3_CCU->SPI0_CLK = (1 << 31) | (0x01 << 24); // Clock is ON, P0

	uint32_t value;

	value = H3_SPI0->GC;
	value |= GC_SRST;
	H3_SPI0->GC = value;

	value = H3_SPI0->GC;
	value |= GC_MODE_MASTER;
	value |= GC_EN;
	H3_SPI0->GC = value;

	H3_SPI0->IE = 0; // Disable interrupts

#ifndef NDEBUG
	const uint64_t pll_frequency = h3_ccu_get_pll_rate(CCU_PLL_PERIPH0);
	printf("pll_frequency=%ld\n", (long int) pll_frequency);
	assert(CCU_PERIPH0_CLOCK_HZ == pll_frequency);
#endif
}

inline static uint32_t _query_txfifo(void) {
	uint32_t value = H3_SPI0->FS & FS_TX_CNT;
	value >>= FS_TXCNT_BIT_POS;
	return value;
}

inline static uint32_t _query_rxfifo(void) {
	uint32_t value = H3_SPI0->FS & FS_RX_CNT;
	value >>= FS_RXCNT_BIT_POS;
	return value;
}

inline static void _clear_fifos(void) {
	H3_SPI0->FC = (FC_RX_RST | FC_TX_RST);

	int timeout;
													// TODO Do we really need to check?
	for (timeout = 1000; timeout > 0; timeout--) { 	// TODO What if failed?
		if (H3_SPI0->FC == 0)
			break;
	}

	assert(H3_SPI0->FC == 0);

	//TODO Do we need to set the trigger level of RxFIFO/TxFIFO?
}

static void _read_rxfifo(void)
{
	if (s_spi0_status.rxcnt == s_spi0_status.rxlen) {
		return;
	}

	uint32_t rx_count = _query_rxfifo();

	while (rx_count-- > 0) {
		const uint8_t value = H3_SPI0->RX.byte;

		if (s_spi0_status.rxcnt < s_spi0_status.rxlen)
			s_spi0_status.rxbuf[s_spi0_status.rxcnt++] = value;
	}
}

static void _write_txfifo(void) {

	if (s_spi0_status.txcnt == s_spi0_status.txlen) {
		return;
	}

	uint32_t tx_count = SPI_FIFO_SIZE - _query_txfifo();

	while (tx_count-- > 0) {
		H3_SPI0->TX.byte = s_spi0_status.txbuf[s_spi0_status.txcnt++];

		if (s_spi0_status.txcnt == s_spi0_status.txlen) {
			break;
		}
	}

}

static void _interrupt_handler(void) {
	uint32_t intr = H3_SPI0->IS;

	if (intr & IS_RX_FULL) {
		_read_rxfifo();
	}

	if (intr & IS_TX_EMP) {
		_write_txfifo();

		if (s_spi0_status.txcnt == s_spi0_status.txlen) {
			H3_SPI0->IE = IE_TC | IE_RX_FULL;
		}
	}

	if (intr & IS_TC) {
		_read_rxfifo();

		H3_SPI0->IE = 0;
		s_spi0_status.transfer_active = false;
	}

	H3_SPI0->IS = intr;
}

static void spi0_writenb(const char *tx_buffer, uint32_t data_length) {
	assert(tx_buffer != 0);

	H3_SPI0->GC &= ~(GC_TP_EN);	// ignore RXFIFO

	_clear_fifos();

	H3_SPI0->MBC = data_length;
	H3_SPI0->MTC = data_length;
	H3_SPI0->BCC = data_length;

	uint32_t fifo_writes = 0;

	uint32_t tx_count = SPI_FIFO_SIZE - _query_txfifo();

	while ((fifo_writes < data_length) && (tx_count-- > 0)) {
		H3_SPI0->TX.byte = tx_buffer[fifo_writes];
		fifo_writes++;
	}

	H3_SPI0->TC |= TC_XCH;
	H3_SPI0->IE = IE_TX_EMP | IE_TC;

	while ((H3_SPI0->IS & IS_TX_EMP) != IS_TX_EMP)
		;

	H3_SPI0->IS = IS_TX_EMP;

	while (fifo_writes < data_length) {
		uint32_t tx_count = SPI_FIFO_SIZE - _query_txfifo();

		while ((fifo_writes < data_length) && (tx_count-- > 0)) {
			H3_SPI0->TX.byte = tx_buffer[fifo_writes];
			fifo_writes++;
		}

		while ((H3_SPI0->IS & IS_TX_EMP) != IS_TX_EMP)
			;

		H3_SPI0->IS = IS_TX_EMP;

	}

	while ((H3_SPI0->IS & IS_TC) != IS_TC)
		;

	uint32_t value = H3_SPI0->IS;
	H3_SPI0->IS = value;
	H3_SPI0->IE = 0;
}

static void spi0_transfernb(char *tx_buffer, /*@null@*/char *rx_buffer, uint32_t data_length) {
	s_spi0_status.rxbuf = (uint8_t *)rx_buffer;
	s_spi0_status.rxcnt = 0;
	s_spi0_status.txbuf = (uint8_t *)tx_buffer;
	s_spi0_status.txcnt = 0;
	s_spi0_status.txlen = data_length;
	s_spi0_status.rxlen = data_length;

	H3_SPI0->GC |= GC_TP_EN;

	_clear_fifos();

	H3_SPI0->MBC = data_length;
	H3_SPI0->MTC = data_length;
	H3_SPI0->BCC = data_length;

	_write_txfifo();

	H3_SPI0->TC |= TC_XCH;
	H3_SPI0->IE = IE_TX_EMP | IE_RX_FULL |  IE_TC;

	s_spi0_status.transfer_active = true;

	while (s_spi0_status.transfer_active) {
		_interrupt_handler();
	}
}

static void spi0_transfern(char *buffer, uint32_t data_length) {
	assert(buffer != 0);

	spi0_transfernb(buffer, buffer, data_length);
}

static void spi0_setup_clock(uint64_t pll_clock, uint64_t spi_clock) {
// We can use CDR2, which is calculated with the formula: SPI_CLK = CCU_PERIPH0_CLOCK_HZ / (2 * (cdr + 1))
	uint32_t cdr = pll_clock / ( 2 * spi_clock);
	assert(cdr <= (0xFF + 1));

	if (cdr > 0) {
		cdr--;
	}

	uint32_t value = H3_SPI0->CC;
	value &= ~(CC_DRS | (CC_CDR2_MASK << CC_CDR2_SHIFT));
	value |=  (CC_DRS | ((cdr & CC_CDR2_MASK) << CC_CDR2_SHIFT));
	H3_SPI0->CC = value;

#ifndef NDEBUG
	printf("H3_SPI0->CC = %p\n", H3_SPI0->CC);
#endif
}

int spi_init(void) {
	spi0_begin();

	spi0_set_chip_select(); // H3_SPI_CS_NONE
	spi0_set_data_mode(H3_SPI_MODE0);
	spi0_setup_clock(CCU_PERIPH0_CLOCK_HZ, SPI_XFER_SPEED_HZ);

	h3_gpio_fsel(H3_PORT_TO_GPIO(H3_GPIO_PORTC, 3), GPIO_FSEL_OUTPUT);
	h3_gpio_set(H3_PORT_TO_GPIO(H3_GPIO_PORTC, 3));

	return 0;
}

int spi_xfer(unsigned bitlen, const void *dout, void *din, unsigned long flags) {
	const uint32_t len = bitlen / 8;

	if (bitlen % 8) {
		DEBUG_PUTS("non byte-aligned SPI transfer");
		return -3;
	}

	if (flags & SPI_XFER_BEGIN) {
		h3_gpio_clr(H3_PORT_TO_GPIO(H3_GPIO_PORTC, 3));
	}

	if (len != 0) {
		if (din == 0) {
			spi0_writenb((char *)dout, len);
		} else if (dout == 0) {
			spi0_transfern(din, len);
		}
		else {
			spi0_transfernb((char *)dout, (char *)din, len);
		}
	}

	if (flags & SPI_XFER_END) {
		h3_gpio_set(H3_PORT_TO_GPIO(H3_GPIO_PORTC, 3));
	}

	return 0;
}
