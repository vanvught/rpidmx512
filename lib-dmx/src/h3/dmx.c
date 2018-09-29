/**
 * @file dmx.c
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
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "arm/arm.h"
#include "arm/synchronize.h"
#include "arm/gic.h"
#include "uart.h"

#include "h3_gpio.h"
#include "h3_ccu.h"
#include "h3_timer.h"
#include "h3_hs_timer.h"
#include "h3_board.h"

#include "irq_timer.h"

#include "gpio.h"
#include "dmx.h"
#include "rdm.h"
#include "rdm_e120.h"

#include "util.h"

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
} _dmx_state;

static uint8_t dmx_data_direction_gpio_pin = GPIO_DMX_DATA_DIRECTION;

static volatile uint32_t dmx_data_buffer_index_head = 0;
static volatile uint32_t dmx_data_buffer_index_tail = 0;
static struct _dmx_data dmx_data[DMX_DATA_BUFFER_INDEX_ENTRIES] ALIGNED;
static uint8_t dmx_data_previous[DMX_DATA_BUFFER_SIZE] ALIGNED;
static volatile _dmx_state dmx_receive_state = IDLE;
static volatile uint32_t dmx_data_index = 0;

static uint32_t dmx_output_break_time = (uint32_t) DMX_TRANSMIT_BREAK_TIME_MIN;
static uint32_t dmx_output_mab_time = (uint32_t) DMX_TRANSMIT_MAB_TIME_MIN;
static uint32_t dmx_output_period = (uint32_t) DMX_TRANSMIT_PERIOD_DEFAULT;
static uint32_t dmx_output_period_requested = (uint32_t) DMX_TRANSMIT_PERIOD_DEFAULT;

static uint32_t dmx_output_break_time_intv = (uint32_t) (DMX_TRANSMIT_BREAK_TIME_MIN * 12);
static uint32_t dmx_output_mab_time_intv = (uint32_t) (DMX_TRANSMIT_MAB_TIME_MIN * 12);
static uint32_t dmx_output_period_intv = (uint32_t) (DMX_TRANSMIT_PERIOD_DEFAULT * 12);

static uint32_t dmx_send_data_length = (uint32_t) (DMX_UNIVERSE_SIZE + 1);		///< SC + UNIVERSE SIZE
static _dmx_port_direction dmx_port_direction = DMX_PORT_DIRECTION_INP;
static volatile uint32_t dmx_fiq_micros_current = 0;
static volatile uint32_t dmx_fiq_micros_previous = 0;
static volatile bool dmx_is_previous_break_dmx = false;
static volatile uint32_t dmx_break_to_break_latest = 0;
static volatile uint32_t dmx_break_to_break_previous = 0;
static volatile uint32_t dmx_slots_in_packet_previous = 0;
static volatile _dmx_state dmx_send_state = IDLE;
static volatile bool dmx_send_always = false;
static volatile uint32_t dmx_send_break_micros = 0;
static volatile uint32_t dmx_send_current_slot = 0;
static bool is_stopped = true;

static volatile uint32_t rdm_data_buffer_index_head = 0;
static volatile uint32_t rdm_data_buffer_index_tail = 0;
static uint8_t rdm_data_buffer[RDM_DATA_BUFFER_INDEX_ENTRIES][RDM_DATA_BUFFER_SIZE] ALIGNED;
static volatile uint16_t rdm_checksum = (uint16_t) 0;							///< This must be uint16_t
static volatile uint32_t rdm_data_receive_end = 0;
static volatile uint32_t rdm_disc_index = 0;

static volatile uint32_t dmx_updates_per_seconde = 0;
static uint32_t dmx_packets_previous = 0;
static volatile struct _total_statistics total_statistics ALIGNED;

static void dmx_set_send_data_length(uint16_t send_data_length) {
	dmx_send_data_length = send_data_length;
	dmx_set_output_period(dmx_output_period_requested);
}

volatile uint32_t dmx_get_updates_per_seconde(void) {
	dmb();
	return dmx_updates_per_seconde;
}

void dmx_set_output_period(const uint32_t period) {
	const uint32_t package_length_us = dmx_output_break_time + dmx_output_mab_time + (dmx_send_data_length * 44);

	dmx_output_period_requested = period;

	if (period != 0) {
		if (period < package_length_us) {
			dmx_output_period = (uint32_t) MAX(DMX_TRANSMIT_BREAK_TO_BREAK_TIME_MIN, package_length_us + 44);
		} else {
			dmx_output_period = period;
		}
	} else {
		dmx_output_period = (uint32_t) MAX(DMX_TRANSMIT_BREAK_TO_BREAK_TIME_MIN, package_length_us + 44);
	}

	dmx_output_period_intv = dmx_output_period * 12;
}

void dmx_set_send_data(const uint8_t *data, uint16_t length) {
	do {
		dmb();
	} while (dmx_send_state != IDLE && dmx_send_state != DMXINTER);

	memcpy(dmx_data[0].data, data, (size_t)length);
	dmx_set_send_data_length(length);
}

void dmx_set_send_data_without_sc(const uint8_t *data, uint16_t length) {
	do {
		dmb();
	} while (dmx_send_state != IDLE && dmx_send_state != DMXINTER);

	dmx_data[0].data[0] = DMX512_START_CODE;
	memcpy(&dmx_data[0].data[1], data, (size_t) length);
	dmx_set_send_data_length(length + 1);
}

void dmx_clear_data(void) {
	uint32_t i = sizeof(dmx_data) / sizeof(uint32_t);
	uint32_t *p = (uint32_t *)dmx_data;

	while (i-- != (uint32_t) 0) {
		*p++ = (uint32_t) 0;
	}
}

uint32_t dmx_get_output_period(void) {
	return dmx_output_period;
}

uint16_t dmx_get_send_data_length(void) {
	return dmx_send_data_length;
}

const uint8_t *rdm_get_available(void)  {
	dmb();
	if (rdm_data_buffer_index_head == rdm_data_buffer_index_tail) {
		return NULL;
	} else {
		const uint8_t *p = &rdm_data_buffer[rdm_data_buffer_index_tail][0];
		rdm_data_buffer_index_tail = (rdm_data_buffer_index_tail + 1) & RDM_DATA_BUFFER_INDEX_MASK;
		return p;
	}
}

const uint8_t *rdm_get_current_data(void) {
	return &rdm_data_buffer[rdm_data_buffer_index_tail][0];
}

const uint8_t *dmx_get_available(void)  {
	dmb();
	if (dmx_data_buffer_index_head == dmx_data_buffer_index_tail) {
		return NULL;
	} else {
		const uint8_t *p = dmx_data[dmx_data_buffer_index_tail].data;
		dmx_data_buffer_index_tail = (dmx_data_buffer_index_tail + 1) & DMX_DATA_BUFFER_INDEX_MASK;
		return p;
	}
}

const uint8_t *dmx_get_current_data(void) {
	return dmx_data[dmx_data_buffer_index_tail].data;
}

volatile uint8_t dmx_get_receive_state(void) {
	dmb();
	return dmx_receive_state;
}

const uint8_t *dmx_is_data_changed(void) {
	uint32_t i;
	uint8_t const *p = (uint8_t *)dmx_get_available();
	uint32_t *src = (uint32_t *)p;
	uint32_t *dst = (uint32_t *)dmx_data_previous;
	bool is_changed = false;

	if (src == NULL) {
		return NULL;
	}

	const struct _dmx_data *dmx_statistics = (struct _dmx_data *)p;

	if (dmx_statistics->statistics.slots_in_packet != dmx_slots_in_packet_previous) {
		dmx_slots_in_packet_previous = dmx_statistics->statistics.slots_in_packet;
		for (i = 0; i < DMX_DATA_BUFFER_SIZE / 4; i++) {
			*dst= *src;
			dst++;
			src++;
		}
		return p;
	}

	for (i = 0; i < DMX_DATA_BUFFER_SIZE / 4; i++) {
		if (*dst != *src) {
			*dst = *src;
			is_changed = true;
		}
		dst++;
		src++;
	}

	return (is_changed ? p : NULL);
}

_dmx_port_direction dmx_get_port_direction(void) {
	return dmx_port_direction;
}

uint32_t rdm_get_data_receive_end(void) {
	return rdm_data_receive_end;
}

uint32_t dmx_get_output_break_time(void) {
	return dmx_output_break_time;
}

void dmx_set_output_break_time(uint32_t break_time) {
	dmx_output_break_time = MAX((uint32_t)DMX_TRANSMIT_BREAK_TIME_MIN, break_time);
	dmx_output_break_time_intv = dmx_output_break_time * 12;

	dmx_set_output_period(dmx_output_period_requested);
}

uint32_t dmx_get_output_mab_time(void) {
	return dmx_output_mab_time;
}

void dmx_set_output_mab_time(uint32_t mab_time) {
	dmx_output_mab_time = MAX((uint32_t)DMX_TRANSMIT_MAB_TIME_MIN, mab_time);
	dmx_output_mab_time_intv = dmx_output_mab_time * 12;

	dmx_set_output_period(dmx_output_period_requested);
}

void dmx_reset_total_statistics(void) {
	total_statistics.dmx_packets = (uint32_t) 0;
	total_statistics.rdm_packets = (uint32_t) 0;
}

const volatile struct _total_statistics *dmx_get_total_statistics(void) {
	return &total_statistics;
}

/**
 * Interrupt handler for continues receiving DMX512 data.
 */
