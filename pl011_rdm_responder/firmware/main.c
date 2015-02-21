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
#include <stdlib.h>
#include <string.h>

#include "bcm2835.h"
#include "bcm2835_gpio.h"
#include "bcm2835_led.h"
#include "bcm2835_pl011.h"
#include "bcm2835_pl011_dmx512.h"
#include "sys_time.h"
#include "hardware.h"
#include "console.h"

#include "rdm_e120.h"

void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {}

extern void fb_init(void);

typedef enum {
	FALSE = 0,
	TRUE = 1
} _boolean;

///< State of receiving DMX Bytes
typedef enum {
  IDLE, BREAK, DMXDATA, RDMDATA, CHECKSUMH, CHECKSUML
} _dmx_receive_state;

/**
 * The size of a UID.
 */
enum
{
	UID_SIZE = 6 /**< The size of a UID in binary form */
};

struct _rdm_command
{
	uint8_t start_code;						///< 1
	uint8_t sub_start_code;					///< 2
	uint8_t message_length;					///< 3
	uint8_t destination_uid[UID_SIZE];		///< 4,5,6,7,8,9
	uint8_t source_uid[UID_SIZE];			///< 10,11,12,13,14,15
	uint8_t transaction_number;				///< 16
	uint8_t port_id;						///< 17
	uint8_t message_count;					///< 18
	uint16_t sub_device;					///< 19, 20
	uint8_t command_class;					///< 21
	uint16_t param_id;						///< 22, 23
	uint8_t param_data_length;				///< 24
	uint8_t param_data[60-24];
} ;

struct _rdm_discovery_msg {
	uint8_t header_FE[7];
	uint8_t header_AA;
	uint8_t masked_device_id[12];
	uint8_t checksum[4];
};

typedef enum
{
	DMX_PORT_DIRECTION_IDLE = 0,	///<
	DMX_PORT_DIRECTION_OUTP = 1,	///< DMX output
	DMX_PORT_DIRECTION_INP = 2		///< DMX input
} _dmx_port_direction;

static uint8_t dmx_port_direction = DMX_PORT_DIRECTION_INP;

static uint8_t dmx_data[512];
static uint8_t dmx_receive_state = IDLE;
static uint16_t dmx_data_index = 0;

static uint8_t rdm_data[60];
static uint16_t rdm_checksum = 0;
static uint64_t rdm_data_receive_end = 0;
static uint8_t rdm_available = FALSE;
static uint8_t rdm_is_mute = FALSE;

// Unique identifier (UID) which consists of a 2 byte ESTA manufacturer ID, and a 4 byte device ID.
static const uint8_t UID_ALL[UID_SIZE] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
// RESERVED FOR PROTOTYPING/EXPERIMENTAL USE ONLY
static const uint8_t UID_DEVICE[UID_SIZE] = { 0x7F, 0xF0, 0x00, 0x00, 0x00, 0x01 };

#define ANALYZER_CH1	RPI_V2_GPIO_P1_23	///< CLK
#define ANALYZER_CH2	RPI_V2_GPIO_P1_21	///< MISO
#define ANALYZER_CH3	RPI_V2_GPIO_P1_19	///< MOSI
#define ANALYZER_CH4	RPI_V2_GPIO_P1_24	///< CE0

static void fiq_init(void) {
	BCM2835_PL011->IMSC = PL011_IMSC_RXIM;
    BCM2835_IRQ->FIQ_CONTROL = BCM2835_FIQ_ENABLE | INTERRUPT_VC_UART;
}

void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {
	dmb();
	bcm2835_gpio_set(ANALYZER_CH1);

	if (BCM2835_PL011 ->DR & PL011_DR_BE ) {
		bcm2835_gpio_set(ANALYZER_CH2); // BREAK
		bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
		bcm2835_gpio_clr(ANALYZER_CH4);	// IDLE

		dmx_receive_state = BREAK;
	}  else if (dmx_receive_state == BREAK) {
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
			rdm_checksum =  0xCC;
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
		} else
		{
			uint8_t data = (BCM2835_PL011->DR & 0xFF);
			rdm_data[dmx_data_index++] =  data;
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
			rdm_available = TRUE;
			rdm_data_receive_end = bcm2835_st_read();
		}

		bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
		bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
		bcm2835_gpio_set(ANALYZER_CH4); // IDLE

		dmx_receive_state = IDLE;
	}

	bcm2835_gpio_clr(ANALYZER_CH1);
	dmb();
}

