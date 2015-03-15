/**
 * @file widget.c
 *
 * @brief DMX USB Pro Widget API Specification 1.44
 *
 */
/* Copyright (C) 2015by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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
#include <string.h>

#include "bcm2835.h"
#include "util.h"
#include "widget.h"
#include "widget_params.h"
#include "usb.h"
#include "dmx.h"
#include "rdm.h"
#include "rdm_e120.h"
#include "console.h"

#define MONITOR_LINE_LABEL		5
#define MONITOR_LINE_INFO		6
#define MONITOR_LINE_RDM_DATA	11
#define MONITOR_LINE_STATUS		23

extern uint8_t dmx_data[DMX_DATA_BUFFER_SIZE];
extern uint8_t rdm_data[RDM_DATA_BUFFER_SIZE];

static uint8_t widget_data[600];
static uint8_t receive_dmx_on_change = SEND_ALWAYS;
static uint8_t rdm_discovery_running = FALSE;
static uint64_t widget_send_rdm_packet_start = 0;
static uint8_t widget_dmx_output_only = FALSE;
static uint64_t widget_dmx_output_period = 0;
static uint64_t widget_dmx_output_elapsed_time = 0;
static uint16_t widget_dmx_output_data_length = 0;

static const uint8_t DEVICE_MANUFACTURER_ID[] = {0xF0, 0x7F};
static const uint8_t DEVICE_MANUFACTURER_NAME[] = "AvV";
static const uint8_t DEVICE_NAME[] = "Raspberry Pi DMX USB Pro";
static const uint8_t DEVICE_ID[] = {1, 0};

inline static void rdm_time_out_message(void);

/*
 * GETTERS / SETTERS
 */

/**
 *
 * @return
 */
uint8_t receive_dmx_on_change_get()
{
	return receive_dmx_on_change;
}

/**
 *
 * @return
 */
uint64_t widget_dmx_output_period_get(void)
{
	return widget_dmx_output_period;
}

/**
 *
 * @param dmx_output_period
 */
