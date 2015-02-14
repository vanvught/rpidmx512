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

#include "bcm2835.h"
#include "bcm2835_gpio.h"
#include "bcm2835_vc.h"
#include "bcm2835_pl011.h"
#include "hardware.h"
#include "dmx_data.h"

uint8_t dmx_data[512];			///<
uint8_t dmx_data_refreshed;		///<

static uint8_t dmx_port_direction = DMX_PORT_DIRECTION_IDLE;

#ifdef DEBUG_ANALYZER
#define ANALYZER_CH1	RPI_V2_GPIO_P1_23	// CLK
#define ANALYZER_CH2	RPI_V2_GPIO_P1_21	// MISO
#define ANALYZER_CH3	RPI_V2_GPIO_P1_19	// MOSI
#define ANALYZER_CH4	RPI_V2_GPIO_P1_24	// CE0
#endif


/**
 * @ingroup dmx
 *
 */
static void pl011_dmx512_init(void) {
	// Set UART clock rate to 4000000 (4MHz)
	bcm2835_vc_set_clock_rate(BCM2835_VC_CLOCK_ID_UART, 4000000);
	//
	BCM2835_PL011->CR	= 0;										// Disable everything
	//
    uint32_t value = BCM2835_GPIO->GPFSEL1;
    value &= ~(7 << 12);
    value |= BCM2835_GPIO_FSEL_ALT0 << 12;							// Pin 14 PL011_TXD
    value &= ~(7 << 15);
    value |= BCM2835_GPIO_FSEL_ALT0 << 15;							// Pin 15 PL011_RXD
    BCM2835_GPIO->GPFSEL1 = value;
	//
	bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_08, BCM2835_GPIO_PUD_OFF);	// Disable pull-up/down
	bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_10, BCM2835_GPIO_PUD_OFF);	// Disable pull-up/down
	//
	while (BCM2835_PL011 ->FR & PL011_FR_BUSY ) {}					// Poll the "flags register" to wait for the UART to stop transmitting or receiving
	//
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
 */
void fiq_init(void) {
	BCM2835_PL011->IMSC = PL011_IMSC_RXIM;
    BCM2835_IRQ->FIQ_CONTROL = BCM2835_FIQ_ENABLE | INTERRUPT_VC_UART;
}

///< State of receiving DMX Bytes
typedef enum {
  IDLE,		///<
  BREAK,	///<
  MAB,		///<
  DATA		///<
} _dmx_state;

static uint8_t dmx_receive_state = IDLE;		///< Current state of the DMX transmission
static uint16_t dmx_receive_data_index = 0;		///<
static uint8_t dmx_send_state = IDLE;			///<

/**
 * @ingroup dmx
 *
 */
void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {
	dmb();
#ifdef DEBUG_ANALYZER
	bcm2835_gpio_set(ANALYZER_CH1);
#endif
	if (BCM2835_PL011 ->DR & PL011_DR_BE ) {
#ifdef DEBUG_ANALYZER
		bcm2835_gpio_set(ANALYZER_CH2);		// BREAK
		bcm2835_gpio_clr(ANALYZER_CH3);		// DATA
		bcm2835_gpio_clr(ANALYZER_CH4);		// IDLE
#endif
		dmx_receive_state = BREAK;
	}  else if (dmx_receive_state == BREAK) {
		if ((BCM2835_PL011 ->DR & 0xFF) == 0x00) {		// "Start Code"
#ifdef DEBUG_ANALYZER
			bcm2835_gpio_clr(ANALYZER_CH2);	// BREAK
			bcm2835_gpio_set(ANALYZER_CH3);	// DATA
			bcm2835_gpio_clr(ANALYZER_CH4);	// IDLE
#endif
			dmx_receive_state = DATA;
			dmx_receive_data_index = 0;
		} else {							// Ignore all other; Text (0x17), System Information (0xCF), RDM (0xCC), etc.
#ifdef DEBUG_ANALYZER
			bcm2835_gpio_clr(ANALYZER_CH2);	// BREAK
			bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
			bcm2835_gpio_set(ANALYZER_CH4);	// IDLE
#endif
			dmx_receive_state = IDLE;
		}
	} else if (dmx_receive_state == DATA) {
		dmx_data[dmx_receive_data_index] = (BCM2835_PL011 ->DR & 0xFF);
		dmx_receive_data_index++;
		dmx_data_refreshed = 0xFF;			// Set all bits
		if (dmx_receive_data_index >= 512){
#ifdef DEBUG_ANALYZER
			bcm2835_gpio_clr(ANALYZER_CH2);	// BREAK
			bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
			bcm2835_gpio_set(ANALYZER_CH4);	// IDLE
#endif
			dmx_receive_state = IDLE;
		}
	}
#ifdef DEBUG_ANALYZER
	bcm2835_gpio_clr(ANALYZER_CH1);
#endif
	dmb();
}