static void dmx_data_start(void)
{
	switch (dmx_port_direction)
	{
	case DMX_PORT_DIRECTION_OUTP:
		// Nothing to do here
		break;
	case DMX_PORT_DIRECTION_INP:
		fiq_init();
		__enable_fiq();
		break;
	default:
		break;
	}
}

static void dmx_data_stop(void)
{
	__disable_fiq();
	//__disable_irq();
}

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

	if (enable_data == TRUE)
	{
		dmx_data_start();
	}

	return;
}

void dmx_data_send(const uint8_t *data, const uint16_t data_length)
{
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2 | PL011_LCRH_BRK;
#ifdef DEBUG_ANALYZER
	bcm2835_gpio_clr(ANALYZER_CH1); // IDLE
	bcm2835_gpio_set(ANALYZER_CH2); // BREAK
	bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
	bcm2835_gpio_clr(ANALYZER_CH4); // MAB
#endif
	udelay(88);						// Break Time
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2;
#ifdef DEBUG_ANALYZER
	bcm2835_gpio_clr(ANALYZER_CH1); // IDLE
	bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
	bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
	bcm2835_gpio_set(ANALYZER_CH4); // MAB
#endif
	udelay(8);						// Mark After Break
#ifdef DEBUG_ANALYZER
	bcm2835_gpio_clr(ANALYZER_CH1); // IDLE
	bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
	bcm2835_gpio_set(ANALYZER_CH3);	// DATA
	bcm2835_gpio_clr(ANALYZER_CH4); // MAB
#endif
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
#ifdef DEBUG_ANALYZER
	bcm2835_gpio_set(ANALYZER_CH1); // IDLE
	bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
	bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
	bcm2835_gpio_clr(ANALYZER_CH4); // MAB
#endif
	while (1)
	{
		if ((BCM2835_PL011->FR & 0x20) == 0)
			break;
	}
	udelay(44);
}

static void process_rdm_message(const uint8_t command_class, const uint16_t param_id, const uint8_t rdm_packet_is_handled)
{

}

static void rdm_respond_message(uint8_t rdm_packet_is_handled)
{
	uint16_t rdm_checksum = 0;

	printf("rdm_respond_message\n");

	struct _rdm_command *rdm_response = (struct _rdm_command *)rdm_data;

	if(rdm_packet_is_handled == TRUE)
	{
		rdm_response->port_id = E120_RESPONSE_TYPE_ACK;
	} else
	{
		rdm_response->port_id = E120_RESPONSE_TYPE_NACK_REASON;
		rdm_response->param_data_length = 2;
		rdm_response->param_data[0] = 0;
		rdm_response->param_data[1] = 0;
	}

	memcpy(rdm_response->destination_uid, rdm_response->source_uid, UID_SIZE);
	memcpy(rdm_response->source_uid, UID_DEVICE, UID_SIZE);

	rdm_response->command_class++;

	uint8_t i = 0;
	for (i = 0; i < rdm_response->message_length; i++)
	{
		rdm_checksum += rdm_data[i];
	}

	rdm_data[i++] = rdm_checksum >> 8;
	rdm_data[i] = rdm_checksum & 0XFF;;

	uint64_t delay = bcm2835_st_read() - rdm_data_receive_end;

	if (delay < 180)
	{
		udelay(180 - delay);
	}

	dmx_port_direction_set(DMX_PORT_DIRECTION_OUTP, FALSE);

	dmx_data_send(rdm_data, rdm_response->message_length + 2);

	dmx_port_direction_set(DMX_PORT_DIRECTION_INP, TRUE);
}

