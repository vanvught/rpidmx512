/**
 * @file widget.c
 *
 * @brief DMX USB Pro Widget API Specification 1.44
 *
 * https://wiki.openlighting.org/index.php/USB_Protocol_Extensions
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
#include <stdbool.h>

#include "c/hardware.h"
#include "util.h"
#include "monitor.h"

#include "widget.h"
#include "widget_params.h"
#include "widget_usb.h"
#include "widget_monitor.h"

#include "dmx.h"
#include "rdm.h"
#include "rdm_device_info.h"
#include "rdm_send.h"
#include "rdm_e120.h"

#include "usb.h"

#define WIDGET_DATA_BUFFER_SIZE		600							///<

static uint8_t widget_data[WIDGET_DATA_BUFFER_SIZE] ALIGNED;	///< Message between widget and the USB host
static _widget_mode widget_mode = MODE_DMX_RDM;					///< \ref _widget_mode
static _widget_send_state receive_dmx_on_change = SEND_ALWAYS;	///< \ref _widget_send_state
static uint32_t widget_received_dmx_packet_period = 0;			///<
static uint32_t widget_received_dmx_packet_start = 0;			///<
static uint32_t widget_send_rdm_packet_start = 0;				///<
static bool widget_rdm_discovery_running = false;				///< Is the Widget in RDM Discovery mode?
static uint32_t widget_received_dmx_packet_count = 0; 			///<

inline static void rdm_time_out_message(void);

/*
 * GETTERS / SETTERS
 */

/**
 * @ingroup widget
 *
 * @return \ref _widget_send_state
 */
const _widget_send_state widget_get_receive_dmx_on_change() {
	return receive_dmx_on_change;
}

/**
 * @ingroup widget
 *
 * Returns the firmware varieties
 *
 * @return \ref _widget_mode
 */
const _widget_mode widget_get_mode() {
	return widget_mode;
}

/**
 * @ingroup widget
 *
 * @param mode
 */
void widget_set_mode(const _widget_mode mode) {
	widget_mode = mode;
}

/**
 * @ingroup widget
 *
 * @return
 */
const uint32_t widget_get_received_dmx_packet_period(void) {
	return widget_received_dmx_packet_period;
}

/**
 * @ingroup widget
 *
 * @param period
 */
void widget_set_received_dmx_packet_period(uint32_t period) {
	widget_received_dmx_packet_period = period;
}

/**
 * @ingroup widget
 *
 * @return
 */
const uint32_t widget_get_received_dmx_packet_count(void) {
	return widget_received_dmx_packet_count;
}

/*
 * Widget LABELs
 */

/**
 * @ingroup widget
 *
 * Get Widget Parameters Reply (Label=3 \ref GET_WIDGET_PARAMS_REPLY)
 * The Widget sends this message to the PC in response to the Get Widget Parameters request.
 */
static void widget_get_params_reply(void) {
	struct _widget_params widget_params;

	monitor_line(MONITOR_LINE_INFO, "GET_WIDGET_PARAMS_REPLY");
	monitor_line(MONITOR_LINE_STATUS, NULL);

	widget_params_get(&widget_params);
	widget_usb_send_message(GET_WIDGET_PARAMS_REPLY, (uint8_t *) &widget_params, sizeof(struct _widget_params));
}

/**
 * @ingroup widget
 *
 * Set Widget Parameters Request (Label=4 \ref SET_WIDGET_PARAMS)
 * This message sets the Widget configuration. The Widget configuration is preserved when the Widget loses power.
 *
 */
static void widget_set_params() {
	struct _widget_params widget_params;

	monitor_line(MONITOR_LINE_INFO, "SET_WIDGET_PARAMS");
	monitor_line(MONITOR_LINE_STATUS, NULL);

	dmx_set_port_direction(DMX_PORT_DIRECTION_INP, false);

	widget_params.break_time = widget_data[2];
	widget_params.mab_time = widget_data[3];
	widget_params.refresh_rate = widget_data[4];
	widget_params_set(&widget_params);

	dmx_set_port_direction(DMX_PORT_DIRECTION_INP, true);

	widget_received_dmx_packet_start = hardware_micros();
}