static void fiq_dmx_in_handler(void) {
	dmx_fiq_micros_current = h3_hs_timer_lo_us();

	if (EXT_UART->LSR & UART_LSR_BI) {
#ifdef LOCIG_ANALYZER
		h3_gpio_set(11); //BREAK
#endif

		dmx_receive_state = PRE_BREAK;
		dmx_break_to_break_latest = dmx_fiq_micros_current;
	} else if (EXT_UART->O08.IIR & UART_IIR_IID_RD) {
#ifdef LOCIG_ANALYZER
		h3_gpio_set(6); //DR
#endif

		const uint8_t data = EXT_UART->O00.RBR;

		switch (dmx_receive_state) {
		case IDLE:
#ifdef LOCIG_ANALYZER
			h3_gpio_clr(16);
#endif
			if (data == 0xFE) {
				dmx_receive_state = RDMDISCFE;
				rdm_data_buffer[rdm_data_buffer_index_head][0] = 0xFE;
				dmx_data_index = 1;
			}
			break;
		case PRE_BREAK:
			dmx_receive_state = BREAK;
			break;
		case BREAK:
			switch (data) {
			case DMX512_START_CODE:
				dmx_receive_state = DMXDATA;
				dmx_data[dmx_data_buffer_index_head].data[0] = DMX512_START_CODE;
				dmx_data_index = 1;
				total_statistics.dmx_packets = total_statistics.dmx_packets + 1;

				if (dmx_is_previous_break_dmx) {
					dmx_data[dmx_data_buffer_index_head].statistics.break_to_break = dmx_break_to_break_latest - dmx_break_to_break_previous;
					dmx_break_to_break_previous = dmx_break_to_break_latest;

				} else {
					dmx_is_previous_break_dmx = true;
					dmx_break_to_break_previous = dmx_break_to_break_latest;
				}
#ifdef LOCIG_ANALYZER
				h3_gpio_clr(11);
				h3_gpio_set(14);
#endif
				break;
			case E120_SC_RDM:
				dmx_receive_state = RDMDATA;
				rdm_data_buffer[rdm_data_buffer_index_head][0] = E120_SC_RDM;
				rdm_checksum = E120_SC_RDM;
				dmx_data_index = 1;
				total_statistics.rdm_packets = total_statistics.rdm_packets + 1;
				dmx_is_previous_break_dmx = false;
				break;
			default:
				dmx_receive_state = IDLE;
				dmx_is_previous_break_dmx = false;
				break;
			}
			break;
		case DMXDATA:
#ifdef LOCIG_ANALYZER
			h3_gpio_clr(14);
			h3_gpio_set(16);
#endif
			dmx_data[dmx_data_buffer_index_head].statistics.slot_to_slot = dmx_fiq_micros_current - dmx_fiq_micros_previous;
			dmx_data[dmx_data_buffer_index_head].data[dmx_data_index++] = data;

			H3_TIMER->TMR0_INTV = (dmx_data[0].statistics.slot_to_slot + (uint32_t) 12) * 12;
			H3_TIMER->TMR0_CTRL |= 0x3;

			if (dmx_data_index > DMX_UNIVERSE_SIZE) {
#ifdef LOCIG_ANALYZER
				h3_gpio_clr(16);
#endif
				dmx_receive_state = IDLE;
				dmx_data[dmx_data_buffer_index_head].statistics.slots_in_packet = DMX_UNIVERSE_SIZE;
				dmx_data_buffer_index_head = (dmx_data_buffer_index_head + 1) & DMX_DATA_BUFFER_INDEX_MASK;
				dmb();
			}
			break;
		case RDMDATA:
			if (dmx_data_index > RDM_DATA_BUFFER_SIZE) {
				dmx_receive_state = IDLE;
			} else {
				rdm_data_buffer[rdm_data_buffer_index_head][dmx_data_index++] = data;
				rdm_checksum += data;

				const struct _rdm_command *p = (struct _rdm_command *)(&rdm_data_buffer[rdm_data_buffer_index_head][0]);
				if (dmx_data_index == p->message_length) {
					dmx_receive_state = CHECKSUMH;
				}
			}
			break;
		case CHECKSUMH:
			rdm_data_buffer[rdm_data_buffer_index_head][dmx_data_index++] =	data;
			rdm_checksum -= data << 8;
			dmx_receive_state = CHECKSUML;
			break;
		case CHECKSUML:
			rdm_data_buffer[rdm_data_buffer_index_head][dmx_data_index++] = data;
			rdm_checksum -= data;
			const struct _rdm_command *p = (struct _rdm_command *)(&rdm_data_buffer[rdm_data_buffer_index_head][0]);

			if ((rdm_checksum == 0) && (p->sub_start_code == E120_SC_SUB_MESSAGE)) {
				rdm_data_buffer_index_head = (rdm_data_buffer_index_head + 1) & RDM_DATA_BUFFER_INDEX_MASK;
				rdm_data_receive_end = h3_hs_timer_lo_us();;
				dmb();
			}
			dmx_receive_state = IDLE;
			break;
		case RDMDISCFE:
			rdm_data_buffer[rdm_data_buffer_index_head][dmx_data_index++] = data;

			if ((data == 0xAA) || (dmx_data_index == 9 )) {
				dmx_receive_state = RDMDISCEUID;
				rdm_disc_index = 0;
			}
			break;
		case RDMDISCEUID:
			rdm_data_buffer[rdm_data_buffer_index_head][dmx_data_index++] = data;
			rdm_disc_index++;

			if (rdm_disc_index == 2 * RDM_UID_SIZE) {
				dmx_receive_state = RDMDISCECS;
				rdm_disc_index = 0;
			}
			break;
		case RDMDISCECS:
			rdm_data_buffer[rdm_data_buffer_index_head][dmx_data_index++] = data;
			rdm_disc_index++;

			if (rdm_disc_index == 4) {
				rdm_data_buffer_index_head = (rdm_data_buffer_index_head + 1) & RDM_DATA_BUFFER_INDEX_MASK;
				dmx_receive_state = IDLE;
				rdm_data_receive_end = h3_hs_timer_lo_us();;
				dmb();
			}

			break;
		default:
			dmx_receive_state = IDLE;
			dmx_is_previous_break_dmx = false;
			break;
		}
#ifdef LOCIG_ANALYZER
		h3_gpio_clr(6);
#endif
	} else {
	}

	dmx_fiq_micros_previous = dmx_fiq_micros_current;
}

