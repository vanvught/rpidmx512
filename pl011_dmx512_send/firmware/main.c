/**
 * @file main.c
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

#include <stdio.h>
#include "bcm2835.h"
#include "bcm2835_gpio.h"
#include "bcm2835_led.h"
#include "bcm2835_pl011.h"
#include "bcm2835_pl011_dmx512.h"
#include "hardware.h"
#ifdef ENABLE_FRAMEBUFFER
#include "console.h"
extern void fb_init(void);
#endif
#ifdef BW_I2C_UI
#include "bw_ui.h"
#endif

void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {}

volatile uint8_t dmx_data[512];

#define ANALYZER_CH1	RPI_V2_GPIO_P1_23     // CLK
#define ANALYZER_CH2	RPI_V2_GPIO_P1_21     // MISO
#define ANALYZER_CH3	RPI_V2_GPIO_P1_19     // MOSI
#define ANALYZER_CH4	RPI_V2_GPIO_P1_24     // CE0

static uint8_t irq_counter;

static void irq_init(void)
{
	irq_counter = 0;
	BCM2835_ST->C1 = BCM2835_ST->CLO + 4;			// 4us
	BCM2835_ST->CS = BCM2835_ST_CS_M1;
	BCM2835_IRQ->IRQ_ENABLE1 = BCM2835_TIMER1_IRQn;
}

// State of sending DMX Bytes
typedef enum {
  IDLE, BREAK, MAB, DATA
} _dmx_send_state;

uint8_t dmx_send_state = IDLE;
uint16_t dmx_data_index = 0;

void __attribute__((interrupt("IRQ"))) c_irq_handler(void)
{
	dmb();
	BCM2835_ST->CS = BCM2835_ST_CS_M1;
	bcm2835_gpio_set(ANALYZER_CH1);

	switch (dmx_send_state)
	{
	case IDLE:
		dmx_send_state = BREAK;
		BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2 | PL011_LCRH_BRK;
		BCM2835_ST->C1 = BCM2835_ST->CLO + 88;
		// DEBUG
		bcm2835_gpio_set(ANALYZER_CH2); // BREAK
		bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
		bcm2835_gpio_clr(ANALYZER_CH4); // MAB
		break;
	case BREAK:
		dmx_send_state = MAB;
		BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2;
		BCM2835_ST->C1 = BCM2835_ST->CLO + 8;
		// DEBUG
		bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
		bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
		bcm2835_gpio_set(ANALYZER_CH4); // MAB
		break;
	case MAB:
		dmx_send_state = DATA;
		// DEBUG
		bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
		bcm2835_gpio_set(ANALYZER_CH3);	// DATA
		bcm2835_gpio_clr(ANALYZER_CH4); // MAB
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
			BCM2835_PL011->DR = i;
		}
		while (1)
		{
			if ((BCM2835_PL011->FR & 0x20) == 0)
				break;
		}
		BCM2835_ST->C1 = BCM2835_ST->CLO + 100;
		dmx_send_state = IDLE;
		// DEBUG
		bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
		bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
		bcm2835_gpio_clr(ANALYZER_CH4); // MAB
		break;
	default:
		BCM2835_ST->C1 = BCM2835_ST->CLO + 44;
		break;
	}

	bcm2835_gpio_clr(ANALYZER_CH1);

	dmb();
}

void task_led(void) {
	static unsigned char led_counter = 0;
	led_set(led_counter++ & 0x01);
}

typedef struct _event {
	uint64_t period;
	void (*f)(void);
} event;

const event events[] = {
		{ 500000, task_led}
};

uint64_t events_elapsed_time[sizeof(events) / sizeof(events[0])];

void events_init() {
	int i;
	uint64_t st_read_now = bcm2835_st_read();
	for (i = 0; i < (sizeof(events) / sizeof(events[0])); i++) {
		events_elapsed_time[i] += st_read_now;
	}
}

static inline void events_check() {
	int i;
	uint64_t st_read_now = bcm2835_st_read();
	for (i = 0; i < (sizeof(events) / sizeof(events[0])); i++) {
		if (st_read_now > events_elapsed_time[i] + events[i].period) {
			events[i].f();
			events_elapsed_time[i] += events[i].period;
		}
	}
}

int notmain(unsigned int earlypc) {

	int i;

	for (i = 0; i < sizeof(dmx_data); i++)
	{
		dmx_data[i] = (uint8_t)(i & 0xFF);
	}

	__disable_irq();

#ifdef ENABLE_FRAMEBUFFER
	fb_init();

	printf("Compiled on %s at %s\n", __DATE__, __TIME__);
	printf("DMX512 Sender\n");
#endif

#ifdef BW_I2C_UI
#endif

	led_init();
	led_set(1);

	bcm2835_gpio_fsel(ANALYZER_CH1, BCM2835_GPIO_FSEL_OUTP); // IRQ
	bcm2835_gpio_fsel(ANALYZER_CH2, BCM2835_GPIO_FSEL_OUTP); // BREAK
	bcm2835_gpio_fsel(ANALYZER_CH3, BCM2835_GPIO_FSEL_OUTP); // DATA
	bcm2835_gpio_fsel(ANALYZER_CH4, BCM2835_GPIO_FSEL_OUTP); // MAB

	bcm2835_gpio_clr(ANALYZER_CH1); // IRQ
	bcm2835_gpio_clr(ANALYZER_CH2);	// BREAK
	bcm2835_gpio_clr(ANALYZER_CH3); // DATA
	bcm2835_gpio_clr(ANALYZER_CH4);	// MAB

	bcm2835_pl011_dmx512_begin();

	irq_init();
	__enable_irq();

	events_init();

	for (;;) {
		events_check();
	}

	return (0);
}