/**
 * @ingroup widget
 *
 * This function is called from the poll table in \ref main.c
 *
 * Received DMX Packet (Label=5 \ref RECEIVED_DMX_PACKET)
 *
 * The Widget sends this message to the PC unsolicited, whenever the Widget receives a DMX or RDM packet from the DMX port,
 * and the Receive DMX on Change mode (\ref receive_dmx_on_change) is 'Send always' (\ref SEND_ALWAYS).
 */
void widget_received_dmx_packet(void) {
	if (widget_mode == MODE_RDM_SNIFFER) {
		return;
	}

	if (widget_rdm_discovery_running
			|| (DMX_PORT_DIRECTION_INP != dmx_get_port_direction())
			|| (SEND_ON_DATA_CHANGE_ONLY == receive_dmx_on_change)) {
		return;
	}

	const uint8_t *dmx_data = dmx_get_available();

	if (dmx_data == NULL) {
		return;
	}

	const uint32_t micros_now = hardware_micros();

	if (micros_now - widget_received_dmx_packet_start < widget_received_dmx_packet_period) {
		return;
	}

	widget_received_dmx_packet_start = micros_now;

	widget_received_dmx_packet_count++;

	const struct _dmx_data *dmx_statistics = (struct _dmx_data *)dmx_data;
	const uint16_t length = (uint16_t)(dmx_statistics->statistics.slots_in_packet + 1);

	monitor_line(MONITOR_LINE_LABEL, "poll:RECEIVED_DMX_PACKET");
	monitor_line(MONITOR_LINE_INFO, "Send DMX data to HOST, %d", length);
	monitor_line(MONITOR_LINE_STATUS, NULL);

	widget_usb_send_header(RECEIVED_DMX_PACKET, length + 1);
	usb_send_byte(0); 	// DMX Receive status
	widget_usb_send_data(dmx_data, length);
	widget_usb_send_footer();
}

/**
 * @ingroup widget
 *
 * This function is called from the poll table in \ref main.c
 *
 * Received RMX Packet (Label=5 \ref RECEIVED_DMX_PACKET)
 *
 * The Widget sends this message to the PC unsolicited, whenever the Widget receives a DMX or RDM packet from the DMX port,
 * and the Receive DMX on Change mode (\ref receive_dmx_on_change) is 'Send always' (\ref SEND_ALWAYS).
 */
void widget_received_rdm_packet(void) {
	if ((widget_mode == MODE_DMX) || (widget_mode == MODE_RDM_SNIFFER)
			|| (receive_dmx_on_change == SEND_ON_DATA_CHANGE_ONLY)) {
		return;
	}

	const uint8_t *rdm_data = rdm_get_available();

	if (rdm_data == NULL) {
		return;
	}

	uint8_t message_length = 0;

	if (rdm_data[0] == E120_SC_RDM) {
		struct _rdm_command *p = (struct _rdm_command *) (rdm_data);
		const uint8_t command_class = p->command_class;
		message_length = p->message_length + 2;

		monitor_line(MONITOR_LINE_INFO, "Send RDM data to HOST, l:%d", message_length);
		monitor_line(MONITOR_LINE_STATUS, "RECEIVED_RDM_PACKET SC:0xCC");

		widget_usb_send_header(RECEIVED_DMX_PACKET, 1 + message_length);
		usb_send_byte(0); 	// RDM Receive status
		widget_usb_send_data(rdm_data, message_length);
		widget_usb_send_footer();

		const uint16_t param_id = (p->param_id[0] << 8) + p->param_id[1];

		if ((command_class == E120_DISCOVERY_COMMAND_RESPONSE) && (param_id != E120_DISC_MUTE)) {
			rdm_time_out_message();
		} else {
			widget_send_rdm_packet_start = 0;
		}
	} else if (rdm_data[0] == 0xFE) {
		message_length = 24;

		monitor_line(MONITOR_LINE_INFO, "Send RDM data to HOST, l:%d", message_length);
		monitor_line(MONITOR_LINE_STATUS, "RECEIVED_RDM_PACKET SC:0xFE");

		widget_usb_send_header(RECEIVED_DMX_PACKET, 1 + message_length);
		usb_send_byte(0); 	// RDM Receive status
		widget_usb_send_data(rdm_data, message_length);
		widget_usb_send_footer();

		rdm_time_out_message();
	}

	monitor_rdm_data(MONITOR_LINE_RDM_DATA, message_length, rdm_data, false);
}