/**
 * Timer 0 interrupt DMX Receiver
 * Slot time-out
 */
static void irq_timer0_dmx_receive(uint32_t clo) {
	dmb();
	if (dmx_receive_state == DMXDATA) {
		if (clo - dmx_fiq_micros_current > dmx_data[0].statistics.slot_to_slot) {
			dmb();
			dmx_receive_state = IDLE;
			dmx_data[dmx_data_buffer_index_head].statistics.slots_in_packet = dmx_data_index - 1;
			dmx_data_buffer_index_head = (dmx_data_buffer_index_head + 1) & DMX_DATA_BUFFER_INDEX_MASK;
#ifdef LOCIG_ANALYZER
			h3_gpio_clr(16);
#endif
		} else {
			H3_TIMER->TMR0_INTV = dmx_data[dmx_data_buffer_index_head].statistics.slot_to_slot * 12;
			H3_TIMER->TMR0_CTRL |= 0x3;
		}
	}
}

/**
 * Timer 1 interrupt DMX Receiver
 * Statistics
 */
static void irq_timer1_dmx_receive(uint32_t clo) {
	dmb();
	dmx_updates_per_seconde = total_statistics.dmx_packets - dmx_packets_previous;
	dmx_packets_previous = total_statistics.dmx_packets;
}

