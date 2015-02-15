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
#ifdef ENABLE_FRAMEBUFFER
#include "console.h"
extern void fb_init(void);
#endif
#ifdef BW_I2C_UI
#include "bw_ui.h"
#endif

void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {}
void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {}

void dmx_data_send(const uint8_t *, const uint16_t);

#define ANALYZER_CH1	RPI_V2_GPIO_P1_23     // CLK
#define ANALYZER_CH2	RPI_V2_GPIO_P1_21     // MISO
#define ANALYZER_CH3	RPI_V2_GPIO_P1_19     // MOSI
#define ANALYZER_CH4	RPI_V2_GPIO_P1_24     // CE0

// State of sending DMX Bytes
typedef enum {
  IDLE, BREAK, MAB, DATA
} _dmx_send_state;

void task_send(void)
{
	uint8_t dmx_data[17];
	dmx_data[0] = 0; // DMX Start code
	uint16_t i = 0;
	for (i = 0; i < 16; i++)
	{
		dmx_data[i + 1] = (uint8_t) (i & 0xFF);
	}
	dmx_data_send(dmx_data, sizeof(dmx_data) / sizeof(uint8_t));
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
		{ 500000, task_led},
		{1000000, task_send}
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

void dmx_data_send(const uint8_t *data, const uint16_t length)
{
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2 | PL011_LCRH_BRK;
	// DEBUG
	bcm2835_gpio_clr(ANALYZER_CH1); // IDLE
	bcm2835_gpio_set(ANALYZER_CH2); // BREAK
	bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
	bcm2835_gpio_clr(ANALYZER_CH4); // MAB
	udelay(88);						// Break Time
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2;
	// DEBUG
	bcm2835_gpio_clr(ANALYZER_CH1); // IDLE
	bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
	bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
	bcm2835_gpio_set(ANALYZER_CH4); // MAB
	udelay(8);						// Mark After Break
	// DEBUG
	bcm2835_gpio_clr(ANALYZER_CH1); // IDLE
	bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
	bcm2835_gpio_set(ANALYZER_CH3);	// DATA
	bcm2835_gpio_clr(ANALYZER_CH4); // MAB
	uint16_t i =0;
	BCM2835_PL011->DR = 0;
	for (i = 0; i < length; i++)
	{
		while (1)
		{
			if ((BCM2835_PL011->FR & 0x20) == 0)
				break;
		}
		BCM2835_PL011->DR = data[i];
	}
	// DEBUG
	bcm2835_gpio_clr(ANALYZER_CH1); // IDLE
	bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
	bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
	bcm2835_gpio_clr(ANALYZER_CH4); // MAB
	while (1)
	{
		if ((BCM2835_PL011->FR & 0x20) == 0)
			break;
	}
	udelay(44);
}

int notmain(unsigned int earlypc) {
#ifdef ENABLE_FRAMEBUFFER
	fb_init();

	printf("Compiled on %s at %s\n", __DATE__, __TIME__);
	printf("DMX512 Sender\n");
#endif

#ifdef BW_I2C_UI
#endif

	led_init();
	led_set(1);

	bcm2835_gpio_fsel(ANALYZER_CH1, BCM2835_GPIO_FSEL_OUTP); // IDLE
	bcm2835_gpio_fsel(ANALYZER_CH2, BCM2835_GPIO_FSEL_OUTP); // BREAK
	bcm2835_gpio_fsel(ANALYZER_CH3, BCM2835_GPIO_FSEL_OUTP); // DATA
	bcm2835_gpio_fsel(ANALYZER_CH4, BCM2835_GPIO_FSEL_OUTP); // MAB

	bcm2835_gpio_set(ANALYZER_CH1); // IDLE
	bcm2835_gpio_clr(ANALYZER_CH2);	// BREAK
	bcm2835_gpio_clr(ANALYZER_CH3); // DATA
	bcm2835_gpio_clr(ANALYZER_CH4);	// MAB

	bcm2835_pl011_dmx512_begin();

	events_init();

	for (;;) {
		events_check();
	}

	return (0);
}
