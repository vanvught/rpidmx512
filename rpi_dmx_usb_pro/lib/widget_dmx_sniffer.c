/**
 * @file widget_dmx_sniffer.c
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
#include <stdio.h>

#include "util.h"
#include "usb.h"
#include "dmx.h"
#include "console.h"

#define MONITOR_LINE_INFO		6

extern uint8_t dmx_data[DMX_DATA_BUFFER_SIZE];

#define	SNIFFER_PACKET			0x81
#define	SNIFFER_PACKET_SIZE  	200
// if the high bit is set, this is a data byte, otherwise it's a control byte
#define CONTROL_MASK			0x00
#define DATA_MASK				0x80

static uint8_t slot0 = 0;

/**
 *
 */
void widget_dmx_sniffer(void)
{
	if (dmx_available_get() == FALSE)
			return;

	dmx_available_set(FALSE);

	console_clear_line(MONITOR_LINE_INFO);

	if (slot0 != dmx_data[0])
	{
		printf("Send DMX data to HOST\n");
		slot0 = dmx_data[0];
	} else
	{
		return;
	}

	uint16_t message_length = 33;

	usb_send_header(SNIFFER_PACKET, SNIFFER_PACKET_SIZE);

	usb_send_byte(DATA_MASK);
	usb_send_byte(0);

	uint8_t i = 0;
	for (i = 0; i < message_length - 1; i++)
	{
		usb_send_byte(DATA_MASK);
		usb_send_byte(dmx_data[i]);
	}

	for (i = message_length; i < SNIFFER_PACKET_SIZE / 2; i++)
	{
		usb_send_byte(CONTROL_MASK);
		usb_send_byte(0x02);
	}

	usb_send_footer();
}