void widget_dmx_output_period_set(uint64_t dmx_output_period)
{
	widget_dmx_output_period = dmx_output_period;
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
static void widget_get_params_reply(void)
{
	struct _widget_params widget_params;
	widget_params_get(&widget_params);
	usb_send_message(GET_WIDGET_PARAMS_REPLY, (uint8_t *)&widget_params, sizeof(struct _widget_params));
}

/**
 * @ingroup widget
 *
 * Set Widget Parameters Request (Label=4 \ref SET_WIDGET_PARAMS)
 * This message sets the Widget configuration. The Widget configuration is preserved when the Widget loses power.
 *
 */
static void widget_set_params()
{
	widget_params_break_time_set(widget_data[2]);
	widget_params_mab_time_set(widget_data[3]);
	widget_params_refresh_rate_set(widget_data[4]);
	dmx_port_direction_set(DMX_PORT_DIRECTION_INP, TRUE);
}

/**
 * @ingroup widget
 *
 * This function is called from the poll table in \file main.c
 *
 * Received DMX Packet (Label=5 \ref RECEIVED_DMX_PACKET)
 *
 * The Widget sends this message to the PC unsolicited, whenever the Widget receives a DMX or RDM packet from the DMX port,
 * and the Receive DMX on Change mode (\ref receive_dmx_on_change) is 'Send always' (\ref SEND_ALWAYS).
 */
void widget_received_dmx_packet(void)
{
	if ((rdm_discovery_running == TRUE) || (DMX_PORT_DIRECTION_INP != dmx_port_direction_get()) || (SEND_ON_DATA_CHANGE_ONLY == receive_dmx_on_change))
		return;

	console_clear_line(MONITOR_LINE_INFO);
	printf("Send DMX data to HOST\n");

	usb_send_header(RECEIVED_DMX_PACKET, 2 + (sizeof(dmx_data) / sizeof(uint8_t)));
	usb_send_byte(0); 	// DMX Receive status
	usb_send_byte(0);	// DMX Start code
	usb_send_data(dmx_data, sizeof(dmx_data) / sizeof(uint8_t));
	usb_send_footer();
}

/**
 * @ingroup widget
 *
 * This function is called from the poll table in \file main.c
 *
 * Received RMX Packet (Label=5 \ref RECEIVED_DMX_PACKET)
 *
 * The Widget sends this message to the PC unsolicited, whenever the Widget receives a DMX or RDM packet from the DMX port,
 * and the Receive DMX on Change mode (\ref receive_dmx_on_change) is 'Send always' (\ref SEND_ALWAYS).
 */
void widget_received_rdm_packet(void)
{
	if ((rdm_available_get() == FALSE)  || (receive_dmx_on_change == SEND_ON_DATA_CHANGE_ONLY))
		return;

	rdm_available_set(FALSE);

	if (rdm_data[0] == E120_SC_RDM)
	{
		struct _rdm_command *p = (struct _rdm_command *) (rdm_data);
		uint8_t message_length = p->message_length + 2;

		console_clear_line(MONITOR_LINE_INFO);
		printf("Send RDM data to HOST, package length : %d\n", message_length);

		usb_send_header(RECEIVED_DMX_PACKET, 1 + message_length);
		usb_send_byte(0); 	// RDM Receive status
		usb_send_data(rdm_data, message_length);
		usb_send_footer();

		if (p->command_class == E120_DISCOVERY_COMMAND_RESPONSE)
			rdm_time_out_message();
		else
			widget_send_rdm_packet_start = 0;

	} else if (rdm_data[0] == 0xFE)
	{
		uint8_t message_length = 24;

		console_clear_line(MONITOR_LINE_INFO);
		printf("Send RDM data to HOST, package length : %d\n", message_length);

		usb_send_header(RECEIVED_DMX_PACKET, 1 + message_length);
		usb_send_byte(0); 	// RDM Receive status
		usb_send_data(rdm_data, message_length);
		usb_send_footer();

		rdm_time_out_message();
	}

	// TODO DEBUG
	console_clear_line(MONITOR_LINE_RDM_DATA);
	struct _rdm_command *p = (struct _rdm_command *) (rdm_data);
	printf("RDM Packet length : %d\n", p->message_length);
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
void widget_output_only_send_dmx_packet_request(const uint16_t data_length)
{
	uint16_t i = 0;
	for (i = 1; i < data_length; i++)
		dmx_data[i - 1] = widget_data[i];

	widget_dmx_output_elapsed_time = bcm2835_st_read();
	widget_dmx_output_data_length = data_length - 1;

	dmx_port_direction_set(DMX_PORT_DIRECTION_OUTP, TRUE);
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
static void widget_send_rdm_packet_request(const uint16_t data_length)
{
	console_clear_line(MONITOR_LINE_STATUS);
	printf("widget_send_rdm_packet_request\n");

	struct _rdm_command *p = (struct _rdm_command *)(widget_data);

	if (p->command_class == E120_DISCOVERY_COMMAND)
		rdm_discovery_running = TRUE;
	else
		rdm_discovery_running = FALSE;

	dmx_port_direction_set(DMX_PORT_DIRECTION_OUTP, FALSE);
	rdm_data_send(widget_data, data_length);
	dmx_port_direction_set(DMX_PORT_DIRECTION_INP, TRUE);
	widget_send_rdm_packet_start =  bcm2835_st_read();

	// TODO DEBUG
	console_clear_line(MONITOR_LINE_RDM_DATA);
	printf("RDM Packet length : %d\n", data_length);
	uint8_t i = 0;
	for (i = 0; i < 9; i++)
	{
		printf("%.2d-%.4d:%.2X  %.2d-%.4d:%.2X %.2d-%.4d:%.2X  %.2d-%.4d:%.2X\n",
				i+1, widget_data[i], widget_data[i],
				i+10, widget_data[i+9], widget_data[i+9],
				i+19, widget_data[i+18], widget_data[i+18],
				i+28, widget_data[i+27], widget_data[i+27]);
	}
}

/**
 * @ingroup widget
 *
 * This function is called from the poll table in \file main.c
 *
 */
void widget_rdm_timeout(void)
{
	if (widget_send_rdm_packet_start == 0)
		return;

	if (bcm2835_st_read() - widget_send_rdm_packet_start > 1000000) {
		rdm_time_out_message();
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
static void widget_receive_dmx_on_change(void)
{
	dmx_port_direction_set(DMX_PORT_DIRECTION_INP, FALSE);
	receive_dmx_on_change = widget_data[0];
	uint16_t i = 0;
	for (i = 0; i < sizeof(dmx_data); i++)
		dmx_data[i] = 0;
	dmx_port_direction_set(DMX_PORT_DIRECTION_INP, TRUE);
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
void widget_received_dmx_change_of_state_packet(void)
{
	if ((rdm_discovery_running == TRUE) || (DMX_PORT_DIRECTION_INP != dmx_port_direction_get()) || (SEND_ALWAYS == receive_dmx_on_change))
		return;
	// TODO widget_received_dmx_change_of_state_packet
	console_clear_line(MONITOR_LINE_INFO);
	printf("Send changed DMX data to HOST\n");
}

/**
 * @ingroup widget
 *
 * Get Widget Serial Number Reply (Label = 10 \ref GET_WIDGET_PARAMS_REPLY)
 *
 */
static void widget_get_sn_reply(void)
{
	struct _widget_sn widget_sn;
	widget_params_sn_get(&widget_sn);
	usb_send_message(GET_WIDGET_SN_REPLY, (uint8_t *)&widget_sn, sizeof(struct _widget_sn));
	dmx_port_direction_set(DMX_PORT_DIRECTION_INP, TRUE);
}

/**
 * @ingroup widget
 *
 * Send RDM Discovery Request (Label=11 \ref SEND_RDM_DISCOVERY_REQUEST)
 *
 * This message requests the Widget to send an RDM Discovery Request packet out of the Widget
 * DMX port, and then receive an RDM Discovery Response (see Received DMX Packet \ref received_dmx_packet).
 */
static void widget_send_rdm_discovery_request(uint16_t data_length)
{
	console_clear_line(MONITOR_LINE_STATUS);
	printf("send_rdm_discovery_request\n");

	dmx_port_direction_set(DMX_PORT_DIRECTION_INP, FALSE);
	rdm_data_send(widget_data, data_length);
	dmx_port_direction_set(DMX_PORT_DIRECTION_INP, TRUE);

	widget_send_rdm_packet_start =  bcm2835_st_read();

	// TODO DEBUG
	console_clear_line(MONITOR_LINE_RDM_DATA);
	printf("RDM Packet length : %d\n", data_length);
	uint8_t i = 0;
	for (i = 0; i < 9; i++)
	{
		printf("%.2d-%.4d:%.2X  %.2d-%.4d:%.2X %.2d-%.4d:%.2X  %.2d-%.4d:%.2X\n",
				i+1, widget_data[i], widget_data[i],
				i+10, widget_data[i+9], widget_data[i+9],
				i+19, widget_data[i+18], widget_data[i+18],
				i+28, widget_data[i+27], widget_data[i+27]);
	}
}

/**
 *
 * (Label=12 \ref RDM_TIMEOUT)
 *
 */
inline static void rdm_time_out_message(void)
{
	console_clear_line(MONITOR_LINE_STATUS);
	printf("rdm_time_out_message\n");

	const uint16_t message_length = 0;

	console_clear_line(MONITOR_LINE_INFO);
	printf("Send RDM data to HOST, message_length : %d\n", message_length);

	usb_send_header(RDM_TIMEOUT, message_length);
	usb_send_footer();

	widget_send_rdm_packet_start = 0;
}

/**
 * @ingroup widget
 *
 * Get Widget Manufacturer Reply (Label = 77 \ref MANUFACTURER_LABEL)
 */
static void widget_get_manufacturer_reply(void)
{
	usb_send_header(MANUFACTURER_LABEL, sizeof(DEVICE_MANUFACTURER_ID) + sizeof(DEVICE_MANUFACTURER_NAME));
	usb_send_data(DEVICE_MANUFACTURER_ID, sizeof(DEVICE_MANUFACTURER_ID));
	usb_send_data(DEVICE_MANUFACTURER_NAME, sizeof(DEVICE_MANUFACTURER_NAME));
	usb_send_footer();
	dmx_port_direction_set(DMX_PORT_DIRECTION_INP, TRUE);
}

/**
 * @ingroup widget
 *
 * Get Widget Name Reply (Label = 78 \ref GET_WIDGET_NAME_LABEL)
 */
static void widget_get_name_reply(void)
{
	usb_send_header(GET_WIDGET_NAME_LABEL, sizeof(DEVICE_ID) + sizeof(DEVICE_NAME));
	usb_send_data(DEVICE_ID, sizeof(DEVICE_ID));
	usb_send_data(DEVICE_NAME, sizeof(DEVICE_NAME));
	usb_send_footer();
	dmx_port_direction_set(DMX_PORT_DIRECTION_INP, TRUE);
}

/**
 * @ingroup widget
 *
 * This function is called from the poll table in \file main.c
 */
void widget_ouput_dmx(void){
	if (widget_dmx_output_only == FALSE)
		return;

	if(bcm2835_st_read() < widget_dmx_output_elapsed_time + widget_dmx_output_period)
		return;

	dmx_data_send(dmx_data, widget_dmx_output_data_length);

	widget_dmx_output_elapsed_time += widget_dmx_output_period;
}

/**
 * @ingroup widget
 *
 */
void widget_receive_data_from_host(void)
{
	if (usb_read_is_byte_available())
	{
		uint8_t c = usb_read_byte();

		if (AMF_START_CODE == c)
		{
			uint8_t label = usb_read_byte();	// Label
			uint8_t lsb = usb_read_byte();	// Data length LSB
			uint8_t msb = usb_read_byte();	// Data length MSB
			uint16_t data_length = ((uint16_t) ((uint16_t) msb << 8)) | ((uint16_t) lsb);

			uint16_t i;
			for (i = 0; i < data_length; i++)
			{
				widget_data[i] = usb_read_byte();
			}

			while ((AMF_END_CODE != usb_read_byte()) && (i++ < sizeof(widget_data)));

			console_clear_line(MONITOR_LINE_LABEL);
			printf("L:%d:%d(%d)\n", label, data_length, i);

			widget_dmx_output_only = (label == OUTPUT_ONLY_SEND_DMX_PACKET_REQUEST);

			switch (label)
			{
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
				widget_output_only_send_dmx_packet_request(data_length);
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