void irq_init(void)
{
	BCM2835_ST->C1 = BCM2835_ST->CLO + 4;			// 4us
	BCM2835_ST->CS = BCM2835_ST_CS_M1;
	BCM2835_IRQ->IRQ_ENABLE1 = BCM2835_TIMER1_IRQn;
}

void __attribute__((interrupt("IRQ"))) c_irq_handler(void)
{
	dmb();
	BCM2835_ST->CS = BCM2835_ST_CS_M1;
#ifdef DEBUG_ANALYZER
	bcm2835_gpio_set(ANALYZER_CH1);
#endif

	switch (dmx_send_state)
	{
	case IDLE:
		dmx_send_state = BREAK;
		BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2 | PL011_LCRH_BRK;
		BCM2835_ST->C1 = BCM2835_ST->CLO + 88;
#ifdef DEBUG_ANALYZER
		bcm2835_gpio_set(ANALYZER_CH2); // BREAK
		bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
		bcm2835_gpio_clr(ANALYZER_CH4); // MAB
#endif
		break;
	case BREAK:
		dmx_send_state = MAB;
		BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2;
		BCM2835_ST->C1 = BCM2835_ST->CLO + 8;
#ifdef DEBUG_ANALYZER
		bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
		bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
		bcm2835_gpio_set(ANALYZER_CH4); // MAB
#endif
		break;
	case MAB:
		dmx_send_state = DATA;
#ifdef DEBUG_ANALYZER
		bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
		bcm2835_gpio_set(ANALYZER_CH3);	// DATA
		bcm2835_gpio_clr(ANALYZER_CH4); // MAB
#endif
		while (1)
		{
			if ((BCM2835_PL011->FR & 0x20) == 0)
				break;
		}
		BCM2835_PL011->DR = 0;
		int i = 0;
		for (i = 0; i < 512; i++)
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
		BCM2835_ST->C1 = BCM2835_ST->CLO + 100;
		dmx_send_state = IDLE;
#ifdef DEBUG_ANALYZER
		bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
		bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
		bcm2835_gpio_clr(ANALYZER_CH4); // MAB
#endif
		break;
	default:
		BCM2835_ST->C1 = BCM2835_ST->CLO + 44;
		break;
	}
#ifdef DEBUG_ANALYZER
	bcm2835_gpio_clr(ANALYZER_CH1);
#endif
	dmb();
}

/**
 *
 * @param port_direction
 */
void dmx_port_direction_set(const uint8_t port_direction)
{
	switch (port_direction)
	{
	case DMX_PORT_DIRECTION_IDLE:
		__disable_fiq();
		__disable_irq();
		bcm2835_gpio_clr(18);	// GPIO18, data direction, 0 = input, 1 = output //TODO
		bcm2835_gpio_fsel(18, BCM2835_GPIO_FSEL_OUTP);
		dmx_port_direction = DMX_PORT_DIRECTION_IDLE;
		break;
	case DMX_PORT_DIRECTION_OUTP:
		__disable_fiq();
		__disable_irq();
		bcm2835_gpio_set(18);	// GPIO18, data direction, 0 = input, 1 = output //TODO
		bcm2835_gpio_fsel(18, BCM2835_GPIO_FSEL_OUTP);
		dmx_port_direction = DMX_PORT_DIRECTION_OUTP;
		irq_init();
		__enable_irq();
		break;
	case DMX_PORT_DIRECTION_INP:
		__disable_fiq();
		__disable_irq();
		bcm2835_gpio_clr(18);	// GPIO18, data direction, 0 = input, 1 = output //TODO
		bcm2835_gpio_fsel(18, BCM2835_GPIO_FSEL_OUTP);
		dmx_port_direction = DMX_PORT_DIRECTION_INP;
		fiq_init();
		__enable_fiq();
		break;
	default:
		dmx_port_direction = DMX_PORT_DIRECTION_IDLE;
		break;
	}

	return;
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
 * @ingroup dmx
 *
 */
void dmx_init(void)
{
#ifdef DEBUG_ANALYZER
	bcm2835_gpio_fsel(ANALYZER_CH1, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(ANALYZER_CH2, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(ANALYZER_CH3, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(ANALYZER_CH4, BCM2835_GPIO_FSEL_OUTP);

	bcm2835_gpio_clr(ANALYZER_CH1);
	bcm2835_gpio_clr(ANALYZER_CH2);
	bcm2835_gpio_clr(ANALYZER_CH3);
	bcm2835_gpio_set(ANALYZER_CH4);
#endif
	int i = 0;
	for (i = 0; i < sizeof(dmx_data) / sizeof(uint8_t); i++)
		dmx_data[i] = 0;

	pl011_dmx512_init();
	dmx_port_direction_set(DMX_PORT_DIRECTION_INP);
}
