/**
 * @file widget_rdm_sniffer.c
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

#include "dmx.h"
#include "rdm.h"
#include "usb_send.h"
#include "ft245rl.h"

// TODO move for util.h
typedef enum {
	FALSE = 0,
	TRUE = 1
} _boolean;

extern uint8_t rdm_data[512];


#define	SNIFFER_PACKET			0x81
#define	SNIFFER_PACKET_SIZE  	200
// if the high bit is set, this is a data byte, otherwise it's a control byte
static const uint8_t CONTROL_MASK = 0x00;
static const uint8_t DATA_MASK = 0x80;

void widget_rdm_sniffer(void)
{
	if (rdm_available_get() == FALSE)
			return;

	rdm_available_set(FALSE);

	uint8_t message_length = 0;

	if (rdm_data[0] == 0xCC)
	{
		struct _rdm_command *p = (struct _rdm_command *) (rdm_data);
		message_length = p->message_length + 2;
	}
	else if (rdm_data[0] == 0xFE)
	{
		message_length = 24;
	}

	usb_send_header(SNIFFER_PACKET, SNIFFER_PACKET_SIZE);

	uint8_t i = 0;
	for (i = 0; i < message_length; i++)
	{
		FT245RL_write_data(DATA_MASK);
		FT245RL_write_data(rdm_data[i]);
	}

	for (i = message_length; i < SNIFFER_PACKET_SIZE / 2; i++)
	{
		FT245RL_write_data(CONTROL_MASK);
		FT245RL_write_data(0x02);
	}

	usb_send_footer();

	// TODO DEBUG
	console_clear_line(11);
	for (i = 0; i < 9; i++)
	{
		printf("%.2d-%.4d:%.2X  %.2d-%.4d:%.2X %.2d-%.4d:%.2X  %.2d-%.4d:%.2X\n",
				i+1, rdm_data[i], rdm_data[i],
				i+10, rdm_data[i+9], rdm_data[i+9],
				i+19, rdm_data[i+18], rdm_data[i+18],
				i+28, rdm_data[i+27], rdm_data[i+27]);
	}
}
