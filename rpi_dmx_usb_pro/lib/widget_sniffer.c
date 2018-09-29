/**
 * @file widget_sniffer.c
 *
 */
/* Copyright (C) 2015-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "c/hardware.h"

#include "monitor.h"

#include "widget.h"
#include "widget_usb.h"
#include "sniffer.h"

#include "dmx.h"
#include "rdm.h"
#include "rdm_e120.h"

#include "usb.h"

#define	SNIFFER_PACKET			0x81	///< Label
#define	SNIFFER_PACKET_SIZE  	200		///< Packet size
#define CONTROL_MASK			0x00	///< If the high bit is set, this is a data byte, otherwise it's a control byte
#define DATA_MASK				0x80	///< If the high bit is set, this is a data byte, otherwise it's a control byte

static struct _rdm_statistics rdm_statistics ALIGNED;	///<

const struct _rdm_statistics *rdm_statistics_get(void) {
	return &rdm_statistics;
}

static void usb_send_package(const uint8_t *data, const uint16_t start, const uint16_t data_length) {
	uint16_t i;

	if (data_length < (uint16_t) (SNIFFER_PACKET_SIZE / 2)) {
		widget_usb_send_header((uint8_t) SNIFFER_PACKET, (uint16_t) SNIFFER_PACKET_SIZE);

		for (i = 0; i < data_length; i++) {
			usb_send_byte(DATA_MASK);
			usb_send_byte(data[i + start]);
		}

		for (i = data_length; i < SNIFFER_PACKET_SIZE / 2; i++) {
			usb_send_byte((uint8_t) CONTROL_MASK);
			usb_send_byte(0x02);
		}

		widget_usb_send_footer();
	} else {
		widget_usb_send_header((uint8_t) SNIFFER_PACKET, (uint16_t) SNIFFER_PACKET_SIZE);

		for (i = 0; i < SNIFFER_PACKET_SIZE / 2; i++) {
			usb_send_byte((uint8_t) DATA_MASK);
			usb_send_byte(data[i + start]);
		}

		widget_usb_send_footer();

		usb_send_package(data, start + SNIFFER_PACKET_SIZE / 2, data_length - SNIFFER_PACKET_SIZE / 2);
	}
}

static bool can_send(void) {
	const uint32_t micros = hardware_micros();

	while (!usb_can_write() && (hardware_micros() - micros < (uint32_t) 1000)) {
	}

	if (!usb_can_write()) {
		monitor_line(MONITOR_LINE_INFO, "!Failed! Cannot send to host");
		return false;
	}

	return true;
}

/**
 * This function is called from the poll table in main.c
 */
void widget_sniffer_dmx(void) {
	if ((widget_get_mode() != MODE_RDM_SNIFFER) || !can_send()) {
		return;
	}

	const uint8_t *dmx_data = dmx_is_data_changed();

	if (dmx_data == NULL) {
		return;
	}

	const struct _dmx_data *dmx_statistics = (struct _dmx_data *)dmx_data;
	const uint16_t data_length = (uint16_t)(dmx_statistics->statistics.slots_in_packet + 1);

	if (!can_send()) {
		return;
	}

	monitor_line(MONITOR_LINE_INFO, "Send DMX data to HOST -> %d", data_length);
	usb_send_package(dmx_data, 0, data_length);
}

/**
 * This function is called from the poll table in main.c
 */
void widget_sniffer_rdm(void) {
	if ((widget_get_mode() != MODE_RDM_SNIFFER) || !can_send()) {
		return;
	}

	const uint8_t *rdm_data = rdm_get_available();

	if (rdm_data == NULL) {
		return;
	}

	uint8_t message_length = 0;

	if (rdm_data[0] == E120_SC_RDM) {
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

	if (!can_send()) {
		return;
	}

	monitor_line(MONITOR_LINE_INFO, "Send RDM data to HOST");
	usb_send_package(rdm_data, 0, message_length);
}

void widget_sniffer_fill_transmit_buffer(void) {
	if (!can_send()) {
		return;
	}

	int i = 256;

	while(i--) {
		if (!can_send()) {
			return;
		}
		usb_send_byte(0);
	}
}
