/**
 * @file main.c
 *
 */
/* Copyright (C) 2014 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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
#include <bcm2835.h>
#include <bcm2835_gpio.h>
#include <bcm2835_led.h>
#include <bcm2835_pl011.h>
#include <bcm2835_pl011_dmx512.h>
#include <hardware.h>
#include "console.h"

void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {}

extern void fb_init(void);

uint8_t dmx_data[512];

#define ANALYZER_CH1	RPI_V2_GPIO_P1_23     // CLK
#define ANALYZER_CH2   RPI_V2_GPIO_P1_21     // MISO
#define ANALYZER_CH3   RPI_V2_GPIO_P1_19     // MOSI
#define ANALYZER_CH4   RPI_V2_GPIO_P1_24     // CE0

static void fiq_init(void) {
	BCM2835_PL011->IMSC = PL011_IMSC_RXIM;
    BCM2835_IRQ->FIQ_CONTROL = BCM2835_FIQ_ENABLE | INTERRUPT_VC_UART;
}

// State of receiving DMX Bytes
typedef enum {
  IDLE, BREAK, DATA
} _dmx_receive_state;

uint8_t dmx_receive_state = IDLE;
uint16_t dmx_data_index = 0;

void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {
	bcm2835_gpio_set(ANALYZER_CH1);

	if (BCM2835_PL011 ->DR & PL011_DR_BE ) {
		bcm2835_gpio_set(ANALYZER_CH2); // BREAK
		bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
		bcm2835_gpio_clr(ANALYZER_CH4);	// IDLE

		dmx_receive_state = BREAK;
	}  else if (dmx_receive_state == BREAK) {
		if ((BCM2835_PL011 ->DR & 0xFF) == 0) {
			bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
			bcm2835_gpio_set(ANALYZER_CH3);	// DATA
			bcm2835_gpio_clr(ANALYZER_CH4); // IDLE

			dmx_receive_state = DATA;
			dmx_data_index = 0;
		} else {
			bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
			bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
			bcm2835_gpio_set(ANALYZER_CH4); // IDLE

			dmx_receive_state = IDLE;
		}
	} else if (dmx_receive_state == DATA) {
		dmx_data[dmx_data_index] = (BCM2835_PL011 ->DR & 0xFF);
		dmx_data_index++;
		if (dmx_data_index >= 512) {
			bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
			bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
			bcm2835_gpio_set(ANALYZER_CH4); // IDLE

			dmx_receive_state = IDLE;
		}
	}

	bcm2835_gpio_clr(ANALYZER_CH1);
}

void task_fb(void) {
	console_set_cursor(0, 4);

	printf("01-16 : %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X\n", dmx_data[0],
			dmx_data[1], dmx_data[2], dmx_data[3], dmx_data[4], dmx_data[5],
			dmx_data[6], dmx_data[7], dmx_data[8], dmx_data[9], dmx_data[10],
			dmx_data[11], dmx_data[12], dmx_data[13], dmx_data[14],
			dmx_data[15]);
	printf("17-32 : %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X\n", dmx_data[16],
			dmx_data[17], dmx_data[18], dmx_data[19], dmx_data[20], dmx_data[21],
			dmx_data[22], dmx_data[23], dmx_data[24], dmx_data[25], dmx_data[26],
			dmx_data[27], dmx_data[28], dmx_data[29], dmx_data[30],
			dmx_data[31]);

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
		{ 1000000, task_fb},
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

	for(i = 0; i < sizeof(dmx_data); i++) {
		dmx_data[i] = 0;
	}

	__disable_fiq();

	fb_init();

	printf("Compiled on %s at %s\n", __DATE__, __TIME__);
	printf("DMX512 Receiver, data analyzer for the first 32 channels\n");

	led_init();
	led_set(1);

	bcm2835_gpio_fsel(ANALYZER_CH1, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(ANALYZER_CH2, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(ANALYZER_CH3, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(ANALYZER_CH4, BCM2835_GPIO_FSEL_OUTP);

	bcm2835_gpio_clr(ANALYZER_CH1); // IRQ
	bcm2835_gpio_clr(ANALYZER_CH2);	// BREAK
	bcm2835_gpio_clr(ANALYZER_CH3); // DATA
	bcm2835_gpio_set(ANALYZER_CH4);	// IDLE

	bcm2835_pl011_dmx512_begin();

	fiq_init();
	__enable_fiq();

	events_init();

	for (;;) {
		events_check();
	}

	return (0);
}
