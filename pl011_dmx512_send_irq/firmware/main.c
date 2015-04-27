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
#include <stdint.h>

#include "bcm2835.h"
#include "bcm2835_gpio.h"
#include "bcm2835_led.h"
#include "bcm2835_pl011.h"
#include "bcm2835_pl011_dmx512.h"
#include "hardware.h"

void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {}

volatile uint8_t dmx_data[513];

#define ANALYZER_CH1	RPI_V2_GPIO_P1_23	// CLK
#define ANALYZER_CH2	RPI_V2_GPIO_P1_21	// MISO
#define ANALYZER_CH3	RPI_V2_GPIO_P1_19	// MOSI
#define ANALYZER_CH4	RPI_V2_GPIO_P1_24	// CE0
#define ANALYZER_CH5	RPI_V2_GPIO_P1_26	// CE1


struct _widget_params
{
	uint8_t firmware_lsb;
	uint8_t firmware_msb;
	uint8_t break_time;
	uint8_t mab_time;
	uint8_t refresh_rate;
} const dmx_usb_pro_params = { 4, 2, 9, 1, 40 };

static uint16_t dmx_output_break_time = 0;
static uint16_t dmx_output_mab_time = 0;
static uint16_t dmx_output_period = 0;

static volatile uint32_t dmx_send_break_micros = 0;
static volatile uint8_t dmx_send_always = 0;
static uint16_t dmx_send_data_length = 0;

static void irq_init(void)
{
	const uint32_t clo = BCM2835_ST->CLO;
	BCM2835_ST->C1 = clo + 4;
	BCM2835_ST->C3 = clo + (1E6 / 2);
    BCM2835_ST->CS = BCM2835_ST_CS_M1 + BCM2835_ST_CS_M3;
	BCM2835_IRQ->IRQ_ENABLE1 = BCM2835_TIMER1_IRQn + BCM2835_TIMER3_IRQn;
}

// State of sending DMX Bytes
typedef enum {
  IDLE, BREAK, MAB, DATA
} _dmx_send_state;

static volatile uint8_t dmx_send_state = IDLE;

static unsigned int irq_counter;

void __attribute__((interrupt("IRQ"))) c_irq_handler(void)
{
	dmb();

	const uint32_t clo = BCM2835_ST->CLO;

	// DEBUG
	bcm2835_gpio_set(ANALYZER_CH1);

	if (BCM2835_ST->CS & BCM2835_ST_CS_M1)
	{
		BCM2835_ST->CS = BCM2835_ST_CS_M1;

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
				// DEBUG
				bcm2835_gpio_set(ANALYZER_CH2); // BREAK
				bcm2835_gpio_clr(ANALYZER_CH4); // MAB
				break;
			case BREAK:
				dmx_send_state = MAB;
				BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2;
				BCM2835_ST->C1 = BCM2835_ST->CLO + dmx_output_mab_time;
				// DEBUG
				bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
				bcm2835_gpio_set(ANALYZER_CH4); // MAB
				break;
			case MAB:
				dmx_send_state = DATA;
				// DEBUG
				bcm2835_gpio_set(ANALYZER_CH3);	// DATA
				bcm2835_gpio_clr(ANALYZER_CH4); // MAB
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
				udelay(44);
				BCM2835_ST->C1 = dmx_output_period + dmx_send_break_micros;
				dmx_send_state = IDLE;
				// DEBUG
				bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
				break;
			default:
				break;
			}
		}
	}

	if (BCM2835_ST->CS & BCM2835_ST_CS_M3)
	{
		BCM2835_ST->CS = BCM2835_ST_CS_M3;
		BCM2835_ST->C3 = clo + (1E6 / 2);
		hardware_led_set(irq_counter++ & 0x01);
	}

	//DEBUG
	bcm2835_gpio_clr(ANALYZER_CH1);

	dmb();
}

static void task_on_off(void) {
	static uint8_t counter = 0;
	if ((counter & 0b01) | (counter & 0b10))
	{
		dmx_send_always = 1;
		BCM2835_ST->C1 = BCM2835_ST->CLO + 4;
		BCM2835_ST->CS = BCM2835_ST_CS_M1;
		bcm2835_gpio_set(ANALYZER_CH5);
	}
	else
	{
		dmx_send_always = 0;
		BCM2835_ST->C1 = BCM2835_ST->CLO;
		BCM2835_ST->CS = BCM2835_ST_CS_M1;
		bcm2835_gpio_clr(ANALYZER_CH5);
	}

	counter++;
}

typedef struct _event {
	uint32_t period;
	void (*f)(void);
} event;

const event events[] = {
		{ 750000, task_on_off}
};

uint32_t events_elapsed_time[sizeof(events) / sizeof(events[0])];

/**
 * @ingroup main
 *
 */
static void events_init() {
	int i;
	const uint32_t mircos_now = hardware_micros();
	for (i = 0; i < (sizeof(events) / sizeof(events[0])); i++) {
		events_elapsed_time[i] += mircos_now;
	}
}

/**
 * @ingroup main
 *
 */
inline static void events_check() {
	int i;
	const uint32_t micros_now = hardware_micros();
	for (i = 0; i < (sizeof(events) / sizeof(events[0])); i++) {
		if (micros_now > events_elapsed_time[i] + events[i].period) {
			events[i].f();
			events_elapsed_time[i] += events[i].period;
		}
	}
}

int notmain(unsigned int earlypc) {
	hardware_init();

	__disable_irq();
	__disable_fiq();

	dmx_output_break_time = (double)(dmx_usb_pro_params.break_time) * (double)(10.67);
	dmx_output_mab_time = (double)(dmx_usb_pro_params.mab_time) * (double)(10.67);
	dmx_output_period = 1E6 / dmx_usb_pro_params.refresh_rate;
	dmx_send_data_length = 32;

	printf("Compiled on %s at %s\n", __DATE__, __TIME__);
	printf("DMX512 Sender IRQ\n\n");
	printf("dmx_output_break_time (%2d) %d us\n", dmx_usb_pro_params.break_time, dmx_output_break_time);
	printf("dmx_output_mab_time   (%2d) %d us\n", dmx_usb_pro_params.mab_time, dmx_output_mab_time);
	printf("dmx_output_period     (%2d) %d us\n", dmx_usb_pro_params.refresh_rate, dmx_output_period);
	printf("dmx_send_data_length       %d\n", dmx_send_data_length);

	bcm2835_gpio_fsel(ANALYZER_CH1, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(ANALYZER_CH2, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(ANALYZER_CH3, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(ANALYZER_CH4, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(ANALYZER_CH5, BCM2835_GPIO_FSEL_OUTP);

	bcm2835_gpio_clr(ANALYZER_CH1);		// IRQ
	bcm2835_gpio_clr(ANALYZER_CH2);		// BREAK
	bcm2835_gpio_clr(ANALYZER_CH3);		// DATA
	bcm2835_gpio_clr(ANALYZER_CH4);		// MAB
	bcm2835_gpio_clr(ANALYZER_CH5);		// TIMEOUT

	int i;

	for (i = 0; i < sizeof(dmx_data); i++)
	{
		dmx_data[i] = (uint8_t)(i & 0xFF);
	}

	bcm2835_pl011_dmx512_begin();

	irq_init();
	__enable_irq();

	events_init();

	for (;;) {
		events_check();
	}

	return (0);
}
