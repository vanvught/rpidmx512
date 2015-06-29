/**
 * @file widget_sniffer.c
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
#include "rdm_e120.h"
#include "widget.h"
#include "monitor.h"
#include "sniffer.h"

#define	SNIFFER_PACKET			0x81	///< Label
#define	SNIFFER_PACKET_SIZE  	200		///< Packet size
#define CONTROL_MASK			0x00	///< If the high bit is set, this is a data byte, otherwise it's a control byte
#define DATA_MASK				0x80	///< If the high bit is set, this is a data byte, otherwise it's a control byte

static struct _rdm_statistics rdm_statistics;

/**
 * @ingroup widget
 *
 * @return
 */
const struct _rdm_statistics *rdm_statistics_get(void) {
	return &rdm_statistics;
}

/**
 * @ingroup widget
 *
 * @param data
 * @param start
 * @param data_length
 */
inline static void usb_send_package(const uint8_t *data, uint16_t start, uint16_t data_length) {
	uint16_t i = 0;

	if (data_length < (uint16_t) (SNIFFER_PACKET_SIZE / 2)) {
		usb_send_header((uint8_t) SNIFFER_PACKET, (uint16_t) SNIFFER_PACKET_SIZE);

		for (i = 0; i < data_length; i++) {
			usb_send_byte(DATA_MASK);
			usb_send_byte(data[i + start]);
		}

		for (i = data_length; i < SNIFFER_PACKET_SIZE / 2; i++) {
			usb_send_byte((uint8_t) CONTROL_MASK);
			usb_send_byte(0x02);
		}

		usb_send_footer();
	} else {
		usb_send_header((uint8_t) SNIFFER_PACKET, (uint16_t) SNIFFER_PACKET_SIZE);

		for (i = 0; i < SNIFFER_PACKET_SIZE / 2; i++) {
			usb_send_byte((uint8_t) DATA_MASK);
			usb_send_byte(data[i + start]);
		}

		usb_send_footer();

		usb_send_package(data, start + SNIFFER_PACKET_SIZE / 2, data_length - SNIFFER_PACKET_SIZE / 2);
	}
}

/**
 * @ingroup widget
 *
 * This function is called from the poll table in \ref main.c
 */
void widget_sniffer_dmx(void) {
	const uint8_t mode = widget_get_mode();

	if (mode != MODE_RDM_SNIFFER)
		return;

	if (dmx_get_available() == FALSE)
		return;

	dmx_set_available_false();

	monitor_line(MONITOR_LINE_INFO, NULL);

	if (dmx_data_is_changed() == TRUE) {
		monitor_line(MONITOR_LINE_INFO, "Send DMX data to HOST");
		const uint16_t data_length = dmx_get_slots_in_packet() + 1;
		usb_send_package(dmx_data, 0, data_length);
	}
}

/**
 * @ingroup widget
 *
 * This function is called from the poll table in \ref main.c
 */
void widget_sniffer_rdm(void) {
	const uint8_t mode = widget_get_mode();

	if (mode != MODE_RDM_SNIFFER)
		return;

	const uint8_t *rdm_data = rdm_get_available();

	if (rdm_data == NULL)
		return;

	monitor_line(MONITOR_LINE_INFO, "Send RDM data to HOST");

	uint8_t message_length = 0;

	if (rdm_data[0] == 0xCC) {
		struct _rdm_command *p = (struct _rdm_command *) (rdm_data);
		message_length = p->message_length + 2;
		switch (p->command_class) {
		case E120_DISCOVERY_COMMAND:
			rdm_statistics.discovery_packets++;
			break;
		case E120_DISCOVERY_COMMAND_RESPONSE:
			rdm_statistics.discovery_response_packets++;
			break;
		case E120_GET_COMMAND:
			rdm_statistics.get_requests++;
			break;
		case E120_SET_COMMAND:
			rdm_statistics.set_requests++;
			break;
		default:
			break;
		}
	} else if (rdm_data[0] == 0xFE) {
		rdm_statistics.discovery_response_packets++;
		message_length = 24;
	}

	usb_send_package(rdm_data, 0, message_length);
}
