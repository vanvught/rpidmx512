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
#include "sys_time.h"
#include "hardware.h"
#include "console.h"

void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {}

extern void fb_init(void);

///< State of receiving DMX Bytes
typedef enum {
  IDLE,			///<
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

/**
 * The size of a UID.
 */
enum
{
	UID_SIZE = 6 /**< The size of a UID in binary form */
};

struct _rdm_command
{
	uint8_t start_code;
	uint8_t sub_start_code;
	uint8_t message_length;
	uint8_t destination_uid[UID_SIZE];
	uint8_t source_uid[UID_SIZE];
	uint8_t transaction_number;
	uint8_t port_id;
	uint8_t message_count;
	uint8_t sub_device[2];
	uint8_t command_class;
	uint8_t param_id[2];
	uint8_t param_data_length;
} ;

static uint8_t dmx_data[512];
static uint8_t dmx_receive_state = IDLE;
static uint16_t dmx_data_index = 0;

static uint8_t rdm_data[60];
static uint16_t rdm_checksum = 0;
static uint8_t rdm_available = 0;
static uint8_t rdm_disc_index = 0;

#define ANALYZER_CH1	RPI_V2_GPIO_P1_23	///< CLK
#define ANALYZER_CH2	RPI_V2_GPIO_P1_21	///< MISO
#define ANALYZER_CH3	RPI_V2_GPIO_P1_19	///< MOSI
#define ANALYZER_CH4	RPI_V2_GPIO_P1_24	///< CE0

typedef enum {
	FALSE = 0,
	TRUE = 1
} _boolean;

static void fiq_init(void) {
	BCM2835_PL011->IMSC = PL011_IMSC_RXIM;
    BCM2835_IRQ->FIQ_CONTROL = BCM2835_FIQ_ENABLE | INTERRUPT_VC_UART;
}

void __attribute__((interrupt("FIQ"))) c_fiq_handler(void)
{
	bcm2835_gpio_set(ANALYZER_CH1);

	if (BCM2835_PL011->DR & PL011_DR_BE)
	{
		bcm2835_gpio_set(ANALYZER_CH2); // BREAK
		bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
		bcm2835_gpio_clr(ANALYZER_CH4);	// IDLE

		dmx_receive_state = BREAK;
	}
	else if (dmx_receive_state == IDLE)
	{
		uint8_t data = BCM2835_PL011->DR & 0xFF;
		if (data == 0xFE)
		{
			bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
			bcm2835_gpio_set(ANALYZER_CH3);	// DATA
			bcm2835_gpio_clr(ANALYZER_CH4); // IDLE

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
			bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
			bcm2835_gpio_set(ANALYZER_CH3);	// DATA
			bcm2835_gpio_clr(ANALYZER_CH4); // IDLE

			dmx_receive_state = DMXDATA;
			dmx_data_index = 0;
		}
		else if (data == 0xCC)	// RDM start code
		{
			bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
			bcm2835_gpio_set(ANALYZER_CH3);	// DATA
			bcm2835_gpio_clr(ANALYZER_CH4); // IDLE

			dmx_receive_state = RDMDATA;
			dmx_data_index = 0;
			rdm_data[dmx_data_index++] = 0xCC;
			rdm_checksum = 0xCC;
		}
		else
		{
			bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
			bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
			bcm2835_gpio_set(ANALYZER_CH4); // IDLE

			dmx_receive_state = IDLE;
		}
	}
	else if (dmx_receive_state == DMXDATA)
	{
		dmx_data[dmx_data_index++] = (BCM2835_PL011->DR & 0xFF);
		if (dmx_data_index >= 512)
		{
			bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
			bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
			bcm2835_gpio_set(ANALYZER_CH4); // IDLE

			dmx_receive_state = IDLE;
		}
	}
	else if (dmx_receive_state == RDMDATA)
	{
		if (dmx_data_index > (sizeof(rdm_data) / sizeof(uint8_t)))
		{
			bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
			bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
			bcm2835_gpio_set(ANALYZER_CH4); // IDLE

			dmx_receive_state = IDLE;
		}
		else
		{
			uint8_t data = (BCM2835_PL011->DR & 0xFF);
			rdm_data[dmx_data_index++] = data;
			rdm_checksum += data;

			struct _rdm_command *p = (struct _rdm_command *) (rdm_data);
			if (dmx_data_index == p->message_length)
			{
				dmx_receive_state = CHECKSUMH;
			}
		}
	}
	else if (dmx_receive_state == CHECKSUMH)
	{
		uint8_t data = (BCM2835_PL011->DR & 0xFF);
		rdm_checksum -= data << 8;
		dmx_receive_state = CHECKSUML;
	}
	else if (dmx_receive_state == CHECKSUML)
	{
		uint8_t data = (BCM2835_PL011->DR & 0xFF);
		rdm_checksum -= data;

		struct _rdm_command *p = (struct _rdm_command *) (rdm_data);
		if ((rdm_checksum == 0) && (p->sub_start_code == 0x01)) // E120_SC_SUB_MESSAGE
		{
			rdm_available = 1;
		}

		bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
		bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
		bcm2835_gpio_set(ANALYZER_CH4); // IDLE

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
		if (rdm_disc_index == 2 * UID_SIZE)
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
		}
	}

	bcm2835_gpio_clr(ANALYZER_CH1);
}

void task_fb(void) {
	time_t ltime = 0;
	struct tm *local_time = NULL;

	ltime = sys_time(NULL);
    local_time = localtime(&ltime);

	console_set_cursor(0,3);

	printf("%.2d:%.2d:%.2d\n", local_time->tm_hour, local_time->tm_min, local_time->tm_sec);

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

	printf("\nRDM data[1..36]:\n");
	uint8_t i = 0;
	for (i = 0; i < 9; i++)
	{
		printf("%.2d-%.4d:%.2X  %.2d-%.4d:%.2X %.2d-%.4d:%.2X  %.2d-%.4d:%.2X\n",
				i+1, rdm_data[i], rdm_data[i],
				i+10, rdm_data[i+9], rdm_data[i+9],
				i+19, rdm_data[i+18], rdm_data[i+18],
				i+28, rdm_data[i+27], rdm_data[i+27]);
	}
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
	__disable_fiq();

	sys_time_init();

	fb_init();

	led_init();
	led_set(1);

	int i;
	for (i = 0; i < sizeof(dmx_data); i++)
	{
		dmx_data[i] = 0;
	}

	printf("Compiled on %s at %s\n", __DATE__, __TIME__);
	printf("DMX512 Receiver, data analyzer for the first 32 channels\n");

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
