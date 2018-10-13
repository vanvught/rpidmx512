/**
 * @file dmx_multi.c
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

#ifdef NDEBUG
//#undef NDEBUG
#endif
//#define LOGIC_ANALYZER

#include <stdint.h>
#include <assert.h>

#include "dmx_multi_internal.h"

#include "dmx.h"
#include "rdm.h"
#include "rdm_e120.h"

#include "gpio.h"

#include "util.h"

#include "arm/arm.h"
#include "arm/synchronize.h"
#include "arm/gic.h"

#include "h3_dma.h"
#include "h3_ccu.h"
#include "h3_gpio.h"
#include "h3_timer.h"
#include "h3_hs_timer.h"

#include "irq_timer.h"

#include "uart.h"

extern int console_error(const char *);

#define DMX_DATA_OUT_INDEX	(1 << 2)

typedef enum {
	IDLE = 0,
	PRE_BREAK,
	BREAK,
	MAB,
	DMXDATA,
	RDMDATA,
	CHECKSUMH,
	CHECKSUML,
	RDMDISCFE,
	RDMDISCEUID,
	RDMDISCECS,
	DMXINTER
} _tx_rx_state;

typedef enum {
	UART_STATE_IDLE = 0, UART_STATE_TX, UART_STATE_RX
} _uart_state;

struct _dmx_multi_data {
	uint8_t data[DMX_DATA_BUFFER_SIZE]; // multiple of uint32_t
	uint32_t length;
};

struct coherent_region {
	struct sunxi_dma_lli lli[DMX_MAX_OUT];
	struct _dmx_multi_data dmx_data[DMX_MAX_OUT][DMX_DATA_OUT_INDEX] ALIGNED;
};

struct _rdm_multi_data {
	uint8_t data[RDM_DATA_BUFFER_SIZE];
	uint16_t checksum;	// This must be uint16_t
	uint16_t _pad;
	uint32_t index;
	uint32_t disc_index;
};

static struct coherent_region *p_coherent_region = (struct coherent_region *)(H3_MEM_COHERENT_REGION + MEGABYTE/2);

static volatile uint32_t dmx_data_write_index[DMX_MAX_OUT] ALIGNED = { 0, };
static volatile uint32_t dmx_data_read_index[DMX_MAX_OUT] ALIGNED = { 0, };

static uint32_t dmx_output_break_time = DMX_TRANSMIT_BREAK_TIME_MIN;
static uint32_t dmx_output_mab_time = DMX_TRANSMIT_MAB_TIME_MIN;
static uint32_t dmx_output_period = DMX_TRANSMIT_PERIOD_DEFAULT;

static uint32_t dmx_output_break_time_intv = DMX_TRANSMIT_BREAK_TIME_MIN * 12;
static uint32_t dmx_output_mab_time_intv = DMX_TRANSMIT_MAB_TIME_MIN * 12;
static uint32_t dmx_output_period_intv = DMX_TRANSMIT_PERIOD_DEFAULT * 12;

static volatile _tx_rx_state dmx_send_state = IDLE;

static _dmx_port_direction dmx_port_direction[DMX_MAX_OUT] ALIGNED;
static uint8_t dmx_data_direction_gpio_pin[DMX_MAX_OUT] ALIGNED;

static struct _rdm_multi_data rdm_data[DMX_MAX_OUT][RDM_DATA_BUFFER_INDEX_ENTRIES] ALIGNED;
static struct _rdm_multi_data *rdm_data_current[DMX_MAX_OUT] ALIGNED;
static volatile _tx_rx_state rdm_receive_state[DMX_MAX_OUT] ALIGNED = { IDLE, };
static volatile uint32_t rdm_data_write_index[DMX_MAX_OUT] ALIGNED = { 0, };
static volatile uint32_t rdm_data_read_index[DMX_MAX_OUT] ALIGNED = { 0, };

static volatile _uart_state uart_state[DMX_MAX_OUT] ALIGNED;
static volatile uint32_t uarts_sending = 0;

static char CONSOLE_ERROR[] ALIGNED = "DMXDATA %\n";
#define CONSOLE_ERROR_LENGTH (sizeof(CONSOLE_ERROR) / sizeof(CONSOLE_ERROR[0]))

static void dmx_multi_clear_data(uint8_t uart) {
	uint32_t i, j;

	for (j = 0; j < DMX_DATA_OUT_INDEX; j++) {
		struct _dmx_multi_data *p = (struct _dmx_multi_data *) &p_coherent_region->dmx_data[uart][j];

		for (i = 0; i < DMX_DATA_BUFFER_SIZE / 4; i++) {
			p->data[i] = 0;
		}

		p->length = 513; // Including START Code
	}
}

static void irq_timer0_dmx_multi_sender(uint32_t clo) {
#ifdef LOGIC_ANALYZER
	h3_gpio_set(6);
#endif

	dmb();
	switch (dmx_send_state) {
	case IDLE:
	case DMXINTER:
		dmb();
		H3_TIMER->TMR0_INTV = dmx_output_break_time_intv;
		H3_TIMER->TMR0_CTRL |= 0x3;
		isb();

		dmb();
		if (uart_state[1] == UART_STATE_TX) {
			H3_UART1->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
		}

		if (uart_state[2] == UART_STATE_TX) {
			H3_UART2->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
		}
#if defined (ORANGE_PI_ONE)
		if (uart_state[3] == UART_STATE_TX) {
			H3_UART3->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
		}
 #ifndef DO_NOT_USE_UART0
		if (uart_state[0] == UART_STATE_TX) {
			H3_UART0->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
		}
 #endif
#endif
		isb();

		if (dmx_data_write_index[1] != dmx_data_read_index[1]) {
			dmx_data_read_index[1] = (dmx_data_read_index[1] + 1) & (DMX_DATA_OUT_INDEX - 1);

			p_coherent_region->lli[1].src = (uint32_t) &p_coherent_region->dmx_data[1][dmx_data_read_index[1]].data[0];
			p_coherent_region->lli[1].len = p_coherent_region->dmx_data[1][dmx_data_read_index[1]].length;
		}

		if (dmx_data_write_index[2] != dmx_data_read_index[2]) {
			dmx_data_read_index[2] = (dmx_data_read_index[2] + 1) & (DMX_DATA_OUT_INDEX - 1);

			p_coherent_region->lli[2].src = (uint32_t) &p_coherent_region->dmx_data[2][dmx_data_read_index[2]].data[0];
			p_coherent_region->lli[2].len = p_coherent_region->dmx_data[2][dmx_data_read_index[2]].length;
		}
#if defined (ORANGE_PI_ONE)
		if (dmx_data_write_index[3] != dmx_data_read_index[3]) {
			dmx_data_read_index[3] = (dmx_data_read_index[3] + 1) & (DMX_DATA_OUT_INDEX - 1);

			p_coherent_region->lli[3].src = (uint32_t) &p_coherent_region->dmx_data[3][dmx_data_read_index[3]].data[0];
			p_coherent_region->lli[3].len = p_coherent_region->dmx_data[3][dmx_data_read_index[3]].length;
		}
 #ifndef DO_NOT_USE_UART0
		if (dmx_data_write_index[0] != dmx_data_read_index[0]) {
			dmx_data_read_index[0] = (dmx_data_read_index[0] + 1) & (DMX_DATA_OUT_INDEX - 1);

			p_coherent_region->lli[0].src = (uint32_t) &p_coherent_region->dmx_data[0][dmx_data_read_index[0]].data[0];
			p_coherent_region->lli[0].len = p_coherent_region->dmx_data[0][dmx_data_read_index[0]].length;
		}
 #endif
#endif

		dmb();
		dmx_send_state = BREAK;
		break;
	case BREAK:
		dmb();
		H3_TIMER->TMR0_INTV = dmx_output_mab_time_intv;
		H3_TIMER->TMR0_CTRL |= 0x3;
		isb();

		dmb();
		if (uart_state[1] == UART_STATE_TX) {
			H3_UART1->LCR = UART_LCR_8_N_2;
		}

		if (uart_state[2] == UART_STATE_TX) {
			H3_UART2->LCR = UART_LCR_8_N_2;
		}
#if defined (ORANGE_PI_ONE)
		if (uart_state[3] == UART_STATE_TX) {
			H3_UART3->LCR = UART_LCR_8_N_2;
		}
 #ifndef DO_NOT_USE_UART0
		if (uart_state[0] == UART_STATE_TX) {
			H3_UART0->LCR = UART_LCR_8_N_2;
		}
 #endif
#endif
		isb();

		dmb();
		dmx_send_state = MAB;
		break;
	case MAB:
		dmb();
		H3_TIMER->TMR0_INTV = dmx_output_period_intv;
		H3_TIMER->TMR0_CTRL |= 0x3;
		isb();

		dmb();
		if (uart_state[1] == UART_STATE_TX) {
			H3_DMA_CHL1->DESC_ADDR = (uint32_t) &p_coherent_region->lli[1];
			H3_DMA_CHL1->EN = DMA_CHAN_ENABLE_START;
			uarts_sending |= (1 << 1);
		}

		if (uart_state[2] == UART_STATE_TX) {
			H3_DMA_CHL2->DESC_ADDR = (uint32_t) &p_coherent_region->lli[2];
			H3_DMA_CHL2->EN = DMA_CHAN_ENABLE_START;
			uarts_sending |= (1 << 2);
		}
#if defined (ORANGE_PI_ONE)
		if (uart_state[3] == UART_STATE_TX) {
			H3_DMA_CHL3->DESC_ADDR = (uint32_t) &p_coherent_region->lli[3];
			H3_DMA_CHL3->EN = DMA_CHAN_ENABLE_START;
			uarts_sending |= (1 << 3);
		}
 #ifndef DO_NOT_USE_UART0
		if (uart_state[0] == UART_STATE_TX) {
			H3_DMA_CHL0->DESC_ADDR = (uint32_t) &p_coherent_region->lli[0];
			H3_DMA_CHL0->EN = DMA_CHAN_ENABLE_START;
			uarts_sending |= (1 << 0);
		}
 #endif
#endif
		isb();

		if (uarts_sending == 0) {
			dmb();
			dmx_send_state = DMXINTER;
		} else {
			dmb();
			dmx_send_state = DMXDATA;
		}
		break;
	case DMXDATA:
		assert(0);
#ifdef LOGIC_ANALYZER
		h3_gpio_set(20);
#endif		
		CONSOLE_ERROR[CONSOLE_ERROR_LENGTH - 3] = '0' + (char) uarts_sending;
		console_error(CONSOLE_ERROR);
#ifdef LOGIC_ANALYZER
		h3_gpio_clr(20);
#endif		
		// Recover from this internal error.
		uarts_sending = 0;
		dmb();
		dmx_send_state = DMXINTER;
		isb();
		H3_TIMER->TMR0_INTV = 12;
		H3_TIMER->TMR0_CTRL |= 0x3;
		dmb();
		break;
	default:
		assert(0);
		break;
	}

#ifdef LOGIC_ANALYZER
	h3_gpio_clr(6);
#endif
}

static void fiq_rdm_in_handler(uint8_t uart, const H3_UART_TypeDef *u) {
	uint16_t index;

	isb();

	if (u->LSR & UART_LSR_BI) {
		rdm_receive_state[uart] = PRE_BREAK;
	} else {
		const uint8_t data = u->O00.RBR;

		switch (rdm_receive_state[uart]) {
		case IDLE:
			if (data == 0xFE) {
				rdm_data_current[uart]->data[0] = 0xFE;
				rdm_data_current[uart]->index = 1;

				rdm_receive_state[uart] = RDMDISCFE;
			}
			break;
		case PRE_BREAK:
			rdm_receive_state[uart] = BREAK;
			break;
		case BREAK:
			switch (data) {
			case E120_SC_RDM:
				rdm_data_current[uart]->data[0] = E120_SC_RDM;
				rdm_data_current[uart]->checksum = E120_SC_RDM;
				rdm_data_current[uart]->index = 1;

				rdm_receive_state[uart] = RDMDATA;
				break;
			default:
				rdm_receive_state[uart] = IDLE;
				break;
			}
			break;
		case RDMDATA:
			if (rdm_data_current[uart]->index > RDM_DATA_BUFFER_SIZE) {
				rdm_receive_state[uart] = IDLE;
			} else {
				index = rdm_data_current[uart]->index;
				rdm_data_current[uart]->data[index] = data;
				rdm_data_current[uart]->index++;

				rdm_data_current[uart]->checksum += data;

				const struct _rdm_command *p = (struct _rdm_command *)(&rdm_data_current[uart]->data[0]);

				if (rdm_data_current[uart]->index == p->message_length) {
					rdm_receive_state[uart] = CHECKSUMH;
				}
			}
			break;
		case CHECKSUMH:
			index = rdm_data_current[uart]->index;
			rdm_data_current[uart]->data[index] = data;
			rdm_data_current[uart]->index++;

			rdm_data_current[uart]->checksum -= data << 8;

			rdm_receive_state[uart] = CHECKSUML;
			break;
		case CHECKSUML:
			index = rdm_data_current[uart]->index;
			rdm_data_current[uart]->data[index] = data;
			rdm_data_current[uart]->index++;

			rdm_data_current[uart]->checksum -= data;

			const struct _rdm_command *p = (struct _rdm_command *)(&rdm_data[uart][rdm_data_write_index[uart]].data[0]);

			if ((rdm_data[uart][rdm_data_write_index[uart]].checksum == 0) && (p->sub_start_code == E120_SC_SUB_MESSAGE)) {
				rdm_data_write_index[uart] = (rdm_data_write_index[uart] + 1) & RDM_DATA_BUFFER_INDEX_MASK;
				dmb();
				rdm_data_current[uart] = &rdm_data[uart][rdm_data_write_index[uart]];
			}

			rdm_receive_state[uart] = IDLE;
			break;
		case RDMDISCFE:
			index = rdm_data_current[uart]->index;
			rdm_data_current[uart]->data[index] = data;
			rdm_data_current[uart]->index++;

			if ((data == 0xAA) || (rdm_data_current[uart]->index == 9)) {
				rdm_data_current[uart]->disc_index = 0;

				rdm_receive_state[uart] = RDMDISCEUID;
			}
			break;
		case RDMDISCEUID:
			index = rdm_data_current[uart]->index;
			rdm_data_current[uart]->data[index] = data;
			rdm_data_current[uart]->index++;

			rdm_data_current[uart]->disc_index++;

			if (rdm_data_current[uart]->disc_index == 2 * RDM_UID_SIZE) {
				rdm_data_current[uart]->disc_index = 0;

				rdm_receive_state[uart] = RDMDISCECS;
			}
			break;
		case RDMDISCECS:
			index = rdm_data_current[uart]->index;
			rdm_data_current[uart]->data[index] = data;
			rdm_data_current[uart]->index++;

			rdm_data_current[uart]->disc_index++;

			if (rdm_data_current[uart]->disc_index == 4) {
				rdm_data_write_index[uart] = (rdm_data_write_index[uart] + 1) & RDM_DATA_BUFFER_INDEX_MASK;
				dmb();
				rdm_data_current[uart] = &rdm_data[uart][rdm_data_write_index[uart]];

				rdm_receive_state[uart] = IDLE;
			}

			break;
		default:
			rdm_receive_state[uart] = IDLE;
			break;
		}
	}

	dmb();
}

static void __attribute__((interrupt("FIQ"))) fiq_dmx_multi(void) {
	dmb();

#ifdef LOGIC_ANALYZER
	h3_gpio_set(3);
#endif

	// UART1
	if (H3_DMA->IRQ_PEND0 & (DMA_IRQ_PEND0_DMA1_HALF_IRQ_EN | DMA_IRQ_PEND0_DMA1_PKG_IRQ_EN)) {
		uarts_sending &= ~(1 << 1);
	}
	// UART2
	if (H3_DMA->IRQ_PEND0 & (DMA_IRQ_PEND0_DMA2_HALF_IRQ_EN | DMA_IRQ_PEND0_DMA2_PKG_IRQ_EN)) {
		uarts_sending &= ~(1 << 2);
	}
#if defined (ORANGE_PI_ONE)
	// UART3
	if (H3_DMA->IRQ_PEND0 & (DMA_IRQ_PEND0_DMA3_HALF_IRQ_EN | DMA_IRQ_PEND0_DMA3_PKG_IRQ_EN)) {
		uarts_sending &= ~(1 << 3);
	}
	// UART0
	if (H3_DMA->IRQ_PEND0 & (DMA_IRQ_PEND0_DMA0_HALF_IRQ_EN | DMA_IRQ_PEND0_DMA0_PKG_IRQ_EN)) {
		uarts_sending &= ~(1 << 0);
	}
#endif

	if (H3_GIC_CPUIF->IA == H3_DMA_IRQn) {
		H3_DMA->IRQ_PEND0 |= H3_DMA->IRQ_PEND0;
			
		H3_GIC_CPUIF->EOI = H3_DMA_IRQn;
		gic_unpend(H3_DMA_IRQn);
		isb();
		
		if (uarts_sending == 0) {
			dmb();
			dmx_send_state = DMXINTER;
		}
	}

	// Single 'threaded', there is just a single RDM process

	if (H3_UART1->O08.IIR & UART_IIR_IID_RD) {
		fiq_rdm_in_handler(1, (H3_UART_TypeDef *) H3_UART1_BASE);
		H3_GIC_CPUIF->EOI = H3_UART1_IRQn;
		gic_unpend(H3_UART1_IRQn);
	} else if (H3_UART2->O08.IIR & UART_IIR_IID_RD) {
		fiq_rdm_in_handler(2, (H3_UART_TypeDef *) H3_UART2_BASE);
		H3_GIC_CPUIF->EOI = H3_UART2_IRQn;
		gic_unpend(H3_UART2_IRQn);
	}
#if defined (ORANGE_PI_ONE)
	else if (H3_UART3->O08.IIR & UART_IIR_IID_RD) {
		fiq_rdm_in_handler(3, (H3_UART_TypeDef *) H3_UART3_BASE);
		H3_GIC_CPUIF->EOI = H3_UART3_IRQn;
		gic_unpend(H3_UART3_IRQn);
	} else if (H3_UART0->O08.IIR & UART_IIR_IID_RD) {
		fiq_rdm_in_handler(0, (H3_UART_TypeDef *) H3_UART0_BASE);
		H3_GIC_CPUIF->EOI = H3_UART0_IRQn;
		gic_unpend(H3_UART0_IRQn);
	}
#endif

#ifdef LOGIC_ANALYZER
	h3_gpio_clr(3);
#endif
	dmb();
}

static void uart_enable_fifo(uint8_t uart) {	// DMX TX
	H3_UART_TypeDef *p = _get_uart(uart);

	assert(p != 0);

	p->O08.FCR = UART_FCR_EFIFO | UART_FCR_TRESET;
	p->O04.IER = 0;
	isb();
}

static void uart_disable_fifo(uint8_t uart) {	// RDM RX
	H3_UART_TypeDef *p = _get_uart(uart);

	assert(p != 0);

	p->O08.FCR = 0;
	p->O04.IER = UART_IER_ERBFI;
	isb();
}

static void uart_init(uint8_t uart) {
	assert(uart < DMX_MAX_OUT);

	H3_UART_TypeDef *p = 0;

	if (uart == 1) {
		p = (H3_UART_TypeDef *)H3_UART1_BASE;

		uint32_t value = H3_PIO_PORTG->CFG0;
		// PG6, TX
		value &= ~(GPIO_SELECT_MASK << PG6_SELECT_CFG0_SHIFT);
		value |= H3_PG6_SELECT_UART1_TX << PG6_SELECT_CFG0_SHIFT;
		// PG7, RX
		value &= ~(GPIO_SELECT_MASK << PG7_SELECT_CFG0_SHIFT);
		value |= H3_PG7_SELECT_UART1_RX << PG7_SELECT_CFG0_SHIFT;
		H3_PIO_PORTG->CFG0 = value;

		H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART1;
		H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART1;
	} else if (uart == 2) {
		p = (H3_UART_TypeDef *)H3_UART2_BASE;

		uint32_t value = H3_PIO_PORTA->CFG0;
		// PA0, TX
		value &= ~(GPIO_SELECT_MASK << PA0_SELECT_CFG0_SHIFT);
		value |= H3_PA0_SELECT_UART2_TX << PA0_SELECT_CFG0_SHIFT;
		// PA1, RX
		value &= ~(GPIO_SELECT_MASK << PA1_SELECT_CFG0_SHIFT);
		value |= H3_PA1_SELECT_UART2_RX << PA1_SELECT_CFG0_SHIFT;
		H3_PIO_PORTA->CFG0 = value;

		H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART2;
		H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART2;
	} else if (uart == 3) {
		p = (H3_UART_TypeDef *)H3_UART3_BASE;

		uint32_t value = H3_PIO_PORTA->CFG1;
		// PA13, TX
		value &= ~(GPIO_SELECT_MASK << PA13_SELECT_CFG1_SHIFT);
		value |= H3_PA13_SELECT_UART3_TX << PA13_SELECT_CFG1_SHIFT;
		// PA14, RX
		value &= ~(GPIO_SELECT_MASK << PA14_SELECT_CFG1_SHIFT);
		value |= H3_PA14_SELECT_UART3_RX << PA14_SELECT_CFG1_SHIFT;
		H3_PIO_PORTA->CFG1 = value;

		H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART3;
		H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART3;
	} else if (uart == 0) {
		p = (H3_UART_TypeDef *)H3_UART0_BASE;

		uint32_t value = H3_PIO_PORTA->CFG0;
		// PA4, TX
		value &= ~(GPIO_SELECT_MASK << PA4_SELECT_CFG0_SHIFT);
		value |= H3_PA4_SELECT_UART0_TX << PA4_SELECT_CFG0_SHIFT;
		// PA5, RX
		value &= ~(GPIO_SELECT_MASK << PA5_SELECT_CFG0_SHIFT);
		value |= H3_PA5_SELECT_UART0_RX << PA5_SELECT_CFG0_SHIFT;
		H3_PIO_PORTA->CFG0 = value;

		H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART0;
		H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART0;
	}

	assert(p != 0);

	p->O08.FCR = 0;
	p->LCR = UART_LCR_DLAB;
	p->O00.DLL = BAUD_250000_L;
	p->O04.DLH = BAUD_250000_H;
	p->O04.IER = 0;
	p->LCR = UART_LCR_8_N_2;

	isb();
}

static void dmx_multi_start_data(uint8_t uart) {
	assert(uart_state[uart] == UART_STATE_IDLE);

	switch (dmx_port_direction[uart]) {
	case DMX_PORT_DIRECTION_OUTP:
		uart_enable_fifo(uart);
		dmb();
		uart_state[uart] = UART_STATE_TX;
		break;
	case DMX_PORT_DIRECTION_INP:
		rdm_receive_state[uart] = IDLE;

		H3_UART_TypeDef *p = _get_uart(uart);

		while ((p->USR & UART_USR_BUSY) == UART_USR_BUSY) {
			(void) p->O00.RBR;
		}

		uart_disable_fifo(uart);
		dmb();
		uart_state[uart] = UART_STATE_RX;
		break;
	default:
		assert(0);
		break;
	}
}

static void dmx_multi_stop_data(uint8_t uart) {
	assert(uart < DMX_MAX_OUT);

	dmb();
	if (uart_state[uart] == UART_STATE_IDLE) {
		return;
	}

	if (dmx_port_direction[uart] == DMX_PORT_DIRECTION_OUTP) {
		H3_UART_TypeDef *p = _get_uart(uart);
		bool is_idle = false;

		do {
			dmb();
			if (dmx_send_state == DMXINTER) {
				while ((p->USR & UART_USR_BUSY) == UART_USR_BUSY)
					;
				is_idle = true;
			}
		} while (!is_idle);
	}

	dmb();
	uart_state[uart] = UART_STATE_IDLE;
}

void dmx_multi_set_port_send_data_without_sc(uint8_t port, const uint8_t *data, uint16_t length) {
	assert(data != 0);
	assert(length != 0);

	const uint32_t uart = _port_to_uart(port);
	assert(uart < DMX_MAX_OUT);

	const uint32_t next = (dmx_data_write_index[uart] + 1) & (DMX_DATA_OUT_INDEX - 1);
	struct _dmx_multi_data *p = &p_coherent_region->dmx_data[uart][next];

	uint8_t *dst = p->data;
	p->length = length + 1;

	memcpy(&dst[1], data, (size_t) length);

	dmx_data_write_index[uart] = next;
}

void dmx_multi_set_port_direction(uint8_t port, _dmx_port_direction port_direction, bool enable_data) {
	const uint32_t uart = _port_to_uart(port);

	if (port_direction != dmx_port_direction[uart]) {
		dmx_multi_stop_data(uart);
		switch (port_direction) {
		case DMX_PORT_DIRECTION_OUTP:
			h3_gpio_set(dmx_data_direction_gpio_pin[uart]);	// 0 = input, 1 = output
			dmx_port_direction[uart] = DMX_PORT_DIRECTION_OUTP;
			break;
		case DMX_PORT_DIRECTION_INP:
			h3_gpio_clr(dmx_data_direction_gpio_pin[uart]);	// 0 = input, 1 = output
			dmx_port_direction[uart] = DMX_PORT_DIRECTION_INP;
			break;
		default:
			assert(0);
			break;
		}
	} else if (!enable_data) {
		dmx_multi_stop_data(uart);
	}

	if (enable_data) {
		dmx_multi_start_data(uart);
	}
}

const uint8_t *dmx_multi_rdm_get_available(uint8_t uart)  {
	dmb();

	if (rdm_data_write_index[uart] == rdm_data_read_index[uart]) {
		return NULL;
	} else {
		const uint8_t *p = &rdm_data[uart][rdm_data_read_index[uart]].data[0];
		rdm_data_read_index[uart] = (rdm_data_read_index[uart] + 1) & RDM_DATA_BUFFER_INDEX_MASK;
		return p;
	}
}

uint32_t dmx_multi_get_output_break_time(void) {
	return dmx_output_break_time;
}

void dmx_multi_set_output_break_time(uint32_t break_time) {
	dmx_output_break_time = MAX((uint32_t)DMX_TRANSMIT_BREAK_TIME_MIN, break_time);
	dmx_output_break_time_intv = dmx_output_break_time * 12;
}

uint32_t dmx_multi_get_output_mab_time(void) {
	return dmx_output_mab_time;
}

void dmx_multi_set_output_mab_time(uint32_t mab_time) {
	dmx_output_mab_time = MAX((uint32_t)DMX_TRANSMIT_MAB_TIME_MIN, mab_time);
	dmx_output_mab_time_intv = dmx_output_mab_time * 12;
}

uint32_t dmx_multi_get_output_period(void) {
	return dmx_output_period;
}

void dmx_multi_init_set_gpiopin(uint8_t port, uint8_t gpio_pin) {
	dmx_data_direction_gpio_pin[_port_to_uart(port)] = gpio_pin;
}

void dmx_multi_init(void) {
	uint32_t i;

#ifdef LOGIC_ANALYZER
	h3_gpio_fsel(3, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(3);
	h3_gpio_fsel(6, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(6);
	h3_gpio_fsel(20, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(20);
#endif

	for (i = 0; i < DMX_MAX_OUT; i++) {
		// DMX TX
		dmx_multi_clear_data(i);
		dmx_data_write_index[i] = 0;
		dmx_data_read_index[i] = 0;
		// DMA UART TX
		struct sunxi_dma_lli *lli = &p_coherent_region->lli[i];
		H3_UART_TypeDef *p = _get_uart(i);

		lli->cfg = DMA_CHAN_CFG_DST_IO_MODE | DMA_CHAN_CFG_SRC_LINEAR_MODE | DMA_CHAN_CFG_SRC_DRQ(DRQSRC_SDRAM) | DMA_CHAN_CFG_DST_DRQ(i + DRQDST_UART0TX);
		lli->src = (uint32_t) &p_coherent_region->dmx_data[i][dmx_data_read_index[i]].data[0];
		lli->dst = (uint32_t) &p->O00.THR;
		lli->len = p_coherent_region->dmx_data[i][dmx_data_read_index[i]].length;
		lli->para = DMA_NORMAL_WAIT;
		lli->p_lli_next = DMA_LLI_LAST_ITEM;
		//
		dmx_port_direction[i] = DMX_PORT_DIRECTION_INP;
		//
		uart_state[i] = UART_STATE_IDLE;
		// RDM RX
		rdm_data_write_index[i] = 0;
		rdm_data_read_index[i] = 0;
		rdm_data_current[i] = &rdm_data[i][0];
		rdm_receive_state[i] = IDLE;
	}

	/*
	 * OPIZERO	OPIONE	OUT	PORT
	 * -		UART1	1	0
	 * UART2	UART2	2	1
	 * UART1	UART3	3	2
	 * -		UART0	4	3
	 */

	//FIXME move to top
