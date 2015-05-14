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

#include "dmx.h"
#include "rdm.h"
#include "rdm_e120.h"

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

typedef enum {
	FALSE = 0,
	TRUE = 1
} _boolean;

uint8_t dmx_data[DMX_DATA_BUFFER_SIZE];									///<
static uint8_t dmx_receive_state = IDLE;								///< Current state of DMX receive
static uint16_t dmx_data_index = 0;										///<
static uint8_t dmx_available = FALSE;									///<
static volatile uint32_t dmx_fiq_micros_current = 0;					///<
static volatile uint32_t dmx_fiq_micros_previous = 0;					///<
static volatile uint32_t dmx_slot_to_slot = 0;							///<
static volatile uint32_t dmx_break_to_break = 0;						///<
static volatile uint32_t dmx_break_to_break_previous = 0;				///<
static volatile uint32_t dmx_slots_in_packet = 0;						///<

#define RDM_DATA_BUFFER_INDEX_SIZE 	0x0F												///<
static uint16_t rdm_data_buffer_index_head = 0;											///<
static uint8_t rdm_data_buffer[RDM_DATA_BUFFER_INDEX_SIZE + 1][RDM_DATA_BUFFER_SIZE];	///<
static uint16_t rdm_checksum = 0;														///<
static uint32_t rdm_data_receive_end = 0;												///<
#ifdef RDM_CONTROLLER
static uint8_t rdm_disc_index = 0;														///<
#endif
static struct _total_statistics total_statistics;										///<

#define ANALYZER_CH1	RPI_V2_GPIO_P1_23	///< CLK
#define ANALYZER_CH2	RPI_V2_GPIO_P1_21	///< MISO
#define ANALYZER_CH3	RPI_V2_GPIO_P1_19	///< MOSI
#define ANALYZER_CH4	RPI_V2_GPIO_P1_24	///< CE0
#define ANALYZER_CH5	RPI_V2_GPIO_P1_26	///< CE1

static void fiq_init(void) {
	BCM2835_PL011->IMSC = PL011_IMSC_RXIM;
    BCM2835_IRQ->FIQ_CONTROL = BCM2835_FIQ_ENABLE | INTERRUPT_VC_UART;
}

void __attribute__((interrupt("FIQ"))) c_fiq_handler(void)
{
	dmb();

	bcm2835_gpio_set(ANALYZER_CH1);

	dmx_fiq_micros_current = BCM2835_ST->CLO;

	if (BCM2835_PL011->DR & PL011_DR_BE)
	{
		bcm2835_gpio_set(ANALYZER_CH2);	// BREAK
		bcm2835_gpio_clr(ANALYZER_CH4);	// IDLE
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
				rdm_data_buffer[rdm_data_buffer_index_head][dmx_data_index++] = 0xFE;
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
				bcm2835_gpio_clr(ANALYZER_CH2);	// BREAK
			    bcm2835_gpio_set(ANALYZER_CH3); // DATA
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
				bcm2835_gpio_set(ANALYZER_CH4);	// IDLE
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
				bcm2835_gpio_clr(ANALYZER_CH3); // DATA
				bcm2835_gpio_set(ANALYZER_CH4);	// IDLE
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

	bcm2835_gpio_clr(ANALYZER_CH1);

	dmb();
}

static unsigned int irq_counter;

void __attribute__((interrupt("IRQ"))) c_irq_handler(void)
{
	dmb();

	bcm2835_gpio_set(ANALYZER_CH5); // IRQ

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
				bcm2835_gpio_clr(ANALYZER_CH3); // DATA
				bcm2835_gpio_set(ANALYZER_CH4);	// IDLE
			}
			else
				BCM2835_ST->C1 = clo + 45;
		}
	}

	if (BCM2835_ST->CS & BCM2835_ST_CS_M3)
	{
		BCM2835_ST->CS = BCM2835_ST_CS_M3;
		BCM2835_ST->C3 = clo + (1E6 / 2);
		hardware_led_set(irq_counter++ & 0x01);
	}

	bcm2835_gpio_clr(ANALYZER_CH5); // IRQ

	dmb();
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

	console_clear_line(14);
	printf("Slots in packet %ld  \n", dmx_slots_in_packet);
	printf("Slot to slot    %ld  \n", dmx_slot_to_slot);
	printf("Break to break  %ld  \n", dmx_break_to_break);

}

typedef struct _event {
	uint64_t period;
	void (*f)(void);
} event;

const event events[] = {
		{ 1000000, task_fb}
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
	__disable_irq();

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
	bcm2835_gpio_fsel(ANALYZER_CH5, BCM2835_GPIO_FSEL_OUTP);

	bcm2835_gpio_clr(ANALYZER_CH1); // FIQ
	bcm2835_gpio_clr(ANALYZER_CH2);	// BREAK
	bcm2835_gpio_clr(ANALYZER_CH3); // DATA
	bcm2835_gpio_set(ANALYZER_CH4);	// IDLE
	bcm2835_gpio_clr(ANALYZER_CH5); // IRQ

	dmx_available = FALSE;

    BCM2835_ST->C3 = BCM2835_ST->CLO + (1E6 / 2);
    BCM2835_ST->CS = BCM2835_ST_CS_M1 + BCM2835_ST_CS_M3;
	BCM2835_IRQ->IRQ_ENABLE1 = BCM2835_TIMER1_IRQn + BCM2835_TIMER3_IRQn;
	__enable_irq();

	bcm2835_pl011_dmx512_begin();
	fiq_init();
	__enable_fiq();

	events_init();

	for (;;) {
		events_check();
	}

	return (0);
}
