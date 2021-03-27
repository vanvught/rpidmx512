/**
 * @file h3_spi.c
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <assert.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif

#include "h3.h"
#include "h3_gpio.h"
#include "h3_spi.h"
#include "h3_ccu.h"
#include "h3_dma.h"

#include "h3_board.h"

#include "h3_spi_internal.h"

#define ALT_FUNCTION_CS		(EXT_SPI_NUMBER == 0 ? (H3_PC3_SELECT_SPI0_CS) : (H3_PA13_SELECT_SPI1_CS))
#define ALT_FUNCTION_CLK	(EXT_SPI_NUMBER == 0 ? (H3_PC2_SELECT_SPI0_CLK) : (H3_PA14_SELECT_SPI1_CLK))
#define ALT_FUNCTION_MOSI	(EXT_SPI_NUMBER == 0 ? (H3_PC1_SELECT_SPI0_MISO) : (H3_PA15_SELECT_SPI1_MOSI))
#define ALT_FUNCTION_MISO	(EXT_SPI_NUMBER == 0 ? (H3_PC0_SELECT_SPI0_MOSI) : (H3_PA16_SELECT_SPI1_MISO))

static bool s_ws28xx_mode = false;
static uint32_t s_current_speed_hz = 0; // This forces an update

struct spi_status {
	bool		transfer_active;
	uint8_t		*rxbuf;
	uint32_t	rxcnt;
	uint8_t		*txbuf;
	uint32_t	txcnt;
	uint32_t	txlen;
	uint32_t	rxlen;
};

static struct spi_status s_spi_status;

void h3_spi_set_ws28xx_mode(bool off_on) {
	s_ws28xx_mode = off_on;
}

bool h3_spi_get_ws28xx_mode(void) {
	return s_ws28xx_mode;
}

static uint32_t _clock_test_cdr1(uint32_t pll_clock, uint32_t spi_clock, uint32_t *ccr) {
	uint32_t cur, best = 0;
	uint32_t i, best_div = 0;

	const uint32_t max = CC_CDR1_MASK >> CC_CDR1_SHIFT;

	for (i = 0; i < max; i++) {
		cur = pll_clock / (1U << i);

		const uint32_t d1 = (spi_clock > cur) ? (spi_clock - cur) : (cur - spi_clock);
		const uint32_t d2 = (spi_clock > best) ? (spi_clock - best) : (best - spi_clock);

		if (d1 < d2) {
			best = cur;
			best_div = i;
		}
	}

	*ccr = (best_div << CC_CDR1_SHIFT);

	return best;
}

static uint32_t _clock_test_cdr2(uint32_t pll_clock, uint32_t spi_clock, uint32_t *ccr) {
	uint32_t cur, best = 0;
	uint32_t i, best_div = 0;

	const uint32_t max = ((CC_CDR2_MASK) >> CC_CDR2_SHIFT);

	for (i = 0; i < max; i++) {
		cur = pll_clock / (2 * i + 1);

		const uint32_t d1 = (spi_clock > cur) ? (spi_clock - cur) : (cur - spi_clock);
		const uint32_t d2 = (spi_clock > best) ? (spi_clock - best) : (best - spi_clock);

		if (d1 < d2) {
			best = cur;
			best_div = i;
		}
	}

	if (best == 0) {
		best_div = max;
		best = pll_clock / (2 * max + 1);
	}

	*ccr = CC_DRS | (best_div << CC_CDR2_SHIFT);

	return best;
}

void _setup_clock(uint32_t pll_clock, uint32_t spi_clock) {
	uint32_t best_ccr1, best_ccr2;
	uint32_t ccr, ccr1, ccr2;

#ifndef NDEBUG
	printf("pll_clock=%ld, spi_clock=%ld\n", (long int)pll_clock, (long int) spi_clock);
#endif

	best_ccr1 = _clock_test_cdr1(pll_clock, spi_clock, &ccr1);
	best_ccr2 = _clock_test_cdr2(pll_clock, spi_clock, &ccr2);
#ifndef NDEBUG
	printf("best_ccr1=%ld, best_ccr2=%ld\n", (long int)best_ccr1, (long int) best_ccr2);
#endif

	if (best_ccr1 == spi_clock) {
		ccr = ccr1;
	} else if (best_ccr2 == spi_clock) {
		ccr = ccr2;
	} else {
		const uint32_t d1 = (spi_clock > best_ccr1) ? (spi_clock - best_ccr1) : (best_ccr1 - spi_clock);
		const uint32_t d2 = (spi_clock > best_ccr2) ? (spi_clock - best_ccr2) : (best_ccr2 - spi_clock);
#ifndef NDEBUG
		printf("d1=%ld, d2=%ld\n", (long int)d1, (long int) d2);
#endif
		if (d1 < d2) {
			ccr = ccr1;
		} else {
			ccr = ccr2;
		}
	}

	EXT_SPI->CC = ccr;

#ifndef NDEBUG
	printf("EXT_SPI->CC = %p\n", EXT_SPI->CC);
#endif
}

inline static uint32_t _query_txfifo(void) {
	uint32_t value = EXT_SPI->FS & FS_TX_CNT;
	value >>= FS_TXCNT_BIT_POS;
	return value;
}

inline static uint32_t _query_rxfifo(void) {
	uint32_t value = EXT_SPI->FS & FS_RX_CNT;
	value >>= FS_RXCNT_BIT_POS;
	return value;
}

inline static void _clear_fifos(void) {
	EXT_SPI->FC = (FC_RX_RST | FC_TX_RST);

	int timeout;
													// TODO Do we really need to check?
	for (timeout = 1000; timeout > 0; timeout--) { 	// TODO What if failed?
		if (EXT_SPI->FC == 0)
			break;
	}

	//TODO Do we need to set the trigger level of RxFIFO/TxFIFO?
}

static void _read_rxfifo(void)
{
	if (s_spi_status.rxcnt == s_spi_status.rxlen) {
		return;
	}

	uint32_t rx_count = _query_rxfifo();

	while (rx_count-- > 0) {
		const uint8_t value = EXT_SPI->RX.byte;

		if (s_spi_status.rxcnt < s_spi_status.rxlen)
			s_spi_status.rxbuf[s_spi_status.rxcnt++] = value;
	}
}

static void _write_txfifo(void) {

	if (s_spi_status.txcnt == s_spi_status.txlen) {
		return;
	}

	uint32_t tx_count = SPI_FIFO_SIZE - _query_txfifo();

	while (tx_count-- > 0) {
		EXT_SPI->TX.byte = s_spi_status.txbuf[s_spi_status.txcnt++];

		if (s_spi_status.txcnt == s_spi_status.txlen) {
			break;
		}
	}

}

static void _interrupt_handler(void) {
	uint32_t intr = EXT_SPI->IS;

	if (intr & IS_RX_FULL) {
		_read_rxfifo();
	}

	if (intr & IS_TX_EMP) {
		_write_txfifo();

		if (s_spi_status.txcnt == s_spi_status.txlen) {
			EXT_SPI->IE = IE_TC | IE_RX_FULL;
		}
	}

	if (intr & IS_TC) {
		_read_rxfifo();

		EXT_SPI->IE = 0;
		s_spi_status.transfer_active = false;
	}

	EXT_SPI->IS = intr;
}

void __attribute__((cold)) h3_spi_begin(void) {
	h3_gpio_fsel(EXT_SPI_CS, ALT_FUNCTION_CS);
	h3_gpio_fsel(EXT_SPI_CLK, ALT_FUNCTION_CLK);
	h3_gpio_fsel(EXT_SPI_MOSI, ALT_FUNCTION_MOSI);
	h3_gpio_fsel(EXT_SPI_MISO, ALT_FUNCTION_MISO);

#if (EXT_SPI_NUMBER == 0)
	H3_CCU->BUS_SOFT_RESET0 |= CCU_BUS_SOFT_RESET0_SPI0;
	udelay(1000); // 1ms
	H3_CCU->BUS_CLK_GATING0 &= ~CCU_BUS_CLK_GATING0_SPI0;
	udelay(1000); // 1ms
	H3_CCU->BUS_CLK_GATING0 |= CCU_BUS_CLK_GATING0_SPI0;
	H3_CCU->SPI0_CLK = (1U << 31) | (0x01 << 24); // Clock is ON, P0
#elif (EXT_SPI_NUMBER == 1)
	H3_CCU->BUS_SOFT_RESET0 |= CCU_BUS_SOFT_RESET0_SPI1;
	udelay(1000); // 1ms
	H3_CCU->BUS_CLK_GATING0 &= ~CCU_BUS_CLK_GATING0_SPI1;
	udelay(1000); // 1ms
	H3_CCU->BUS_CLK_GATING0 |= CCU_BUS_CLK_GATING0_SPI1;
	H3_CCU->SPI1_CLK = (1U << 31) | (0x01 << 24); // Clock is ON, P0
#else
 #error Unsupported SPI device configured
#endif

	uint32_t value;

	value = EXT_SPI->GC;
	value |= GC_SRST;
	EXT_SPI->GC = value;

	value = EXT_SPI->GC;
	value |= GC_MODE_MASTER;
	value |= GC_EN;
	EXT_SPI->GC = value;

	EXT_SPI->IE = 0; // Disable interrupts

#ifndef NDEBUG
	const uint64_t pll_frequency = h3_ccu_get_pll_rate(CCU_PLL_PERIPH0);
	printf("pll_frequency=%ld\n", (long int) pll_frequency);
	assert(CCU_PERIPH0_CLOCK_HZ == pll_frequency);
#endif

	// Defaults
	h3_spi_setBitOrder(H3_SPI_BIT_ORDER_MSBFIRST);
	h3_spi_setDataMode(H3_SPI_MODE0);
	h3_spi_chipSelect(H3_SPI_CS0);
	h3_spi_setChipSelectPolarity(H3_SPI_CS0, LOW);
	h3_spi_set_speed_hz((uint64_t) 1000000); // Default 1MHz SPI Clock

#ifndef NDEBUG
	printf("%s SPI%c\n", __FUNCTION__, '0' + EXT_SPI_NUMBER);
	printf("H3_CCU->BUS_CLK_GATING0=%p\n", H3_CCU->BUS_CLK_GATING0);
	printf("H3_CCU->BUS_SOFT_RESET0=%p\n", H3_CCU->BUS_SOFT_RESET0);
	printf("EXT_SPI=%p\n", EXT_SPI);
	printf("EXT_SPI->GC=%p\n", EXT_SPI->GC);
	printf("EXT_SPI->TC=%p\n", EXT_SPI->TC);
	printf("EXT_SPI->CC=%p\n", EXT_SPI->CC);
#endif
}

void __attribute__((cold)) h3_spi_end(void) {
	uint32_t value;

	value = EXT_SPI->GC;
	value |= GC_SRST;
	EXT_SPI->GC = value;

	value = EXT_SPI->GC;
	value &= (uint32_t)~GC_EN;
	EXT_SPI->GC = value;

#if (EXT_SPI_NUMBER == 0)
	H3_CCU->BUS_CLK_GATING0 &= ~CCU_BUS_CLK_GATING0_SPI0;
	H3_CCU->BUS_SOFT_RESET0 &= ~CCU_BUS_SOFT_RESET0_SPI0;
	H3_CCU->SPI0_CLK &= ~(1 << 31); // Clock is OFF
#elif (EXT_SPI_NUMBER == 1)
	H3_CCU->BUS_CLK_GATING0 &= ~CCU_BUS_CLK_GATING0_SPI1;
	H3_CCU->BUS_SOFT_RESET0 &= ~CCU_BUS_SOFT_RESET0_SPI1;
	H3_CCU->SPI1_CLK &= ~(1U << 31); // Clock is OFF
#endif

	h3_gpio_fsel(EXT_SPI_CS, GPIO_FSEL_DISABLE);
	h3_gpio_fsel(EXT_SPI_CLK, GPIO_FSEL_DISABLE);
	h3_gpio_fsel(EXT_SPI_MOSI, GPIO_FSEL_DISABLE);
	h3_gpio_fsel(EXT_SPI_MISO, GPIO_FSEL_DISABLE);
}

void h3_spi_set_speed_hz(uint32_t speed_hz) {
	assert(speed_hz != 0);

	if (__builtin_expect((s_current_speed_hz != speed_hz), 0)) {
		s_current_speed_hz = speed_hz;
		_setup_clock(CCU_PERIPH0_CLOCK_HZ, speed_hz);
	}
}

void h3_spi_setBitOrder(h3_spi_bit_order_t bit_order) {
	uint32_t value = EXT_SPI->TC;

	if (bit_order == H3_SPI_BIT_ORDER_LSBFIRST) {
		value |= TC_FBS;
	} else {
		value &= (uint32_t)~TC_FBS;
	}

	EXT_SPI->TC = value;
}

void h3_spi_setDataMode(uint8_t mode) {
	uint32_t value = EXT_SPI->TC;
	value &= (uint32_t)~TC_CPHA;
	value &= (uint32_t)~TC_CPOL;
	value |= (mode & 0x3);
	EXT_SPI->TC = value;
}

void h3_spi_chipSelect(uint8_t chip_select) {
	uint32_t value = EXT_SPI->TC;

	if (chip_select < H3_SPI_CS_NONE) {
		value &= (uint32_t)~TC_SS_OWNER;
		value &= (uint32_t)~TC_SS_MASK;
		value |= (uint32_t)(chip_select << TC_SS_MASK_SHIFT);
	} else {
		value |= TC_SS_OWNER;	// Software controlled
	}

	EXT_SPI->TC = value;
}

void h3_spi_setChipSelectPolarity(__attribute__((unused)) uint8_t chip_select, uint8_t polarity) {
	uint32_t value = EXT_SPI->TC;

	if (polarity == HIGH) {
		value &= (uint32_t)~TC_SPOL;
	} else {
		value |= TC_SPOL;
	}

	EXT_SPI->TC = value;
}

void h3_spi_transfernb(char *tx_buffer, /*@null@*/char *rx_buffer, uint32_t data_length) {
	s_spi_status.rxbuf = (uint8_t *)rx_buffer;
	s_spi_status.rxcnt = 0;
	s_spi_status.txbuf = (uint8_t *)tx_buffer;
	s_spi_status.txcnt = 0;
	s_spi_status.txlen = data_length;
	s_spi_status.rxlen = data_length;

	_clear_fifos();

	EXT_SPI->MBC = data_length;
	EXT_SPI->MTC = data_length;
	EXT_SPI->BCC = data_length;

	_write_txfifo();

	EXT_SPI->TC |= TC_XCH;
	EXT_SPI->IE = IE_TX_EMP | IE_RX_FULL |  IE_TC;

	s_spi_status.transfer_active = true;

	while (s_spi_status.transfer_active) {
		_interrupt_handler();
	}
}

