/**
 * @file dmx_data.c
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

#include "bcm2835.h"
#include "bcm2835_gpio.h"
#include "bcm2835_vc.h"
#include "bcm2835_pl011.h"
#include "hardware.h"

#include "util.h"
#include "dmx.h"
#include "rdm.h"
#include "rdm_e120.h"

///< State of receiving DMX Bytes
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

uint8_t dmx_data[DMX_DATA_BUFFER_SIZE];			///<
static uint8_t dmx_receive_state = IDLE;		///< Current state of DMX receive
static uint16_t dmx_data_index = 0;				///<
static uint8_t dmx_available = FALSE;			///<

uint8_t rdm_data[RDM_DATA_BUFFER_SIZE];							///<
static uint16_t rdm_checksum = 0;				///<
static uint64_t rdm_data_receive_end = 0;		///<
static uint8_t rdm_available = FALSE;			///<
static uint8_t rdm_disc_index = 0;				///<

static uint8_t dmx_port_direction = DMX_PORT_DIRECTION_IDLE;	///<

/**
 * IRQ Handler is not used
 */
void __attribute__((interrupt("IRQ"))) c_irq_handler(void)
{
}

/*
 * GETTERS / SETTERS
 */

/**
 * @ingroup rdm
 *
 * @return
 */
uint8_t rdm_available_get(void)
{
	return rdm_available;
}

/**
 * @ingroup rdm
 *
 * @param is_available
 */
void rdm_available_set(uint8_t is_available)
{
	rdm_available = is_available;
}

/**
 * @ingroup dmx
 *
 * @return
 */
uint8_t dmx_available_get(void)
{
	return dmx_available;
}

/**
 * @ingroup dmx
 *
 * @param is_available
 */
void dmx_available_set(uint8_t is_available)
{
	dmx_available = is_available;
}

/**
 *
 * @return
 */
uint8_t dmx_port_direction_get(void)
{
	return dmx_port_direction;
}

/**
 *
 * @return
 */