/**
 * @ingroup widget
 *
 * Output Only Send DMX Packet Request (label = 6 \ref OUTPUT_ONLY_SEND_DMX_PACKET_REQUEST)
 *
 * This message requests the Widget to periodically send a DMX packet out of the Widget DMX port
 * at the configured DMX output rate. This message causes the widget to leave the DMX port direction
 * as output after each DMX packet is sent, so no DMX packets will be received as a result of this
 * request.
 *
 * The periodic DMX packet output will stop and the Widget DMX port direction will change to input
 * when the Widget receives any request message other than the Output Only Send DMX Packet
 * request, or the Get Widget Parameters request.
 *
 * @param data_length DMX data to send, beginning with the start code.
 */
void widget_send_dmx_packet_request_output_only(const uint16_t data_length) {
	monitor_line(MONITOR_LINE_INFO, "OUTPUT_ONLY_SEND_DMX_PACKET_REQUEST");
	monitor_line(MONITOR_LINE_STATUS, NULL);

	dmx_set_port_direction(DMX_PORT_DIRECTION_OUTP, false);

	dmx_set_send_data(widget_data, data_length);

	dmx_set_port_direction(DMX_PORT_DIRECTION_OUTP, true);
}

/**
 * @ingroup widget
 *
 * Send RDM Packet Request (label = 7 \ref SEND_RDM_PACKET_REQUEST)
 *
 * This message requests the Widget to send an RDM packet out of the Widget DMX port, and then
 * change the DMX port direction to input, so that RDM or DMX packets can be received.
 *
 * @param data_length RDM data to send, beginning with the start code.
 */
static void widget_send_rdm_packet_request(const uint16_t data_length) {
	monitor_line(MONITOR_LINE_INFO, "SEND_RDM_PACKET_REQUEST");
	monitor_line(MONITOR_LINE_STATUS, NULL);

	const struct _rdm_command *p = (struct _rdm_command *) (widget_data);

	widget_rdm_discovery_running = (p->command_class == E120_DISCOVERY_COMMAND);

	dmx_set_port_direction(DMX_PORT_DIRECTION_OUTP, false);

	rdm_send_data(widget_data, data_length);
	udelay(RDM_RESPONDER_DATA_DIRECTION_DELAY);

	dmx_set_port_direction(DMX_PORT_DIRECTION_INP, true);

	widget_send_rdm_packet_start = hardware_micros();

	monitor_rdm_data(MONITOR_LINE_RDM_DATA, data_length, widget_data, true);
}

/**
 * @ingroup widget
 *
 * This function is called from the poll table in \ref main.c
 *
 */
void widget_rdm_timeout(void) {
	if (widget_mode == MODE_RDM_SNIFFER) {
		return;
	}

	if (widget_send_rdm_packet_start == 0) {
		return;
	}

	if (hardware_micros() - widget_send_rdm_packet_start > 1000000) {// 1 second
		rdm_time_out_message();		// Send message to host Label=12 RDM_TIMEOUT
		widget_send_rdm_packet_start = 0;
	}

}

/**
 * @ingroup widget
 *
 * Receive DMX on Change (label = 8 \ref RECEIVE_DMX_ON_CHANGE)
 *
 * This message requests the Widget send a DMX packet to the PC only when the DMX values change
 * on the input port.
 *
 * By default the widget will always send, if you want to send on change it must be enabled by sending
 * this message.
 *
 * This message also reinitializes the DMX receive processing, so that if change of state reception is
 * selected, the initial received DMX data is cleared to all zeros.
 *
 */
static void widget_receive_dmx_on_change(void) {
	monitor_line(MONITOR_LINE_INFO, "RECEIVE_DMX_ON_CHANGE");
	monitor_line(MONITOR_LINE_STATUS, NULL);

	dmx_set_port_direction(DMX_PORT_DIRECTION_INP, false);

	receive_dmx_on_change = widget_data[0];

	dmx_clear_data();

	dmx_set_port_direction(DMX_PORT_DIRECTION_INP, true);

	widget_received_dmx_packet_start = hardware_micros();
}

