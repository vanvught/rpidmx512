/**
 * @file dmx.c
 *
 */
/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#include "bcm2835.h"
#include "bcm2835_gpio.h"
#include "bcm2835_vc.h"
#include "bcm2835_pl011.h"
#include "gpio.h"
#include "util.h"
#include "dmx.h"
#include "rdm.h"
#include "rdm_e120.h"

///< State of receiving DMX/RDM Bytes
typedef enum {
  IDLE = 0,		///<
  BREAK,		///<
  MAB,			///<
  DMXDATA,		///<
  RDMDATA,		///<
  CHECKSUMH,	///<
  CHECKSUML,	///<
  RDMDISCFE,	///<
  RDMDISCEUID,  ///<
  RDMDISCECS	///<
} _dmx_state;

uint8_t dmx_data[DMX_DATA_BUFFER_SIZE] __attribute__((aligned(4)));				///<
static uint8_t dmx_data_previous[DMX_DATA_BUFFER_SIZE] __attribute__((aligned(4)));	///<
static uint8_t dmx_receive_state = IDLE;										///< Current state of DMX receive
static uint16_t dmx_data_index = 0;												///<
static bool dmx_available = false;												///<
static uint32_t dmx_output_break_time = DMX_TRANSMIT_BREAK_TIME_MIN;			///<
static uint32_t dmx_output_mab_time = DMX_TRANSMIT_MAB_TIME_MIN;				///<
static uint32_t dmx_output_period = DMX_TRANSMIT_REFRESH_DEFAULT;				///<
static bool dmx_output_fast_as_possible = false;								///<
static uint16_t dmx_send_data_length = (uint16_t) DMX_UNIVERSE_SIZE + 1;		///< SC + UNIVERSE SIZE
static uint8_t dmx_port_direction = DMX_PORT_DIRECTION_INP;						///<
static volatile uint32_t dmx_fiq_micros_current = 0;							///< Timestamp FIQ
static volatile uint32_t dmx_fiq_micros_previous = 0;							///< Timestamp previous FIQ
static volatile uint32_t dmx_break_to_break_previous = 0;						///<
static uint32_t dmx_slots_in_packet_previous = 0;								///<
static volatile uint8_t dmx_send_state = IDLE;									///<
static volatile bool dmx_send_always = false;									///<
static volatile uint32_t dmx_send_break_micros = 0;								///<

static uint16_t rdm_data_buffer_index_head = 0;									///<
static uint16_t rdm_data_buffer_index_tail = 0;									///<
static uint8_t rdm_data_buffer[RDM_DATA_BUFFER_INDEX_SIZE + 1][RDM_DATA_BUFFER_SIZE] __attribute__((aligned(4)));	///<
static uint16_t rdm_checksum = 0;												///<
static uint32_t rdm_data_receive_end = 0;										///<
#if defined(RDM_CONTROLLER) || defined(LOGIC_ANALYZER)
static uint8_t rdm_disc_index = 0;												///<
#endif

static uint32_t dmx_packets_previous = 0;										///<
static struct _dmx_statistics dmx_statistics __attribute__((aligned(4)));		///<
static struct _total_statistics total_statistics __attribute__((aligned(4)));	///<

/**
 * @ingroup dmx
 *
 * @return
 */
const uint32_t dmx_get_output_period(void) {
	return dmx_output_period;
}

/**
 * @ingroup dmx
 *
 * @param period
 */
void dmx_set_output_period(const uint32_t period) {
	const uint32_t package_length_us = dmx_output_break_time + dmx_output_mab_time + (dmx_send_data_length * 44);

	if (period != 0) {
		if (period < package_length_us) {
			dmx_output_period = (uint32_t) MAX(DMX_TRANSMIT_BREAK_TO_BREAK_TIME_MIN, package_length_us + 4);
		} else {
			dmx_output_period = period;
		}

		dmx_output_fast_as_possible = false;
	} else {
		dmx_output_period = (uint32_t) MAX(DMX_TRANSMIT_BREAK_TO_BREAK_TIME_MIN, package_length_us + 4);

		dmx_output_fast_as_possible = true;
	}
}

/**
 * @ingroup dmx
 *
 * @return
 */
const uint16_t dmx_get_send_data_length(void) {
	return dmx_send_data_length;
}

/**
 * @ingroup dmx
 *
 * @param send_data_length
 */
void dmx_set_send_data_length(uint16_t send_data_length) {
	dmx_send_data_length = send_data_length;

	if (dmx_output_fast_as_possible != 0) {
		dmx_set_output_period(0);
	}
}