void h3_spi_transfern(char *buffer, uint32_t data_length) {
	assert(buffer != 0);

	h3_spi_transfernb(buffer, buffer, data_length);
}

void h3_spi_writenb(const char *tx_buffer, uint32_t data_length) {
	assert(tx_buffer != 0);

	EXT_SPI->GC &= (uint32_t)~(GC_TP_EN);	// ignore RXFIFO

	_clear_fifos();

	if (s_ws28xx_mode) {
		EXT_SPI->MBC = data_length + 1;
		EXT_SPI->MTC = data_length + 1;
		EXT_SPI->BCC = data_length + 1;

		EXT_SPI->TX.byte = (uint8_t) 0x00;
	} else {
		EXT_SPI->MBC = data_length;
		EXT_SPI->MTC = data_length;
		EXT_SPI->BCC = data_length;
	}

	uint32_t fifo_writes = 0;

	uint32_t tx_count = SPI_FIFO_SIZE - _query_txfifo();

	while ((fifo_writes < data_length) && (tx_count-- > 0)) {
		EXT_SPI->TX.byte = tx_buffer[fifo_writes];
		fifo_writes++;
	}

	EXT_SPI->TC |= TC_XCH;
	EXT_SPI->IE = IE_TX_EMP | IE_TC;

	while ((EXT_SPI->IS & IS_TX_EMP) != IS_TX_EMP)
		;

	EXT_SPI->IS = IS_TX_EMP;

	while (fifo_writes < data_length) {
		uint32_t tx_count = SPI_FIFO_SIZE - _query_txfifo();

		while ((fifo_writes < data_length) && (tx_count-- > 0)) {
			EXT_SPI->TX.byte = tx_buffer[fifo_writes];
			fifo_writes++;
		}

		while ((EXT_SPI->IS & IS_TX_EMP) != IS_TX_EMP)
			;

		EXT_SPI->IS = IS_TX_EMP;

	}

	while ((EXT_SPI->IS & IS_TC) != IS_TC)
		;

	uint32_t value = EXT_SPI->IS;
	EXT_SPI->IS = value;
	EXT_SPI->IE = 0;
}

