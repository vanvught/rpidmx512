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
#include "irq_led.h"

///< State of receiving DMX Bytes
typedef enum {
  IDLE,		///<
  BREAK,	///<
  DATA		///<
} _dmx_receive_state;

uint8_t dmx_data[512];			///<
uint8_t volatile dmx_data_refreshed;		///<

static volatile uint8_t dmx_receive_state = IDLE;	///< Current state of the DMX transmission
static volatile uint16_t dmx_data_index = 0;		///<
static volatile uint32_t dmx_fiq_micros_current = 0;///<
static volatile uint32_t dmx_fiq_micros_previous = 0;///<
static volatile uint32_t dmx_slot_to_slot = 0;		///<
static volatile uint32_t dmx_slots_in_packet = 0;	///<

#ifdef DEBUG_ANALYZER
#define ANALYZER_CH1	RPI_V2_GPIO_P1_23	// CLK
#define ANALYZER_CH2	RPI_V2_GPIO_P1_21	// MISO
#define ANALYZER_CH3	RPI_V2_GPIO_P1_19	// MOSI
#define ANALYZER_CH4	RPI_V2_GPIO_P1_24	// CE0
#define ANALYZER_CH5	RPI_V2_GPIO_P1_26	// CE1
#endif

uint32_t dmx_slot_to_slot_get(void)
{
	return dmx_slot_to_slot;
}

uint32_t dmx_slots_in_packet_get(void)
{
	return dmx_slots_in_packet;
}

/**
 * @ingroup dmx
 *
 */
void pl011_dmx512_init(void) {
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
#ifdef DEBUG_ANALYZER
	bcm2835_gpio_fsel(ANALYZER_CH1, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(ANALYZER_CH2, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(ANALYZER_CH3, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(ANALYZER_CH4, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(ANALYZER_CH5, BCM2835_GPIO_FSEL_OUTP);

	bcm2835_gpio_clr(ANALYZER_CH1);		// IRQ
	bcm2835_gpio_clr(ANALYZER_CH2);		// BREAK
	bcm2835_gpio_clr(ANALYZER_CH3);		// DATA
	bcm2835_gpio_set(ANALYZER_CH4);		// IDLE
	bcm2835_gpio_clr(ANALYZER_CH5);		// TIMEOUT
#endif
	BCM2835_PL011->IMSC = PL011_IMSC_RXIM;
    BCM2835_IRQ->FIQ_CONTROL = BCM2835_FIQ_ENABLE | INTERRUPT_VC_UART;
}

/**
 * @ingroup dmx
 *
 */
void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {
	dmb();

	dmx_fiq_micros_current = BCM2835_ST->CLO;

#ifdef DEBUG_ANALYZER
	bcm2835_gpio_set(ANALYZER_CH1);
#endif
	if (BCM2835_PL011 ->DR & PL011_DR_BE ) {
#ifdef DEBUG_ANALYZER
		bcm2835_gpio_set(ANALYZER_CH2);		// BREAK
		bcm2835_gpio_clr(ANALYZER_CH4);		// IDLE
#endif
		dmx_receive_state = BREAK;
	}  else if (dmx_receive_state == BREAK) {
		if ((BCM2835_PL011 ->DR & 0xFF) == 0x00) {		// "Start Code"
#ifdef DEBUG_ANALYZER
			bcm2835_gpio_clr(ANALYZER_CH2);	// BREAK
			bcm2835_gpio_set(ANALYZER_CH3);	// DATA
#endif
			dmx_receive_state = DATA;
			dmx_data_index = 0;
			dmx_slot_to_slot = dmx_fiq_micros_current - dmx_fiq_micros_previous;
		    BCM2835_ST->C1 = dmx_fiq_micros_current + 45;
		    BCM2835_ST->CS = BCM2835_ST_CS_M1;
		} else {							// Ignore all other; Text (0x17), System Information (0xCF), RDM (0xCC), etc.
#ifdef DEBUG_ANALYZER
			bcm2835_gpio_clr(ANALYZER_CH2);	// BREAK
			bcm2835_gpio_set(ANALYZER_CH4);	// IDLE
#endif
			dmx_receive_state = IDLE;
		}
	} else if (dmx_receive_state == DATA) {
		dmx_data[dmx_data_index] = (BCM2835_PL011 ->DR & 0xFF);
		dmx_data_index++;
		dmx_slot_to_slot = dmx_fiq_micros_current - dmx_fiq_micros_previous;
	    BCM2835_ST->C1 = dmx_fiq_micros_current + 45;
	    BCM2835_ST->CS = BCM2835_ST_CS_M1;
		if (dmx_data_index >= 512){
			dmx_receive_state = IDLE;
			dmx_slots_in_packet = 512;
			dmx_data_refreshed = 0xFF;			// Set all bits
#ifdef DEBUG_ANALYZER
			bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
			bcm2835_gpio_set(ANALYZER_CH4);	// IDLE
#endif
		}
	}
#ifdef DEBUG_ANALYZER
	bcm2835_gpio_clr(ANALYZER_CH1);
#endif
	dmx_fiq_micros_previous = dmx_fiq_micros_current;

	dmb();
}

unsigned int irq_counter;

void __attribute__((interrupt("IRQ"))) c_irq_handler(void)
{
	dmb();

	const uint32_t clo = BCM2835_ST->CLO;

	#ifdef DEBUG_ANALYZER
	bcm2835_gpio_set(ANALYZER_CH2);	// BREAK
#endif

	BCM2835_ST->CS = BCM2835_ST_CS_M1;

	if (dmx_receive_state == DATA)
	{
		if (clo > dmx_fiq_micros_current + dmx_slot_to_slot)
		{
			dmx_receive_state = IDLE;
			dmx_slots_in_packet = dmx_data_index;
			dmx_data_refreshed = 0xFF;			// Set all bits
#ifdef DEBUG_ANALYZER
			bcm2835_gpio_set(ANALYZER_CH5);
			bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
			bcm2835_gpio_set(ANALYZER_CH4);	// IDLE
			bcm2835_gpio_clr(ANALYZER_CH5);
#endif
		} else
			BCM2835_ST->C1 = clo + 45;
	}

	if (BCM2835_ST->CS & BCM2835_ST_CS_M3)
	{
		BCM2835_ST->CS = BCM2835_ST_CS_M3;
		BCM2835_ST->C3 = clo + ticks_per_second_get();
		hardware_led_set(irq_counter++ & 0x01);
	}

#ifdef DEBUG_ANALYZER
	bcm2835_gpio_clr(ANALYZER_CH2);	// BREAK
#endif

	dmb();
}