#if defined(ORANGE_PI) || defined(NANO_PI)
	dmx_data_direction_gpio_pin[2] = GPIO_DMX_DATA_DIRECTION_OUT_B;
	dmx_data_direction_gpio_pin[1] = GPIO_DMX_DATA_DIRECTION_OUT_C;
#elif defined (ORANGE_PI_ONE)
	dmx_data_direction_gpio_pin[0] = GPIO_DMX_DATA_DIRECTION_OUT_D;
	dmx_data_direction_gpio_pin[1] = GPIO_DMX_DATA_DIRECTION_OUT_A;
	dmx_data_direction_gpio_pin[2] = GPIO_DMX_DATA_DIRECTION_OUT_B;
	dmx_data_direction_gpio_pin[3] = GPIO_DMX_DATA_DIRECTION_OUT_C;
#endif

	h3_gpio_fsel(dmx_data_direction_gpio_pin[1], GPIO_FSEL_OUTPUT);
	h3_gpio_clr(dmx_data_direction_gpio_pin[1]);	// 0 = input, 1 = output
	h3_gpio_fsel(dmx_data_direction_gpio_pin[2], GPIO_FSEL_OUTPUT);
	h3_gpio_clr(dmx_data_direction_gpio_pin[2]);	// 0 = input, 1 = output