static void handle_rdm_data(void)
{
	if(rdm_available == FALSE)
		return;

	rdm_available = FALSE;

	uint8_t rdm_packet_is_for_me = FALSE;
	uint8_t rdm_packet_is_for_all = FALSE;
	uint8_t rdm_packet_is_handled = FALSE;

	struct _rdm_command *rdm_cmd = (struct _rdm_command *)rdm_data;

	uint8_t command_class = rdm_cmd->command_class;
	uint16_t param_id = rdm_cmd->param_id;

	console_set_cursor(0, 23);
	printf("handle_rdm_data, param_id %d\n", param_id);

	if (memcmp(rdm_cmd->destination_uid, UID_ALL, UID_SIZE) == 0)
	{
		rdm_packet_is_for_all = TRUE;
	}
	else if (memcmp(rdm_cmd->destination_uid, UID_DEVICE, UID_SIZE) == 0)
	{
		rdm_packet_is_for_me = TRUE;
	}

	if ((rdm_packet_is_for_me == FALSE) && (rdm_packet_is_for_all == FALSE))
	{
		// Ignore RDM packet
	}
	else if (command_class == E120_DISCOVERY_COMMAND)
	{
		if (param_id == E120_DISC_UNIQUE_BRANCH)
		{
			if (rdm_is_mute == FALSE)
			{
				if ((memcmp(rdm_cmd->param_data, UID_DEVICE, UID_SIZE) <= 0)
						&& (memcmp(UID_DEVICE, rdm_cmd->param_data + 6,	UID_SIZE) <= 0))
				{
					struct _rdm_discovery_msg *p = (struct _rdm_discovery_msg *) (rdm_data);
					uint16_t rdm_checksum = 6 * 0xFF;

					uint8_t i = 0;
					for (i = 0; i < 7; i++)
					{
						p->header_FE[i] = 0xFE;
					}
					p->header_AA = 0xAA;

					for (i = 0; i < 6; i++)
					{
						p->masked_device_id[i + i] = UID_DEVICE[i] | 0xAA;
						p->masked_device_id[i + i + 1] = UID_DEVICE[i] | 0x55;
						rdm_checksum += UID_DEVICE[i];
					}

					p->checksum[0] = (rdm_checksum >> 8) | 0xAA;
					p->checksum[1] = (rdm_checksum >> 8) | 0x55;
					p->checksum[2] = (rdm_checksum & 0xFF) | 0xAA;
					p->checksum[3] = (rdm_checksum & 0xFF) | 0x55;

					//TODO for the time being we use dmx_data_send WITH a break. Need to fix the widget code fist.
					dmx_port_direction_set(DMX_PORT_DIRECTION_OUTP, FALSE);

					dmx_data_send(rdm_data, sizeof(struct _rdm_discovery_msg));

					dmx_port_direction_set(DMX_PORT_DIRECTION_INP, TRUE);

				}
			}
		} else if (param_id == E120_DISC_UN_MUTE)
		{
			rdm_is_mute = FALSE;
			rdm_packet_is_handled = TRUE;
			//rdm_respond_message(TRUE);
		} else if (param_id == E120_DISC_MUTE)
		{
			rdm_is_mute = TRUE;
			rdm_packet_is_handled = TRUE;
			rdm_respond_message(TRUE);
		}
	}
	else if (rdm_packet_is_for_me == FALSE)
	{
		process_rdm_message(command_class, param_id, rdm_packet_is_handled);
	}
}

void task_fb(void) {
	time_t ltime = 0;
	struct tm *local_time = NULL;

	ltime = sys_time(NULL);
    local_time = localtime(&ltime);

	console_set_cursor(0,2);

	printf("%.2d:%.2d:%.2d\n", local_time->tm_hour, local_time->tm_min, local_time->tm_sec);

	printf("%s\n", dmx_port_direction == DMX_PORT_DIRECTION_INP ? "Input" : "Output");

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

	printf("RDM data[1..28]:\n");
	uint16_t i = 0;
	for (i = 0; i < 14; i++)
	{
		printf("%.2d-%.4d:%.2x  %.2d-%.4d:%.2x\n", i+1, rdm_data[i], rdm_data[i], i+15, rdm_data[i+14], rdm_data[i+14]);
	}
}

void task_led(void) {
	static unsigned char led_counter = 0;
	led_set(led_counter++ & 0x01);
}

struct _poll
{
	void (*f)(void);
} const poll_table[] = {
		{ handle_rdm_data } };

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
	printf("RDM Responder, DMX512 data analyzer for the channels 1-32\n");

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

		for (i = 0; i < sizeof(poll_table) / sizeof(poll_table[0]); i++) {
			poll_table[i].f();
		}

		events_check();
	}

	return (0);
}
