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
#include <stdio.h>

#include "util.h"

#include "usb.h"
#include "dmx.h"
#include "rdm.h"
#include "widget.h"
#include "monitor_debug.h"
#include "sniffer.h"

/*
 * @bug What is RDM package > 100 bytes?
 *
 * This function is called from the poll table in \file main.c
 */
void widget_rdm_sniffer(void)
{
	uint8_t mode = widget_mode_get();

	if (mode != MODE_RDM_SNIFFER)
		return;

	if (rdm_available_get() == FALSE)
			return;

	rdm_available_set(FALSE);

	monitor_debug_line(MONITOR_LINE_INFO, "Send RDM data to HOST");

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
		usb_send_byte(DATA_MASK);
		usb_send_byte(rdm_data[i]);
	}

	for (i = message_length; i < SNIFFER_PACKET_SIZE / 2; i++)
	{
		usb_send_byte(CONTROL_MASK);
		usb_send_byte(0x02);
	}

	usb_send_footer();

	monitor_debug_data(MONITOR_LINE_RDM_DATA, message_length, rdm_data);
}