#if defined (ORANGE_PI_ONE)
	h3_gpio_fsel(dmx_data_direction_gpio_pin[3], GPIO_FSEL_OUTPUT);
	h3_gpio_clr(dmx_data_direction_gpio_pin[3]);	// 0 = input, 1 = output
	h3_gpio_fsel(dmx_data_direction_gpio_pin[0], GPIO_FSEL_OUTPUT);
	h3_gpio_clr(dmx_data_direction_gpio_pin[0]);	// 0 = input, 1 = output
#endif

	uarts_sending = 0;

	uart_init(1);
	uart_init(2);
#if defined (ORANGE_PI_ONE)
	uart_init(3);
 #ifndef DO_NOT_USE_UART0
	uart_init(0);
 #endif
#endif

	__disable_fiq();

	arm_install_handler((unsigned) fiq_dmx_multi, ARM_VECTOR(ARM_VECTOR_FIQ));

	gic_fiq_config(H3_DMA_IRQn, GIC_CORE0);

	gic_fiq_config(H3_UART1_IRQn, 1);
	gic_fiq_config(H3_UART2_IRQn, 1);
#if defined (ORANGE_PI_ONE)
	gic_fiq_config(H3_UART3_IRQn, 1);
 #ifndef DO_NOT_USE_UART0
	gic_fiq_config(H3_UART0_IRQn, 1);
 #endif
#endif

	dmx_send_state = IDLE;

	uart_enable_fifo(1);
	uart_enable_fifo(2);
#if defined (ORANGE_PI_ONE)
	uart_enable_fifo(3);
 #ifndef DO_NOT_USE_UART0
	uart_enable_fifo(0);
 #endif
#endif

	irq_timer_init();
	irq_timer_set(IRQ_TIMER_0, irq_timer0_dmx_multi_sender);

	H3_TIMER->TMR0_CTRL |= TIMER_CTRL_SINGLE_MODE;
	H3_TIMER->TMR0_INTV = (dmx_output_period + 4) * 12;
	H3_TIMER->TMR0_CTRL |= 0x3;

	H3_DMA->IRQ_PEND0 |= H3_DMA->IRQ_PEND0;
	H3_DMA->IRQ_PEND1 |= H3_DMA->IRQ_PEND1;

	H3_DMA->IRQ_EN0 = DMA_IRQ_EN0_DMA0_PKG_IRQ_EN |  DMA_IRQ_EN0_DMA1_PKG_IRQ_EN | DMA_IRQ_EN0_DMA2_PKG_IRQ_EN | DMA_IRQ_EN0_DMA3_PKG_IRQ_EN;

	isb();

	__enable_fiq();
}