/**
 *
 * @return
 */
const struct _dmx_statistics *dmx_get_statistics(void) {
	return &dmx_statistics;
}

/**
 * @ingroup rdm
 *
 * @return
 */
const uint8_t *rdm_get_available(void)  {
	if (rdm_data_buffer_index_head == rdm_data_buffer_index_tail) {
		return NULL;
	} else {
		uint16_t saved_tail = rdm_data_buffer_index_tail;
		rdm_data_buffer_index_tail = (rdm_data_buffer_index_tail + 1) & RDM_DATA_BUFFER_INDEX_SIZE;
		return &rdm_data_buffer[saved_tail][0];
	}
}

/**
 * @ingroup rdm
 *
 * @return
 */
const uint8_t *rdm_get_current_data(void) {
	return &rdm_data_buffer[rdm_data_buffer_index_tail][0];
}

/**
 * @ingroup dmx
 *
 * @return
 */
const bool dmx_get_available(void) {
	return dmx_available;
}

/**
 * @ingroup dmx
 *
 * @param is_available
 */
void dmx_set_available_false(void) {
	dmx_available = false;
}


/**
 * @ingroup dmx
 *
 * The DMX data is changed when slots in packets is changed,
 * or when the data itself is changed.
 *
 * @return
 */
bool dmx_is_data_changed(void) {
	uint32_t i = 0;
	bool is_changed = false;

	if (dmx_statistics.slots_in_packet != dmx_slots_in_packet_previous) {
		dmx_slots_in_packet_previous = dmx_statistics.slots_in_packet;
		for (i = 1; i < DMX_DATA_BUFFER_SIZE; i++) {
			dmx_data_previous[i] = dmx_data[i];
		}
		return true;
	}

	for (i = 1; i < DMX_DATA_BUFFER_SIZE; i++) {
		if (dmx_data_previous[i] != dmx_data[i]) {
			is_changed = true;
			dmx_data_previous[i] = dmx_data[i];
		}
	}

	return is_changed;
}

/**
 * @ingroup dmx
 *
 * @return
 */
const _dmx_port_direction dmx_get_port_direction(void) {
	return dmx_port_direction;
}

/**
 * @ingroup dmx
 *
 * @return
 */
const uint32_t rdm_get_data_receive_end(void) {
	return rdm_data_receive_end;
}

/**
 * @ingroup dmx
 *
 * @return
 */
const uint32_t dmx_get_output_break_time(void) {
	return dmx_output_break_time;
}

/**
 * @ingroup dmx
 *
 * @param break_time
 */
void dmx_set_output_break_time(const uint32_t break_time) {
	dmx_output_break_time = MAX((uint32_t)DMX_TRANSMIT_BREAK_TIME_MIN, break_time);

	if (dmx_output_fast_as_possible) {
		dmx_set_output_period(0);
	}
}

/**
 * @ingroup dmx
 *
 * @return
 */
const uint32_t dmx_get_output_mab_time(void) {
	return dmx_output_mab_time;
}

/**
 * @ingroup dmx
 *
 * @param mab_time
 */
void dmx_set_output_mab_time(const uint32_t mab_time) {
	dmx_output_mab_time = MAX((uint32_t)DMX_TRANSMIT_MAB_TIME_MIN, mab_time);

	if (dmx_output_fast_as_possible) {
		dmx_set_output_period(0);
	}
}

/**
 * @ingroup dmx
 *
 */
void dmx_reset_total_statistics(void) {
	total_statistics.dmx_packets = (uint32_t) 0;
	total_statistics.rdm_packets = (uint32_t) 0;
}

/**
 * @ingroup dmx
 *
 * @return
 */
const struct _total_statistics *dmx_get_total_statistics(void) {
	return &total_statistics;
}

/**
 * @ingroup dmx
 *
 * Interrupt enable routine for PL011 receiving DMX data.
 *
 */
inline static void dmx_receive_fiq_init(void) {
	BCM2835_PL011->IMSC = PL011_IMSC_RXIM;
    BCM2835_IRQ->FIQ_CONTROL = (uint32_t)BCM2835_FIQ_ENABLE | (uint32_t)INTERRUPT_VC_UART;
}

/**
 * @ingroup dmx
 *
 * Interrupt handler for continues receiving DMX data.
 * \sa dmx_receive_fiq_init
 */