/**
 * @ingroup widget
 *
 * Received DMX Change Of State Packet (Label = 9 \ref RECEIVED_DMX_COS_TYPE)
 *
 * The Widget sends one or more instances of this message to the PC unsolicited, whenever the
 * Widget receives a changed DMX packet from the DMX port, and the Receive DMX on Change
 * mode (\ref receive_dmx_on_change) is 'Send on data change only' (\ref SEND_ON_DATA_CHANGE_ONLY).
 */
void widget_received_dmx_change_of_state_packet(void) {
	if (widget_mode == MODE_RDM_SNIFFER) {
		return;
	}

	if (widget_rdm_discovery_running
			|| (DMX_PORT_DIRECTION_INP != dmx_get_port_direction())
			|| (SEND_ALWAYS == receive_dmx_on_change)) {
		return;
	}

	if (dmx_is_data_changed()) {
		monitor_line(MONITOR_LINE_INFO, "RECEIVED_DMX_COS_TYPE");
		monitor_line(MONITOR_LINE_STATUS, NULL);
		// TODO widget_received_dmx_change_of_state_packet
		monitor_line(MONITOR_LINE_INFO, "Sent changed DMX data to HOST");
	}
}

/**
 * @ingroup widget
 *
 * Get Widget Serial Number Reply (Label = 10 \ref GET_WIDGET_PARAMS_REPLY)
 *
 */
static void widget_get_sn_reply(void) {
	struct _rdm_device_info_data rdm_device_info_sn;

	monitor_line(MONITOR_LINE_INFO, "GET_WIDGET_PARAMS_REPLY");
	monitor_line(MONITOR_LINE_STATUS, NULL);

	rdm_device_info_get_sn(&rdm_device_info_sn);

	dmx_set_port_direction(DMX_PORT_DIRECTION_INP, false);

	widget_usb_send_message(GET_WIDGET_SN_REPLY, rdm_device_info_sn.data, rdm_device_info_sn.length);

	dmx_set_port_direction(DMX_PORT_DIRECTION_INP, true);

	widget_received_dmx_packet_start = hardware_micros();
}

/**
 * @ingroup widget
 *
 * Send RDM Discovery Request (Label=11 \ref SEND_RDM_DISCOVERY_REQUEST)
 *
 * This message requests the Widget to send an RDM Discovery Request packet out of the Widget
 * DMX port, and then receive an RDM Discovery Response.
 */
static void widget_send_rdm_discovery_request(uint16_t data_length) {
	monitor_line(MONITOR_LINE_INFO, "SEND_RDM_DISCOVERY_REQUEST");
	monitor_line(MONITOR_LINE_STATUS, NULL);

	dmx_set_port_direction(DMX_PORT_DIRECTION_OUTP, false);

	rdm_send_data(widget_data, data_length);
	udelay(RDM_RESPONDER_DATA_DIRECTION_DELAY);

	dmx_set_port_direction(DMX_PORT_DIRECTION_INP, true);

	widget_rdm_discovery_running = true;
	widget_send_rdm_packet_start = hardware_micros();

	monitor_rdm_data(MONITOR_LINE_RDM_DATA, data_length, widget_data, true);
}

/**
 * @ingroup widget
 *
 * See https://github.com/OpenLightingProject/ola/blob/master/plugins/usbpro/EnttecUsbProWidget.cpp#L353
 *
 * (Label=12 \ref RDM_TIMEOUT)
 *
 */
inline static void rdm_time_out_message(void) {
	const uint16_t message_length = 0;

	monitor_line(MONITOR_LINE_INFO, "Send RDM data to HOST, l:%d", message_length);
	monitor_line(MONITOR_LINE_STATUS, "rdm_time_out_message");

	widget_usb_send_header(RDM_TIMEOUT, message_length);
	widget_usb_send_footer();

	widget_rdm_discovery_running = false;
	widget_send_rdm_packet_start = 0;
}

/**
 * @ingroup widget
 *
 * https://wiki.openlighting.org/index.php/USB_Protocol_Extensions#Device_Manufacturer.2C_Label_.3D_77.2C_no_data
 *
 * Get Widget Manufacturer Reply (Label = 77 \ref MANUFACTURER_LABEL)
 */
