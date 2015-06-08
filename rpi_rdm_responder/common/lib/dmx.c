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
#include <string.h>

#include "bcm2835.h"
#include "bcm2835_gpio.h"
#include "bcm2835_vc.h"
#include "bcm2835_pl011.h"
#include "hardware.h"
#include "irq_led.h"
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

uint8_t dmx_data[DMX_DATA_BUFFER_SIZE];									///<
static uint8_t dmx_receive_state = IDLE;								///< Current state of DMX receive
static uint16_t dmx_data_index = 0;										///<
static uint8_t dmx_data_previous[DMX_DATA_BUFFER_SIZE];					///<
static uint8_t dmx_available = FALSE;									///<
static uint32_t dmx_output_break_time = DMX_TRANSMIT_BREAK_TIME_MIN;	///<
static uint32_t dmx_output_mab_time = DMX_TRANSMIT_MAB_TIME_MIN;		///<
static uint32_t dmx_output_period = DMX_TRANSMIT_REFRESH_DEFAULT;		///<
static uint8_t dmx_output_fast_as_possible = FALSE;						///<
static uint16_t dmx_send_data_length = DMX_UNIVERSE_SIZE + 1;			///< SC + UNIVERSE SIZE
static uint8_t dmx_port_direction = DMX_PORT_DIRECTION_INP;				///<
static volatile uint32_t dmx_fiq_micros_current = 0;					///< Timestamp FIQ
static volatile uint32_t dmx_fiq_micros_previous = 0;					///< Timestamp previous FIQ
static volatile uint32_t dmx_slot_to_slot = 0;							///<
static volatile uint32_t dmx_break_to_break = 0;						///<
static volatile uint32_t dmx_break_to_break_previous = 0;				///<
static volatile uint32_t dmx_slots_in_packet = 0;						///<
static uint32_t dmx_slots_in_packet_previous = 0;						///<
static volatile uint8_t dmx_send_state = IDLE;							///<
static volatile uint8_t dmx_send_always = FALSE;						///<
static volatile uint32_t dmx_send_break_micros = 0;						///<

#define RDM_DATA_BUFFER_INDEX_SIZE 	0x0F												///<
static uint16_t rdm_data_buffer_index_head = 0;											///<
static uint16_t rdm_data_buffer_index_tail = 0;											///<
static uint8_t rdm_data_buffer[RDM_DATA_BUFFER_INDEX_SIZE + 1][RDM_DATA_BUFFER_SIZE];	///<
static uint16_t rdm_checksum = 0;														///<
static uint32_t rdm_data_receive_end = 0;												///<
#ifdef RDM_CONTROLLER
static uint8_t rdm_disc_index = 0;														///<
#endif
static struct _dmx_statistics dmx_statistics;											///<
static struct _total_statistics total_statistics;										///<

/*
 * GETTERS / SETTERS
 */

/**
 * @ingroup dmx
 *
 * @return
 */
const uint32_t dmx_get_output_period(void)
{
	return dmx_output_period;
}

/**
 * @ingroup dmx
 *
 * @param period
 */
void dmx_set_output_period(const uint32_t period)
{
	const uint32_t package_length_us = dmx_output_break_time + dmx_output_mab_time + (dmx_send_data_length * 44);

	if (period)
	{
		if (period < package_length_us)
		{
			dmx_output_period = MAX(DMX_TRANSMIT_BREAK_TO_BREAK_TIME_MIN, package_length_us + 4);
		}
		else
		{
			dmx_output_period = period;
		}

		dmx_output_fast_as_possible = FALSE;
	}
	else
	{
		dmx_output_period = MAX(DMX_TRANSMIT_BREAK_TO_BREAK_TIME_MIN, package_length_us + 4);

		dmx_output_fast_as_possible = TRUE;
	}
}

/**
 * @ingroup dmx
 *
 * @return
 */
const uint16_t dmx_get_send_data_length(void)
{
	return dmx_send_data_length;
}

/**
 * @ingroup dmx
 *
 * @param send_data_length
 */
void dmx_set_send_data_length(uint16_t send_data_length)
{
	dmx_send_data_length = send_data_length;

	if(dmx_output_fast_as_possible)
		dmx_set_output_period(0);
}

/**
 * @ingroup dmx
 *
 * @return
 */
const uint32_t dmx_get_slot_to_slot(void)
{
	return dmx_slot_to_slot;
}

/**
 * @ingroup dmx
 *
 * @return
 */
const uint32_t dmx_get_slots_in_packet(void)
{
	return dmx_slots_in_packet;
}