void h3_spi_write(uint16_t data) {
	EXT_SPI->GC &= (uint32_t)~(GC_TP_EN);	// ignore RXFIFO

	_clear_fifos();

	EXT_SPI->MBC = 2;
	EXT_SPI->MTC = 2;
	EXT_SPI->BCC = 2;

	EXT_SPI->TX.byte = (uint8_t) (data >> 8);
	EXT_SPI->TX.byte = (uint8_t) data & 0xFF;

	EXT_SPI->TC |= TC_XCH;
	EXT_SPI->IE = IE_TX_EMP | IE_TC;

	while ((EXT_SPI->IS & IS_TX_EMP) != IS_TX_EMP)
		;

	while ((EXT_SPI->IS & IS_TC) != IS_TC)
		;

	uint32_t value = EXT_SPI->IS;
	EXT_SPI->IS = value;
	EXT_SPI->IE = 0;
}

uint8_t h3_spi_transfer(uint8_t data) {
	uint8_t ret;

	_clear_fifos();

	EXT_SPI->MBC = 1;
	EXT_SPI->MTC = 1;
	EXT_SPI->BCC = 1;

	EXT_SPI->TX.byte = data;

	EXT_SPI->TC |= TC_XCH;
	EXT_SPI->IE = IE_TX_EMP | IE_TC;

	while ((EXT_SPI->IS & IS_TX_EMP) != IS_TX_EMP)
		;

	while ((EXT_SPI->IS & IS_TC) != IS_TC)
		;

	ret = EXT_SPI->RX.byte;

	uint32_t value = EXT_SPI->IS;
	EXT_SPI->IS = value;
	EXT_SPI->IE = 0;

	return ret;
}