void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {
	dmb();

#ifdef LOGIC_ANALYZER
	bcm2835_gpio_set(GPIO_ANALYZER_CH1);
#endif

	dmx_fiq_micros_current = BCM2835_ST->CLO;

	if (BCM2835_PL011->DR & PL011_DR_BE) {
		dmx_receive_state = BREAK;
		dmx_statistics.break_to_break = dmx_fiq_micros_current - dmx_break_to_break_previous;
		dmx_break_to_break_previous = dmx_fiq_micros_current;
	} else {
		const uint8_t data = BCM2835_PL011->DR & 0xFF;

		switch (dmx_receive_state) {
#if defined(RDM_CONTROLLER) || defined(LOGIC_ANALYZER)
		case IDLE:
			if (data == 0xFE) {
				dmx_receive_state = RDMDISCFE;
				dmx_data_index = 0;
				rdm_data_buffer[rdm_data_buffer_index_head][dmx_data_index++] = 0xFE;
			}
			break;
#endif
		case BREAK:
			switch (data) {
			case DMX512_START_CODE:	// DMX data start code
				dmx_receive_state = DMXDATA;
				dmx_data[0] = DMX512_START_CODE;
				dmx_data_index = 1;
				total_statistics.dmx_packets = total_statistics.dmx_packets + 1;
#ifdef LOGIC_ANALYZER
				bcm2835_gpio_clr(GPIO_ANALYZER_CH2);	// BREAK
			    bcm2835_gpio_set(GPIO_ANALYZER_CH3);	// DMX DATA
#endif
				break;
			case E120_SC_RDM:		// RDM start code
				dmx_receive_state = RDMDATA;
				dmx_data_index = 0;
				rdm_data_buffer[rdm_data_buffer_index_head][dmx_data_index++] =	E120_SC_RDM;
				rdm_checksum = E120_SC_RDM;
				total_statistics.rdm_packets = total_statistics.rdm_packets + 1;
				break;
			default:
				dmx_receive_state = IDLE;
				break;
			}
			break;
		case DMXDATA:
			dmx_data[dmx_data_index++] = data;
			dmx_statistics.slot_to_slot = dmx_fiq_micros_current - dmx_fiq_micros_previous;
		    BCM2835_ST->C1 = dmx_fiq_micros_current + dmx_statistics.slot_to_slot + 2;
		    BCM2835_ST->CS = BCM2835_ST_CS_M1;
			if (dmx_data_index > DMX_UNIVERSE_SIZE)
			{
				dmx_receive_state = IDLE;
				dmx_available = true;
				dmx_statistics.slots_in_packet = DMX_UNIVERSE_SIZE;
#ifdef LOGIC_ANALYZER
				bcm2835_gpio_clr(GPIO_ANALYZER_CH3);	// DMX DATA
				bcm2835_gpio_set(GPIO_ANALYZER_CH4);	// IDLE
#endif
			}
			break;
		case RDMDATA:
			if (dmx_data_index > RDM_DATA_BUFFER_SIZE) {
				dmx_receive_state = IDLE;
#ifdef LOGIC_ANALYZER
				bcm2835_gpio_set(GPIO_ANALYZER_CH4);	// IDLE
#endif
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
			if ((rdm_checksum == 0) && (p->sub_start_code == E120_SC_SUB_MESSAGE))
			{
				rdm_data_buffer_index_head = (rdm_data_buffer_index_head + 1) & RDM_DATA_BUFFER_INDEX_SIZE;
				rdm_data_receive_end = BCM2835_ST->CLO;
			}
			dmx_receive_state = IDLE;
#ifdef LOGIC_ANALYZER
			bcm2835_gpio_set(GPIO_ANALYZER_CH4);	// IDLE
#endif
			break;
#if defined(RDM_CONTROLLER) || defined(LOGIC_ANALYZER)
		case RDMDISCFE:
			switch (data)
			{
			case 0xFE:
				rdm_data_buffer[rdm_data_buffer_index_head][dmx_data_index++] =	0xFE;
				break;
			case 0xAA:
				dmx_receive_state = RDMDISCEUID;
				rdm_disc_index = 0;
				rdm_data_buffer[rdm_data_buffer_index_head][dmx_data_index++] =	0xAA;
				break;
			default:
				dmx_receive_state = IDLE;
#ifdef LOGIC_ANALYZER
				bcm2835_gpio_set(GPIO_ANALYZER_CH4);	// IDLE
#endif
				break;
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
				rdm_data_buffer_index_head = (rdm_data_buffer_index_head + 1) & RDM_DATA_BUFFER_INDEX_SIZE;
				dmx_receive_state = IDLE;
				rdm_data_receive_end = BCM2835_ST->CLO;
#ifdef LOGIC_ANALYZER
				bcm2835_gpio_set(GPIO_ANALYZER_CH4);	// IDLE
#endif
			}
			break;
#endif
		default:
			break;
		}
	}

	dmx_fiq_micros_previous = dmx_fiq_micros_current;

#ifdef LOGIC_ANALYZER
	bcm2835_gpio_clr(GPIO_ANALYZER_CH1);
#endif

	dmb();
}

void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {
	dmb();

#ifdef LOGIC_ANALYZER
	bcm2835_gpio_set(GPIO_ANALYZER_CH5);
#endif

	const uint32_t clo = BCM2835_ST->CLO;

	if (BCM2835_ST->CS & BCM2835_ST_CS_M1) {
		BCM2835_ST->CS = BCM2835_ST_CS_M1;

		if (dmx_receive_state == DMXDATA) {
			if (clo > dmx_fiq_micros_current + dmx_statistics.slot_to_slot) {
				dmx_receive_state = IDLE;
				dmx_available = true;
				dmx_statistics.slots_in_packet = dmx_data_index - 1;
#ifdef LOGIC_ANALYZER
				bcm2835_gpio_clr(GPIO_ANALYZER_CH3);	// DMX DATA
				bcm2835_gpio_set(GPIO_ANALYZER_CH4);	// IDLE
#endif
			} else {
				BCM2835_ST->C1 = clo + 45;
			}
		}

		if (dmx_send_always) {
			switch (dmx_send_state) {
			case IDLE:
				dmx_send_state = BREAK;
				BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2 | PL011_LCRH_BRK;
				const uint32_t clo_break = BCM2835_ST->CLO;
				BCM2835_ST->C1 = clo_break + dmx_output_break_time;
				dmx_send_break_micros = clo_break;
				break;
			case BREAK:
				dmx_send_state = MAB;
				BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2;
				BCM2835_ST->C1 = BCM2835_ST->CLO + dmx_output_mab_time;
				break;
			case MAB:
				dmx_send_state = DMXDATA;
				uint16_t i = 0;
				for (i = 0; i < dmx_send_data_length; i++) {
					while (1) {
						if ((BCM2835_PL011->FR & PL011_FR_TXFF) == 0) {
							break;
						}
					}
					BCM2835_PL011->DR = dmx_data[i];
				}
				while (1) {
					if ((BCM2835_PL011->FR & PL011_FR_TXFF) == 0) {
						break;
					}
				}
				udelay(44);
				BCM2835_ST->C1 = dmx_output_period + dmx_send_break_micros;
				dmx_send_state = IDLE;
#ifdef LOGIC_ANALYZER
				bcm2835_gpio_set(GPIO_ANALYZER_CH4);	// IDLE
#endif
				break;
			default:
				break;
			}
		}
	}

	if (BCM2835_ST->CS & BCM2835_ST_CS_M3) {
		BCM2835_ST->CS = BCM2835_ST_CS_M3;
		BCM2835_ST->C3 = clo + (uint32_t)1000000;;
		dmx_statistics.updates_per_seconde = total_statistics.dmx_packets - dmx_packets_previous;
		dmx_packets_previous = total_statistics.dmx_packets;
	}

#ifdef LOGIC_ANALYZER
	bcm2835_gpio_clr(GPIO_ANALYZER_CH5);
#endif

	dmb();
}

/**
 * @ingroup dmx
 *
 * The DMX port direction is set based on \ref dmx_port_direction (\ref DMX_PORT_DIRECTION_OUTP or \ref DMX_PORT_DIRECTION_INP).
 *
 */
void dmx_start_data(void) {
	switch (dmx_port_direction) {
	case DMX_PORT_DIRECTION_OUTP:
		dmx_send_always = true;
		BCM2835_ST->C1 = BCM2835_ST->CLO + 4;
		BCM2835_ST->CS = BCM2835_ST_CS_M1;
		break;
	case DMX_PORT_DIRECTION_INP:
		dmx_receive_state = IDLE;
		__enable_fiq();
		break;
	default:
		break;
	}
}

/**
 * @ingroup dmx
 *
 */
void dmx_stop_data(void) {
	dmx_send_always = false;
	BCM2835_ST->C1 = BCM2835_ST->CLO;
	BCM2835_ST->CS = BCM2835_ST_CS_M1;
	__disable_fiq();
}

/**
 * @ingroup dmx
 *
 * @param port_direction
 * @param enable_data
 */
void dmx_set_port_direction(const _dmx_port_direction port_direction, const bool enable_data) {
	switch (port_direction) {
	case DMX_PORT_DIRECTION_IDLE:
		dmx_stop_data();
		bcm2835_gpio_clr(GPIO_DMX_DATA_DIRECTION);	// 0 = input, 1 = output
		dmx_port_direction = DMX_PORT_DIRECTION_IDLE;
		break;
	case DMX_PORT_DIRECTION_OUTP:
		dmx_stop_data();
		bcm2835_gpio_set(GPIO_DMX_DATA_DIRECTION);	// 0 = input, 1 = output
		dmx_port_direction = DMX_PORT_DIRECTION_OUTP;
		break;
	case DMX_PORT_DIRECTION_INP:
		dmx_stop_data();
		bcm2835_gpio_clr(GPIO_DMX_DATA_DIRECTION);	// 0 = input, 1 = output
		dmx_port_direction = DMX_PORT_DIRECTION_INP;
		break;
	default:
		dmx_port_direction = DMX_PORT_DIRECTION_IDLE;
		break;
	}

	if (enable_data) {
		dmx_start_data();
	}

	return;
}

/**
 * @ingroup dmx
 *
 * Configure PL011 for DMX512 transmission. Enable the UART.
 *
 */
static void pl011_init(void) {
	uint32_t value;
	(void) bcm2835_vc_set_clock_rate(BCM2835_VC_CLOCK_ID_UART, 4000000);// Set UART clock rate to 4000000 (4MHz)
	BCM2835_PL011->CR = 0;												// Disable everything
	value = BCM2835_GPIO->GPFSEL1;
	value &= ~(7 << 12);
	value |= BCM2835_GPIO_FSEL_ALT0 << 12;								// Pin 14 PL011_TXD
	value &= ~(7 << 15);
	value |= BCM2835_GPIO_FSEL_ALT0 << 15;								// Pin 15 PL011_RXD
	BCM2835_GPIO->GPFSEL1 = value;
	bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_08, BCM2835_GPIO_PUD_OFF);		// Disable pull-up/down
	bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_10, BCM2835_GPIO_PUD_OFF);		// Disable pull-up/down
	while ((BCM2835_PL011->FR & PL011_FR_BUSY) != 0) {
	}																	// Poll the "flags register" to wait for the UART to stop transmitting or receiving
	BCM2835_PL011->LCRH &= ~PL011_LCRH_FEN;								// Flush the transmit FIFO by marking FIFOs as disabled in the "line control register"
	BCM2835_PL011->ICR = 0x7FF;											// Clear all interrupt status
	BCM2835_PL011->IBRD = 1;											// UART Clock
	BCM2835_PL011->FBRD = 0;											// 4000000 (4MHz)
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2;			// Set 8, N, 2, FIFO disabled
	BCM2835_PL011->CR = 0x301;											// Enable UART
}