/**
 * Timer 0 interrupt DMX Sender
 */
static void irq_timer0_dmx_sender(uint32_t clo) {
	switch (dmx_send_state) {
	case IDLE:
	case DMXINTER:
		H3_TIMER->TMR0_INTV = dmx_output_break_time_intv;
		H3_TIMER->TMR0_CTRL |= 0x3;

		EXT_UART->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
		dmx_send_break_micros = clo;
		dmb();
		dmx_send_state = BREAK;
		break;
	case BREAK:
		H3_TIMER->TMR0_INTV = dmx_output_mab_time_intv;
		H3_TIMER->TMR0_CTRL |= 0x3;

		EXT_UART->LCR = UART_LCR_8_N_2;
		dmb();
		dmx_send_state = MAB;
		break;
	case MAB:
		H3_TIMER->TMR0_INTV = dmx_output_period_intv;
		H3_TIMER->TMR0_CTRL |= 0x3;

		uint32_t fifo_cnt = 16;

		for (dmx_send_current_slot = 0; fifo_cnt-- > 0; dmx_send_current_slot++) {
			if (dmx_send_current_slot >= dmx_send_data_length) {
				break;
			}

			EXT_UART->O00.THR = dmx_data[0].data[dmx_send_current_slot];
		}

		if (dmx_send_current_slot < dmx_send_data_length) {
			dmb();
			dmx_send_state = DMXDATA;
			EXT_UART->O04.IER = UART_IER_ETBEI;
		} else {
			dmb();
			dmx_send_state = DMXINTER;
		}

		break;
	case DMXDATA:
		printf("Output period too short (brk %d, mab %d, period %d, dlen %d, slot %d)\n",
				(int)dmx_output_break_time, (int)dmx_output_mab_time, (int)dmx_output_period, (int)dmx_send_data_length, (int)dmx_send_current_slot);
		assert(0);
		break;
	default:
		assert(0);
		break;
	}
}