/**
 * @ingroup dmx
 *
 * @return
 */
const uint32_t dmx_get_break_to_break(void)
{
	return dmx_break_to_break;
}

/**
 * @ingroup rdm
 *
 * @return
 */
const uint8_t *rdm_get_available(void)
{
	if (rdm_data_buffer_index_head == rdm_data_buffer_index_tail)
		return NULL;
	else
	{
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
const uint8_t *rdm_get_current_data(void)
{
	return &rdm_data_buffer[rdm_data_buffer_index_tail][0];
}

/**
 * @ingroup dmx
 *
 * @return
 */
const uint8_t dmx_get_available(void)
{
	return dmx_available;
}

/**
 * @ingroup dmx
 *
 * @param is_available
 */
void dmx_available_set(const uint8_t is_available)
{
	dmx_available = is_available;
}


/**
 * @ingroup dmx
 *
 * The DMX data is changed when slots in packets is changed,
 * or when the data itself is changed.
 *
 * @return
 */
uint8_t dmx_data_is_changed(void)
{
	uint32_t i = 0;
	uint8_t is_changed = FALSE;

	if (dmx_slots_in_packet != dmx_slots_in_packet_previous) {
		dmx_slots_in_packet_previous = dmx_slots_in_packet;
		for (i = 1; i < DMX_DATA_BUFFER_SIZE; i++)
		{
			dmx_data_previous[i] = dmx_data[i];
		}
		return TRUE;
	}

	for (i = 1; i < DMX_DATA_BUFFER_SIZE; i++)
	{
		if (dmx_data_previous[i] != dmx_data[i])
		{
			is_changed = TRUE;
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
const uint8_t dmx_get_port_direction(void)
{
	return dmx_port_direction;
}

/**
 * @ingroup dmx
 *
 * @return
 */
const uint32_t rdm_data_receive_end_get(void)
{
	return rdm_data_receive_end;
}

/**
 * @ingroup dmx
 *
 * @return
 */
const uint32_t dmx_get_output_break_time(void)
{
	return dmx_output_break_time;
}

/**
 * @ingroup dmx
 *
 * @param break_time
 */
void dmx_set_output_break_time(const uint32_t break_time)
{
	dmx_output_break_time = MAX(DMX_TRANSMIT_BREAK_TIME_MIN, break_time);

	if(dmx_output_fast_as_possible)
		dmx_set_output_period(0);
}

/**
 * @ingroup dmx
 *
 * @return
 */
const uint32_t dmx_get_output_mab_time(void)
{
	return dmx_output_mab_time;
}

/**
 * @ingroup dmx
 *
 * @param mab_time
 */
void dmx_set_output_mab_time(const uint32_t mab_time)
{
	dmx_output_mab_time = MAX(DMX_TRANSMIT_MAB_TIME_MIN, mab_time);

	if(dmx_output_fast_as_possible)
		dmx_set_output_period(0);
}

/**
 * @ingroup dmx
 *
 */
void dmx_statistics_reset(void)
{
	memset(&dmx_statistics, 0, sizeof(struct _dmx_statistics));
}

/**
 * @ingroup dmx
 *
 */
void total_statistics_reset(void)
{
	memset(&total_statistics, 0, sizeof(struct _total_statistics));
}

/**
 * @ingroup dmx
 *
 * @return
 */
const struct _total_statistics *total_statistics_get(void)
{
	return &total_statistics;
}

/**
 * @ingroup dmx
 *
 * Configure PL011 for DMX512 transmission. Enable the UART.
 *
 */
static void pl011_dmx512_init(void) {
	bcm2835_vc_set_clock_rate(BCM2835_VC_CLOCK_ID_UART, 4000000);	// Set UART clock rate to 4000000 (4MHz)
	BCM2835_PL011->CR	= 0;										// Disable everything
    uint32_t value = BCM2835_GPIO->GPFSEL1;
    value &= ~(7 << 12);
    value |= BCM2835_GPIO_FSEL_ALT0 << 12;							// Pin 14 PL011_TXD
    value &= ~(7 << 15);
    value |= BCM2835_GPIO_FSEL_ALT0 << 15;							// Pin 15 PL011_RXD
    BCM2835_GPIO->GPFSEL1 = value;
	bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_08, BCM2835_GPIO_PUD_OFF);	// Disable pull-up/down
	bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_10, BCM2835_GPIO_PUD_OFF);	// Disable pull-up/down
	while (BCM2835_PL011 ->FR & PL011_FR_BUSY ) {}					// Poll the "flags register" to wait for the UART to stop transmitting or receiving
	BCM2835_PL011->LCRH &= ~PL011_LCRH_FEN;							// Flush the transmit FIFO by marking FIFOs as disabled in the "line control register"
	BCM2835_PL011->ICR 	= 0x7FF;									// Clear all interrupt status
	BCM2835_PL011->IBRD = 1;										// UART Clock
	BCM2835_PL011->FBRD = 0;										// 4000000 (4MHz)
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2 ;		// Set 8, N, 2, FIFO disabled
	BCM2835_PL011->CR 	= 0x301;									// Enable UART
}

/**
 * @ingroup dmx
 *
 * Interrupt enable routine for PL011 receiving DMX data.
 *
 */
inline static void dmx_receive_fiq_init(void) {
	BCM2835_PL011->IMSC = PL011_IMSC_RXIM;
    BCM2835_IRQ->FIQ_CONTROL = BCM2835_FIQ_ENABLE | INTERRUPT_VC_UART;
}

/**
 * @ingroup dmx
 *
 * Interrupt handler for continues receiving DMX data.
 * \sa dmx_receive_fiq_init
 */
void __attribute__((interrupt("FIQ"))) c_fiq_handler(void)
{
	dmb();

	dmx_fiq_micros_current = BCM2835_ST->CLO;

	if (BCM2835_PL011->DR & PL011_DR_BE)
	{
		dmx_receive_state = BREAK;
		dmx_break_to_break = dmx_fiq_micros_current - dmx_break_to_break_previous;
		dmx_break_to_break_previous = dmx_fiq_micros_current;
	}
	else
	{
		const uint8_t data = BCM2835_PL011->DR & 0xFF;

		switch (dmx_receive_state)
		{
#ifdef RDM_CONTROLLER
		case IDLE:
			if (data == 0xFE)
			{
				dmx_receive_state = RDMDISCFE;
				dmx_data_index = 0;
				rdm_data_buffer[rdm_data_buffer_index_head][dmx_data_index++] = data; // 0xFE;
			}
			break;
#endif
		case BREAK:
			switch (data)
			{
			case DMX512_START_CODE:	// DMX data start code
				dmx_receive_state = DMXDATA;
				dmx_data[0] = DMX512_START_CODE;
				dmx_data_index = 1;
				total_statistics.dmx_packets = total_statistics.dmx_packets + 1;
			    //BCM2835_ST->C1 = dmx_fiq_micros_current + 45;
			    //BCM2835_ST->CS = BCM2835_ST_CS_M1;
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
			dmx_slot_to_slot = dmx_fiq_micros_current - dmx_fiq_micros_previous;
		    BCM2835_ST->C1 = dmx_fiq_micros_current + dmx_slot_to_slot + 2;
		    BCM2835_ST->CS = BCM2835_ST_CS_M1;
			if (dmx_data_index > DMX_UNIVERSE_SIZE)
			{
				dmx_receive_state = IDLE;
				dmx_available = TRUE;
				dmx_slots_in_packet = DMX_UNIVERSE_SIZE;
			}
			break;
		case RDMDATA:
			if (dmx_data_index > RDM_DATA_BUFFER_SIZE)
			{
				dmx_receive_state = IDLE;
			} else
			{
				rdm_data_buffer[rdm_data_buffer_index_head][dmx_data_index++] = data;
				rdm_checksum += data;

				const struct _rdm_command *p = (struct _rdm_command *)(&rdm_data_buffer[rdm_data_buffer_index_head][0]);
				if (dmx_data_index == p->message_length)
				{
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
				rdm_data_receive_end = hardware_micros();
			}
			dmx_receive_state = IDLE;
			break;
#ifdef RDM_CONTROLLER
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
				break;
			}
			break;
		case RDMDISCEUID:
			rdm_data_buffer[rdm_data_buffer_index_head][dmx_data_index++] = data;
			rdm_disc_index++;
			if (rdm_disc_index == 2 * RDM_UID_SIZE)
			{
				dmx_receive_state = RDMDISCECS;
				rdm_disc_index = 0;
			}
			break;
		case RDMDISCECS:
			rdm_data_buffer[rdm_data_buffer_index_head][dmx_data_index++] = data;
			rdm_disc_index++;
			if (rdm_disc_index == 4)
			{
				rdm_data_buffer_index_head = (rdm_data_buffer_index_head + 1) & RDM_DATA_BUFFER_INDEX_SIZE;
				dmx_receive_state = IDLE;
				rdm_data_receive_end = hardware_micros();
			}
			break;
#endif
		default:
			break;
		}
	}

	dmx_fiq_micros_previous = dmx_fiq_micros_current;

	dmb();
}

/**
 * @ingroup dmx
 *
 * The DMX port direction is set based on \ref dmx_port_direction (\ref DMX_PORT_DIRECTION_OUTP or \ref DMX_PORT_DIRECTION_INP).
 *
 */
void dmx_data_start(void)
{
	switch (dmx_port_direction)
	{
	case DMX_PORT_DIRECTION_OUTP:
		dmx_send_always = TRUE;
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
void dmx_data_stop(void)
{
	dmx_send_always = FALSE;
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
void dmx_port_direction_set(const uint8_t port_direction, const uint8_t enable_data)
{
	switch (port_direction)
	{
	case DMX_PORT_DIRECTION_IDLE:
		dmx_data_stop();
		bcm2835_gpio_clr(18);	// GPIO18, data direction, 0 = input, 1 = output
		dmx_port_direction = DMX_PORT_DIRECTION_IDLE;
		break;
	case DMX_PORT_DIRECTION_OUTP:
		dmx_data_stop();
		bcm2835_gpio_set(18);	// GPIO18, data direction, 0 = input, 1 = output
		dmx_port_direction = DMX_PORT_DIRECTION_OUTP;
		break;
	case DMX_PORT_DIRECTION_INP:
		dmx_data_stop();
		bcm2835_gpio_clr(18);	// GPIO18, data direction, 0 = input, 1 = output
		dmx_port_direction = DMX_PORT_DIRECTION_INP;
		break;
	default:
		dmx_port_direction = DMX_PORT_DIRECTION_IDLE;
		break;
	}

	if (enable_data)
	{
		dmx_data_start();
	}

	return;
}

/**
 * @ingroup dmx
 *
 */
void dmx_init(void)
{
	int i = 0;
	for (i = 0; i < sizeof(dmx_data) / sizeof(uint8_t); i++)
	{
		dmx_data[i] = 0;
		dmx_data_previous[i] = 0;
	}

	rdm_data_buffer_index_head = 0;
	rdm_data_buffer_index_tail = 0;

	dmx_receive_state = IDLE;
	dmx_send_always = FALSE;
	dmx_available = FALSE;

    BCM2835_ST->C3 = BCM2835_ST->CLO + ticks_per_second_get();
    BCM2835_ST->CS = BCM2835_ST_CS_M1 + BCM2835_ST_CS_M3;
	BCM2835_IRQ->IRQ_ENABLE1 = BCM2835_TIMER1_IRQn + BCM2835_TIMER3_IRQn;
	__enable_irq();

	pl011_dmx512_init();

	bcm2835_gpio_fsel(18, BCM2835_GPIO_FSEL_OUTP);
	dmx_receive_fiq_init();
	dmx_port_direction_set(DMX_PORT_DIRECTION_INP, 1);
}

static unsigned int irq_counter;

void __attribute__((interrupt("IRQ"))) c_irq_handler(void)
{
	dmb();

	const uint32_t clo = BCM2835_ST->CLO;

	if (BCM2835_ST->CS & BCM2835_ST_CS_M1)
	{
		BCM2835_ST->CS = BCM2835_ST_CS_M1;

		if (dmx_receive_state == DMXDATA)
		{
			if (clo > dmx_fiq_micros_current + dmx_slot_to_slot)
			{
				dmx_receive_state = IDLE;
				dmx_available = TRUE;
				dmx_slots_in_packet = dmx_data_index - 1;
			}
			else
				BCM2835_ST->C1 = clo + 45;
		}

		if (dmx_send_always)
		{
			switch (dmx_send_state)
			{
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
				for (i = 0; i < dmx_send_data_length; i++)
				{
					while (1)
					{
						if ((BCM2835_PL011->FR & 0x20) == 0)
							break;
					}
					BCM2835_PL011->DR = dmx_data[i];
				}
				while (1)
				{
					if ((BCM2835_PL011->FR & 0x20) == 0)
						break;
				}
				udelay(44); //TODO remove
				BCM2835_ST->C1 = dmx_output_period + dmx_send_break_micros;
				dmx_send_state = IDLE;
				break;
			default:
				break;
			}
		}
	}

	if (BCM2835_ST->CS & BCM2835_ST_CS_M3)
	{
		BCM2835_ST->CS = BCM2835_ST_CS_M3;
		BCM2835_ST->C3 = clo + ticks_per_second_get();
		hardware_led_set(irq_counter++ & 0x01);
	}

	dmb();
}