static void widget_get_manufacturer_reply(void) {
	struct _rdm_device_info_data rdm_device_info_manufacturer_name;
	struct _rdm_device_info_data rdm_device_info_manufacturer_id;

	monitor_line(MONITOR_LINE_INFO, "MANUFACTURER_LABEL");
	monitor_line(MONITOR_LINE_STATUS, NULL);

	rdm_device_info_get_manufacturer_name(&rdm_device_info_manufacturer_name);
	rdm_device_info_get_manufacturer_id(&rdm_device_info_manufacturer_id);

	dmx_set_port_direction(DMX_PORT_DIRECTION_INP, false);

	widget_usb_send_header(MANUFACTURER_LABEL, rdm_device_info_manufacturer_id.length + rdm_device_info_manufacturer_name.length);
	widget_usb_send_data(rdm_device_info_manufacturer_id.data, rdm_device_info_manufacturer_id.length);
	widget_usb_send_data(rdm_device_info_manufacturer_name.data, rdm_device_info_manufacturer_name.length);
	widget_usb_send_footer();

	dmx_set_port_direction(DMX_PORT_DIRECTION_INP, true);

	widget_received_dmx_packet_start = hardware_micros();
}

/**
 * @ingroup widget
 *
 * https://wiki.openlighting.org/index.php/USB_Protocol_Extensions#Device_Name.2C_Label_.3D_78.2C_no_data
 *
 * Get Widget Name Reply (Label = 78 \ref GET_WIDGET_NAME_LABEL)
 */
static void widget_get_name_reply(void) {
	struct _rdm_device_info_data rdm_device_info_label;
	struct _widget_params_data widget_params_type_id;

	monitor_line(MONITOR_LINE_INFO, "GET_WIDGET_NAME_LABEL");
	monitor_line(MONITOR_LINE_STATUS, NULL);

	rdm_device_info_get_label(0, &rdm_device_info_label);
	widget_params_get_type_id(&widget_params_type_id);

	dmx_set_port_direction(DMX_PORT_DIRECTION_INP, false);

	widget_usb_send_header(GET_WIDGET_NAME_LABEL, widget_params_type_id.length + rdm_device_info_label.length);
	widget_usb_send_data(widget_params_type_id.data, widget_params_type_id.length);
	widget_usb_send_data(rdm_device_info_label.data, rdm_device_info_label.length);
	widget_usb_send_footer();

	dmx_set_port_direction(DMX_PORT_DIRECTION_INP, true);

	widget_received_dmx_packet_start = hardware_micros();
}

/**
 * @ingroup widget
 *
 * Read bytes from host
 *
 * This function is called from the poll table in \ref main.c
 */
void widget_receive_data_from_host(void) {
	if (usb_read_is_byte_available()) {
		const uint8_t c = usb_read_byte();

		if (AMF_START_CODE == c) {
			const uint8_t label = usb_read_byte();	// Label
			const uint8_t lsb = usb_read_byte();	// Data length LSB
			const uint8_t msb = usb_read_byte();	// Data length MSB
			const uint16_t data_length = ((uint16_t) ((uint16_t) msb << 8)) | ((uint16_t) lsb);

			uint16_t i;
			for (i = 0; i < data_length; i++) {
				widget_data[i] = usb_read_byte();
			}

			while ((AMF_END_CODE != usb_read_byte()) && (i++ < (sizeof(widget_data) / sizeof(widget_data[0]))))
				;

			monitor_line(MONITOR_LINE_LABEL, "L:%d:%d(%d)", label, data_length, i);

			switch (label) {
			case GET_WIDGET_PARAMS:
				widget_get_params_reply();
				break;
			case GET_WIDGET_SN_REQUEST:
				widget_get_sn_reply();
				break;
			case SET_WIDGET_PARAMS:
				widget_set_params();
				break;
			case GET_WIDGET_NAME_LABEL:
				widget_get_name_reply();
				break;
			case MANUFACTURER_LABEL:
				widget_get_manufacturer_reply();
				break;
			case OUTPUT_ONLY_SEND_DMX_PACKET_REQUEST:
				widget_send_dmx_packet_request_output_only(data_length);
				break;
			case RECEIVE_DMX_ON_CHANGE:
				widget_receive_dmx_on_change();
				break;
			case SEND_RDM_PACKET_REQUEST:
				widget_send_rdm_packet_request(data_length);
				break;
			case SEND_RDM_DISCOVERY_REQUEST:
				widget_send_rdm_discovery_request(data_length);
				break;
			default:
				break;
			}
		}
	}
}