uint64_t rdm_data_receive_end_get(void)
{
	return rdm_data_receive_end;
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
static void dmx_receive_fiq_init(void) {
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

	if (BCM2835_PL011->DR & PL011_DR_BE)
	{
		dmx_receive_state = BREAK;
	}
	else if (dmx_receive_state == IDLE)
	{
		uint8_t data = BCM2835_PL011->DR & 0xFF;
		if (data == 0xFE)
		{
			dmx_receive_state = RDMDISCFE;
			dmx_data_index = 0;
			rdm_data[dmx_data_index++] = 0xFE;
		}
	}
	else if (dmx_receive_state == BREAK)
	{
		uint8_t data = BCM2835_PL011->DR & 0xFF;
		if (data == 0x00)			// DMX data start code
		{
			dmx_receive_state = DMXDATA;
			dmx_data_index = 0;
		}
		else if (data == E120_SC_RDM)		// RDM start code
		{
			dmx_receive_state = RDMDATA;
			dmx_data_index = 0;
			rdm_data[dmx_data_index++] = E120_SC_RDM;
			rdm_checksum = E120_SC_RDM;
		}
		else
		{
			dmx_receive_state = IDLE;
		}
	}
	else if (dmx_receive_state == DMXDATA)
	{
		dmx_data[dmx_data_index++] = (BCM2835_PL011->DR & 0xFF);
		if (dmx_data_index >= 512)
		{
			dmx_receive_state = IDLE;
			dmx_available = TRUE;
		}
	}
	else if (dmx_receive_state == RDMDATA)
	{
		if (dmx_data_index > (sizeof(rdm_data) / sizeof(uint8_t)))
		{
			dmx_receive_state = IDLE;
		} else
		{
			uint8_t data = (BCM2835_PL011->DR & 0xFF);
			rdm_data[dmx_data_index++] = data;
			rdm_checksum += data;

			struct _rdm_command *p = (struct _rdm_command *)(rdm_data);
			if (dmx_data_index == p->message_length)
			{
				dmx_receive_state =	CHECKSUMH;
			}
		}
	}
	else if (dmx_receive_state == CHECKSUMH)
	{
		uint8_t data = (BCM2835_PL011->DR & 0xFF);
		rdm_data[dmx_data_index++] =  data;
		rdm_checksum -= data << 8;
		dmx_receive_state = CHECKSUML;
	}
	else if (dmx_receive_state == CHECKSUML)
	{
		uint8_t data = (BCM2835_PL011->DR & 0xFF);
		rdm_data[dmx_data_index++] = data;
		rdm_checksum -= data;

		struct _rdm_command *p = (struct _rdm_command *) (rdm_data);
		if ((rdm_checksum == 0) && (p->sub_start_code == E120_SC_SUB_MESSAGE))
		{
			rdm_available = TRUE;
			rdm_data_receive_end = bcm2835_st_read();
		}

		dmx_receive_state = IDLE;
	}
	else if (dmx_receive_state == RDMDISCFE)
	{
		uint8_t data = (BCM2835_PL011->DR & 0xFF);
		if (data == 0xFE)
		{
			rdm_data[dmx_data_index++] = 0xFE;
		}
		else if (data == 0xAA)
		{
			dmx_receive_state = RDMDISCEUID;
			rdm_disc_index = 0;
			rdm_data[dmx_data_index++] = 0xAA;
		}
	}
	else if (dmx_receive_state == RDMDISCEUID)
	{
		rdm_data[dmx_data_index++] = (BCM2835_PL011->DR & 0xFF);
		rdm_disc_index++;
		if (rdm_disc_index == 2 * RDM_UID_SIZE)
		{
			dmx_receive_state = RDMDISCECS;
			rdm_disc_index = 0;
		}
	}
	else if (dmx_receive_state == RDMDISCECS)
	{
		rdm_data[dmx_data_index++] = (BCM2835_PL011->DR & 0xFF);
		rdm_disc_index++;
		if (rdm_disc_index == 4)
		{
			rdm_available = TRUE;
			dmx_receive_state = IDLE;
			rdm_data_receive_end = bcm2835_st_read();
		}
	}

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
		break;
	case DMX_PORT_DIRECTION_INP:
		dmx_receive_fiq_init();
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
	__disable_fiq();
}

/**
 * @ingroup dmx
 *
 * @param port_direction
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
 *
 * @param data
 * @param data_length
 */
void dmx_data_send(const uint8_t *data, const uint16_t data_length)
{
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2 | PL011_LCRH_BRK;
	udelay(RDM_TRANSMIT_BREAK_TIME);	// Break Time

	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2;
	udelay(RDM_TRANSMIT_MAB_TIME);	// Mark After Break

	while (1)
	{
		if ((BCM2835_PL011->FR & 0x20) == 0)
			break;
	}
	BCM2835_PL011->DR = 0x00;

	uint16_t i = 0;
	for (i = 0; i < data_length; i++)
	{
		while (1)
		{
			if ((BCM2835_PL011->FR & 0x20) == 0)
				break;
		}
		BCM2835_PL011->DR = data[i];
	}
	while (1)
	{
		if ((BCM2835_PL011->FR & 0x20) == 0)
			break;
	}
	udelay(44);
}


/**
 * @ingroup rdm
 *
 *
 * @param data
 * @param data_length
 */
void rdm_data_send(const uint8_t *data, const uint16_t data_length)
{
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2 | PL011_LCRH_BRK;
	udelay(RDM_TRANSMIT_BREAK_TIME);	// Break Time

	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2;
	udelay(RDM_TRANSMIT_MAB_TIME);	// Mark After Break

	uint16_t i = 0;
	for (i = 0; i < data_length; i++)
	{
		while (1)
		{
			if ((BCM2835_PL011->FR & 0x20) == 0)
				break;
		}
		BCM2835_PL011->DR = data[i];
	}
	while (1)
	{
		if ((BCM2835_PL011->FR & 0x20) == 0)
			break;
	}
	udelay(44);
}

/**
 * @ingroup dmx
 *
 */
void dmx_init(void)
{
	int i = 0;
	for (i = 0; i < sizeof(dmx_data) / sizeof(uint8_t); i++)
		dmx_data[i] = 0;

	pl011_dmx512_init();
	bcm2835_gpio_fsel(18, BCM2835_GPIO_FSEL_OUTP);
	dmx_port_direction_set(DMX_PORT_DIRECTION_INP, 1);
}