/**
 * EXT_UART TX interrupt
 */
static void fiq_dmx_out_handler(void) {
	if (EXT_UART->O08.IIR & UART_IIR_IID_THRE) {

		uint32_t fifo_cnt = 16;

		for (; fifo_cnt-- > 0; dmx_send_current_slot++) {
			if (dmx_send_current_slot >= dmx_send_data_length) {
				break;
			}

			EXT_UART->O00.THR = dmx_data[0].data[dmx_send_current_slot];
		}

		if (dmx_send_current_slot >= dmx_send_data_length) {
			EXT_UART->O04.IER &= ~UART_IER_ETBEI; //UART_IER_PTIME;
			dmb();
			dmx_send_state = DMXINTER;
		}
	}
}

static void __attribute__((interrupt("FIQ"))) fiq_dmx(void) {
	dmb();

	if (dmx_port_direction == DMX_PORT_DIRECTION_INP) {
		fiq_dmx_in_handler();
	} else {
		fiq_dmx_out_handler();
	}

#if (EXT_UART_NUMBER == 1)
	H3_GIC_CPUIF->EOI = H3_UART1_IRQn;
	gic_unpend(H3_UART1_IRQn);
#elif (EXT_UART_NUMBER == 3)
	H3_GIC_CPUIF->EOI = H3_UART3_IRQn;
	gic_unpend(H3_UART3_IRQn);
#endif

	dmb();
}