/*
 * DMA support
 */

#define SPI_DMA_COHERENT_REGION_SIZE	(MEGABYTE/8)
#define SPI_DMA_COHERENT_REGION			(H3_MEM_COHERENT_REGION + MEGABYTE/2 + MEGABYTE/4)
#define SPI_DMA_TX_BUFFER_SIZE			(SPI_DMA_COHERENT_REGION_SIZE - sizeof(struct sunxi_dma_lli))

struct dma_spi {
	struct sunxi_dma_lli lli;
	uint8_t tx_buffer[SPI_DMA_TX_BUFFER_SIZE] __attribute__ ((aligned (4)));
};

static volatile struct dma_spi *p_dma_tx = (struct dma_spi *) SPI_DMA_COHERENT_REGION;
static bool is_running = false;

bool h3_spi_dma_tx_is_active(void) {
	if (!is_running) {
		return false;
	}

	const uint32_t intr = EXT_SPI->IS;

	if (intr & IS_TC) {
		EXT_SPI->IS = intr;
		is_running = false;
		return false;
	}

	return true;
}

const uint8_t *h3_spi_dma_tx_prepare(uint32_t *size) {
	assert(size != 0);

	H3_CCU->BUS_SOFT_RESET0 |= CCU_BUS_SOFT_RESET0_DMA;
	H3_CCU->BUS_CLK_GATING0 |= CCU_BUS_CLK_GATING0_DMA;

	p_dma_tx->lli.cfg = DMA_CHAN_CFG_SRC_LINEAR_MODE | DMA_CHAN_CFG_SRC_DRQ(DRQSRC_SDRAM) | DMA_CHAN_CFG_SRC_WIDTH(0) | DMA_CHAN_CFG_SRC_BURST(0)
						  | DMA_CHAN_CFG_DST_IO_MODE  | DMA_CHAN_CFG_DST_DRQ(DRQDST_SPIO1) | DMA_CHAN_CFG_DST_WIDTH(0) | DMA_CHAN_CFG_DST_BURST(0);
	p_dma_tx->lli.src = (uint32_t) &p_dma_tx->tx_buffer;
	p_dma_tx->lli.dst = (uint32_t) &EXT_SPI->TX.byte;
	p_dma_tx->lli.para = DMA_NORMAL_WAIT;
	p_dma_tx->lli.p_lli_next = DMA_LLI_LAST_ITEM;
#ifndef NDEBUG
	h3_dma_dump_lli((const struct sunxi_dma_lli *)&p_dma_tx->lli);
#endif
	*size = (uint32_t) sizeof(p_dma_tx->tx_buffer);

	return (const uint8_t *)&p_dma_tx->tx_buffer;
}

