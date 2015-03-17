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
#include "widget.h"
#include "monitor_debug.h"
#include "sniffer.h"

#define MONITOR_LINE_INFO		6

static uint8_t dmx_data_previous[DMX_DATA_BUFFER_SIZE];

/**
 *
 * @param start
 */
inline static void usb_send_next_package(uint16_t start)
{
	uint16_t i = 0;

	usb_send_header(SNIFFER_PACKET, SNIFFER_PACKET_SIZE);

	for (i = start; i < (start + 100); i++)
	{
		usb_send_byte(DATA_MASK);
		usb_send_byte(dmx_data[i]);
	}

	usb_send_footer();
}

/**
 *
 * @return
 */
inline static uint8_t dmx_data_is_changed(void)
{
	uint16_t i = 0;
	uint8_t is_changed = FALSE;

	for (i = 0; i < DMX_DATA_BUFFER_SIZE; i++)
	{
		if (dmx_data_previous[i] != dmx_data[i])	//TODO Is a memcpy more efficient?
		{
			is_changed = TRUE;
			dmx_data_previous[i] = dmx_data[i];
		}
	}

	return is_changed;
}

/**
 *
 * This function is called from the poll table in \file main.c
 */
void widget_dmx_sniffer(void)
{
	uint8_t mode = widget_mode_get();

	if (mode != MODE_RDM_SNIFFER)
		return;

	if (dmx_available_get() == FALSE)
			return;

	dmx_available_set(FALSE);

	monitor_debug_line(MONITOR_LINE_INFO, NULL);

	if (dmx_data_is_changed())
	{
		monitor_debug_line(MONITOR_LINE_INFO, "Send DMX data to HOST");
	} else
	{
		return;
	}

	// DMX 0-98
	usb_send_header(SNIFFER_PACKET, SNIFFER_PACKET_SIZE);

	usb_send_byte(DATA_MASK);		// TODO Start byte part of dmx_data
	usb_send_byte(0);

	uint16_t i = 0;
	for (i = 0; i < 99; i++)
	{
		usb_send_byte(DATA_MASK);
		usb_send_byte(dmx_data[i]);
	}

	usb_send_footer();

	usb_send_next_package(99);	// DMX 99-198
	usb_send_next_package(199);	// DMX 199-298
	usb_send_next_package(299);	// DMX 299-398
	usb_send_next_package(399);	// DMX 399-498

	// DMX 499-511 + remaining control bytes
	usb_send_header(SNIFFER_PACKET, SNIFFER_PACKET_SIZE);

	for (i = 499; i < 512; i++)
	{
		usb_send_byte(DATA_MASK);
		usb_send_byte(dmx_data[i]);
	}

	for (i = 13; i < SNIFFER_PACKET_SIZE / 2; i++)
	{
		usb_send_byte(CONTROL_MASK);
		usb_send_byte(0x02);
	}

	usb_send_footer();

}