static void uart_enable_fifo(void) {	// DMX Output
	EXT_UART->O08.FCR = UART_FCR_EFIFO | UART_FCR_TRESET;
	EXT_UART->O04.IER = 0;
	isb();
}

static void uart_disable_fifo(void) {	// DMX Input
	EXT_UART->O08.FCR = 0;
	EXT_UART->O04.IER = UART_IER_ERBFI;
	isb();
}

static void dmx_start_data(void) {
	switch (dmx_port_direction) {
	case DMX_PORT_DIRECTION_OUTP:
		dmx_send_always = true;
		dmx_send_state = IDLE;

		uart_enable_fifo();
		__enable_fiq();

		irq_timer_set(IRQ_TIMER_0, irq_timer0_dmx_sender);

		const uint32_t clo = h3_hs_timer_lo_us();

		if (clo - dmx_send_break_micros > dmx_output_period) {
			H3_TIMER->TMR0_CTRL |= TIMER_CTRL_SINGLE_MODE;
			H3_TIMER->TMR0_INTV = 4 * 12;
			H3_TIMER->TMR0_CTRL |= 0x3;
		} else {
			H3_TIMER->TMR0_CTRL |= TIMER_CTRL_SINGLE_MODE;
			H3_TIMER->TMR0_INTV = (dmx_output_period + 4) * 12;
			H3_TIMER->TMR0_CTRL |= 0x3;
		}

		isb();

		break;
	case DMX_PORT_DIRECTION_INP:
		dmx_receive_state = IDLE;

		irq_timer_set(IRQ_TIMER_0, irq_timer0_dmx_receive);

		while ((EXT_UART->USR & UART_USR_BUSY) == UART_USR_BUSY) {
			(void) EXT_UART->O00.RBR;
		}

		uart_disable_fifo();
		__enable_fiq();

		break;
	default:
		break;
	}

	is_stopped = false;
}

static void dmx_stop_data(void) {
	uint32_t i;

	if (is_stopped) {
		return;
	}

	if (dmx_send_always) {
		do {
			dmb();
			if (dmx_send_state == DMXINTER) {
				while ((EXT_UART->USR & UART_USR_BUSY) == UART_USR_BUSY)
					;
				dmb();
				dmx_send_state = IDLE;
			}
			dmb();
		} while (dmx_send_state != IDLE);
	}

	dmx_send_always = false;

	__disable_fiq();
	isb();

	irq_timer_set(IRQ_TIMER_0, NULL);

	dmx_receive_state = IDLE;

	for (i = 0; i < DMX_DATA_BUFFER_INDEX_ENTRIES; i++) {
		dmx_data[i].statistics.slots_in_packet = 0;
	}

	is_stopped = true;
}