void h3_spi_dma_tx_start(const uint8_t *tx_buffer, uint32_t data_length) {
	assert(!is_running);
	assert(tx_buffer != 0);	// TODO Not valid when SRAM is used
	assert(data_length <= (uint32_t) sizeof(p_dma_tx->tx_buffer) - ((uint32_t) tx_buffer) - (uint32_t) &p_dma_tx->tx_buffer);
	assert(((uint32_t) tx_buffer & H3_MEM_COHERENT_REGION) == H3_MEM_COHERENT_REGION);

	p_dma_tx->lli.src = (uint32_t) tx_buffer;
	p_dma_tx->lli.len = data_length;

	EXT_SPI->GC &= (uint32_t)~(1 << 7);
	EXT_SPI->FC = ((1U << 31) | (1 << 15) );

	EXT_SPI->IS = IE_TC;
	EXT_SPI->IE = IE_TC;

	if (s_ws28xx_mode) {
		EXT_SPI->MBC = data_length + 1;
		EXT_SPI->MTC = data_length + 1;
		EXT_SPI->BCC = data_length + 1;

		EXT_SPI->TX.byte = 0x00;
	} else {
		EXT_SPI->MBC = data_length;
		EXT_SPI->MTC = data_length;
		EXT_SPI->BCC = data_length;
	}

	EXT_SPI->TC |= (1U << 31);
	EXT_SPI->FC |= (1 << 24);

	H3_DMA_CHL2->DESC_ADDR = (uint32_t)&p_dma_tx->lli;
	H3_DMA_CHL2->EN = DMA_CHAN_ENABLE_START;

	is_running = true;
}