/**
 * @ingroup dmx
 *
 */
void dmx_init(void) {
	int i;
#ifdef LOGIC_ANALYZER
	bcm2835_gpio_fsel(GPIO_ANALYZER_CH1, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(GPIO_ANALYZER_CH2, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(GPIO_ANALYZER_CH3, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(GPIO_ANALYZER_CH4, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(GPIO_ANALYZER_CH5, BCM2835_GPIO_FSEL_OUTP);

	bcm2835_gpio_clr(GPIO_ANALYZER_CH1); // FIQ
	bcm2835_gpio_clr(GPIO_ANALYZER_CH2);	// BREAK
	bcm2835_gpio_clr(GPIO_ANALYZER_CH3); // DMX DATA
	bcm2835_gpio_set(GPIO_ANALYZER_CH4);	// IDLE
	bcm2835_gpio_clr(GPIO_ANALYZER_CH5); // IRQ
#endif

	for (i = 0; i < sizeof(dmx_data) / sizeof(dmx_data[0]); i++) {
		dmx_data[i] = (uint8_t)0;
		dmx_data_previous[i] = (uint8_t)0;
	}

	rdm_data_buffer_index_head = 0;
	rdm_data_buffer_index_tail = 0;

	dmx_receive_state = IDLE;
	dmx_send_always = false;
	dmx_available = false;

    BCM2835_ST->C3 = BCM2835_ST->CLO + (uint32_t)1000000;
    BCM2835_ST->CS = BCM2835_ST_CS_M1 + BCM2835_ST_CS_M3;
	BCM2835_IRQ->IRQ_ENABLE1 = BCM2835_TIMER1_IRQn + BCM2835_TIMER3_IRQn;
	__enable_irq();

	pl011_init();

	bcm2835_gpio_fsel(18, BCM2835_GPIO_FSEL_OUTP);
	dmx_receive_fiq_init();
	dmx_set_port_direction(DMX_PORT_DIRECTION_INP, 1);
}