void dmx_set_port_direction(_dmx_port_direction port_direction, bool enable_data) {
	if (port_direction != dmx_port_direction) {
		dmx_stop_data();

		switch (port_direction) {
		case DMX_PORT_DIRECTION_OUTP:
			h3_gpio_set(dmx_data_direction_gpio_pin);// 0 = input, 1 = output
			dmx_port_direction = DMX_PORT_DIRECTION_OUTP;
			break;
		case DMX_PORT_DIRECTION_INP:
			h3_gpio_clr(dmx_data_direction_gpio_pin);// 0 = input, 1 = output
			dmx_port_direction = DMX_PORT_DIRECTION_INP;
			break;
		default:
			h3_gpio_clr(dmx_data_direction_gpio_pin);// 0 = input, 1 = output
			dmx_port_direction = DMX_PORT_DIRECTION_INP;
			break;
		}
	} else if (!enable_data) {
		dmx_stop_data();
	}

	if (enable_data) {
		dmx_start_data();
	}
}

static void uart_init(void) {
#if (EXT_UART_NUMBER == 1)
	uint32_t value = H3_PIO_PORTG->CFG0;
	// PG6, TX
	value &= ~(GPIO_SELECT_MASK << PG6_SELECT_CFG0_SHIFT);
	value |= H3_PG6_SELECT_UART1_TX << PG6_SELECT_CFG0_SHIFT;
	// PG7, RX
	value &= ~(GPIO_SELECT_MASK << PG7_SELECT_CFG0_SHIFT);
	value |= H3_PG7_SELECT_UART1_RX << PG7_SELECT_CFG0_SHIFT;
	H3_PIO_PORTG->CFG0 = value;

	H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART1;
	H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART1;
#elif (EXT_UART_NUMBER == 3)
	uint32_t value = H3_PIO_PORTA->CFG1;
	// PA13, TX
	value &= ~(GPIO_SELECT_MASK << PA13_SELECT_CFG1_SHIFT);
	value |= H3_PA13_SELECT_UART3_TX << PA13_SELECT_CFG1_SHIFT;
	// PA14, RX
	value &= ~(GPIO_SELECT_MASK << PA14_SELECT_CFG1_SHIFT);
	value |= H3_PA14_SELECT_UART3_RX << PA14_SELECT_CFG1_SHIFT;
	H3_PIO_PORTA->CFG1 = value;

	H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART3;
	H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART3;
#else
 #error Unsupported UART device configured
#endif

	EXT_UART->O08.FCR = 0;

	EXT_UART->LCR = UART_LCR_DLAB;
	EXT_UART->O00.DLL = BAUD_250000_L;
	EXT_UART->O04.DLH = BAUD_250000_H;
	EXT_UART->LCR = UART_LCR_8_N_2;

	isb();
}

void dmx_init_set_gpiopin(uint8_t gpio_pin) {
	dmx_data_direction_gpio_pin = gpio_pin;
}

void dmx_init(void) {
	h3_gpio_fsel(dmx_data_direction_gpio_pin, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(dmx_data_direction_gpio_pin);	// 0 = input, 1 = output

#ifdef LOCIG_ANALYZER
	h3_gpio_clr(12); // FIQ
	h3_gpio_clr(11); // BREAK
	h3_gpio_clr(6);  // DR
	h3_gpio_clr(14); // SC
	h3_gpio_clr(16); // DATA
#endif

	dmx_clear_data();

	dmx_data_buffer_index_head = 0;
	dmx_data_buffer_index_tail = 0;

	rdm_data_buffer_index_head = 0;
	rdm_data_buffer_index_tail = 0;

	dmx_receive_state = IDLE;

	dmx_send_state = IDLE;
	dmx_send_always = false;

	irq_timer_init();

	irq_timer_set(IRQ_TIMER_1, irq_timer1_dmx_receive);
	H3_TIMER->TMR1_INTV = 0xB71B00; // 1 second
	H3_TIMER->TMR1_CTRL |= 0x3; /* set reload bit */

#if (EXT_UART_NUMBER == 1)
	gic_fiq_config(H3_UART1_IRQn, 1);
#elif (EXT_UART_NUMBER == 3)
	gic_fiq_config(H3_UART3_IRQn, 1);
#else
 #error Unsupported UART device configured
#endif

	uart_init();

	__disable_fiq();
	arm_install_handler((unsigned) fiq_dmx, ARM_VECTOR(ARM_VECTOR_FIQ));
}
