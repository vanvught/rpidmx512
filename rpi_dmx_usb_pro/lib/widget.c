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

#include "widget.h"
#include "dmx_data.h"
#include "ft245rl.h"

#include "console.h"

extern uint8_t dmx_data[512];

static uint8_t receive_dmx_on_change = SEND_ALWAYS;
static uint8_t widget_data[600];

struct _DMXUSBPRO_params
{
	uint8_t firmware_lsb;
	uint8_t firmware_msb;
	uint8_t break_time;
	uint8_t mab_time;
	uint8_t refresh_rate;
} static DMXUSBPRO_params = { 4, FIRMWARE_RDM, 9, 1, 40 };

struct _DMXUSBPRO_sn
{
	uint8_t bcd_0;
	uint8_t bcd_1;
	uint8_t bcd_2;
	uint8_t bcd_3;
} static const DMXUSBPRO_SN = { DEC2BCD(0), DEC2BCD(0), DEC2BCD(0), DEC2BCD(0) };

static const uint8_t DEVICE_MANUFACTURER_ID[] = {0x00, 0x00};
static const uint8_t DEVICE_MANUFACTURER_NAME[] = "Arjan van Vught";
static const uint8_t DEVICE_NAME[] = "Raspberry Pi DMX/RDM Controller";
static const uint8_t DEVICE_ID[] = {1, 0};

/**
 *
 * @param label
 * @param length
 */
static void send_header(const uint8_t label, const uint16_t length)
{
	FT245RL_write_data(AMF_START_CODE);
	FT245RL_write_data(label);
	FT245RL_write_data((uint8_t)(length & 0x00FF));
	FT245RL_write_data((uint8_t)(length >> 8));
}

/**
 *
 * @param data
 * @param length
 */
static void send_data(const uint8_t *data, const uint16_t length)
{
	uint16_t i;
	for (i = 0; i < length; i++)
	{
		FT245RL_write_data(data[i]);
	}
}

/**
 *
 */
static void send_footer(void)
{
	FT245RL_write_data(AMF_END_CODE);
}

/**
 *
 * @param label
 * @param data
 * @param length
 */
static void send_message(const uint8_t label, const uint8_t *data, const uint16_t length)
{
	send_header(label, length);
	send_data(data, length);
	send_footer();
}

/**
 * TODO
 */
void widget_print_parms(void)
{
	char dir[3][10] = {"Idle", "Output", "Input"};

	printf("Firmware %d.%d BreakTime %d MaBTime %d RefreshRate %d\n",
			DMXUSBPRO_params.firmware_msb, DMXUSBPRO_params.firmware_lsb,
			DMXUSBPRO_params.break_time, DMXUSBPRO_params.mab_time, DMXUSBPRO_params.refresh_rate);

	if (DMX_PORT_DIRECTION_INP == dmx_port_direction_get())
		printf("Input [%s]\n", receive_dmx_on_change == SEND_ALWAYS ? "SEND_ALWAYS" : "SEND_ON_DATA_CHANGE_ONLY");
	else
		printf("%s                                     \n", dir[dmx_port_direction_get()]);
}

/**
 * @ingroup widget
 *
 * Get Widget Parameters Reply (Label=3 \ref GET_WIDGET_PARAMS_REPLY)
 * The Widget sends this message to the PC in response to the Get Widget Parameters request.
 */
static void widget_get_params_reply(void)
{
	send_message(GET_WIDGET_PARAMS_REPLY, (uint8_t *)&DMXUSBPRO_params, sizeof(struct _DMXUSBPRO_params));
}

/**
 * @ingroup widget
 *
 * Get Widget Serial Number Reply (Label = 10 \ref GET_WIDGET_PARAMS_REPLY)
 *
 */
static void widget_get_sn_reply(void)
{
	send_message(GET_WIDGET_SN_REPLY, (uint8_t *)&DMXUSBPRO_SN, sizeof(struct _DMXUSBPRO_sn));
}

/**
 *
 */
static void widget_get_manufacturer_reply(void)
{
	send_header(MANUFACTURER_LABEL, sizeof(DEVICE_MANUFACTURER_ID) + sizeof(DEVICE_MANUFACTURER_NAME));
	send_data(DEVICE_MANUFACTURER_ID, sizeof(DEVICE_MANUFACTURER_ID));
	send_data(DEVICE_MANUFACTURER_NAME, sizeof(DEVICE_MANUFACTURER_NAME));
	send_footer();
}

/**
 *
 */
static void widget_get_name_reply(void)
{
	send_header(GET_WIDGET_NAME_LABEL, sizeof(DEVICE_ID) + sizeof(DEVICE_NAME));
	send_data(DEVICE_ID, sizeof(DEVICE_ID));
	send_data(DEVICE_NAME, sizeof(DEVICE_NAME));
	send_footer();
}

/**
 * @ingroup widget
 *
 * Set Widget Parameters Request (Label=4 \ref SET_WIDGET_PARAMS)
 * This message sets the Widget configuration. The Widget configuration is preserved when the Widget loses power.
 *
 * TODO use RTC RAM (battery)
 */
static void widget_set_params()
{
	DMXUSBPRO_params.break_time = widget_data[2];
	DMXUSBPRO_params.mab_time = widget_data[3];
	DMXUSBPRO_params.refresh_rate = widget_data[4];
	widget_print_parms();
}

/**
 * @ingroup widget
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
	dmx_port_direction_set(DMX_PORT_DIRECTION_OUTP, 1);

	uint16_t i = 0;
	for (i = 1; i < data_length; i++)
		dmx_data[i - 1] = widget_data[i];
}

/**
 * @ingroup widget
 *
 * This message requests the Widget to send an RDM packet out of the Widget DMX port, and then
 * change the DMX port direction to input, so that RDM or DMX packets can be received.
 *
 * @param data_length RDM data to send, beginning with the start code.
 */
static void widget_send_rdm_packet_request(const uint16_t data_length)
{
	dmx_port_direction_set(DMX_PORT_DIRECTION_OUTP, 0);
	dmx_data_send(widget_data, data_length);
	dmx_port_direction_set(DMX_PORT_DIRECTION_INP, 1);

	printf("RDM Packet length : %d\n", data_length);
#if 1
	uint16_t i = 0;
	for (i = 0; i < data_length; i++)
	{
		printf("%.2d-%.4d:%.2x ", i,widget_data[i], widget_data[i]);
	}
	printf("\n");
#endif

}

/**
 * @ingroup widget
 *
 * This message requests the Widget to send an RDM Discovery Request packet out of the Widget
 * DMX port, and then receive an RDM Discovery Response (see Received DMX Packet \ref received_dmx_packet).
 */
static void send_rdm_discovery_request(void)
{
	// TODO
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
	dmx_port_direction_set(DMX_PORT_DIRECTION_INP, 1);
	receive_dmx_on_change = widget_data[0];
}

/**
 * @ingroup widget
 *
 * Received DMX Packet (Label=5)
 * The Widget sends this message to the PC unsolicited, whenever the Widget receives a DMX or RDM packet from the DMX port,
 * and the Receive DMX on Change mode (\ref receive_dmx_on_change) is 'Send always' (\ref SEND_ALWAYS).
 */
void received_dmx_packet(void)
{
	if ((DMX_PORT_DIRECTION_INP != dmx_port_direction_get()) || (SEND_ON_DATA_CHANGE_ONLY == receive_dmx_on_change))
		return;

	console_set_cursor(0,6);
	printf("Send DMX data to PC\n");

	send_header(RECEIVED_DMX_PACKET, 2 + (sizeof(dmx_data) / sizeof(uint8_t)));
	FT245RL_write_data(0); 	// DMX Receive status
	FT245RL_write_data(0);	// DMX Start code
	send_data(dmx_data, sizeof(dmx_data) / sizeof(uint8_t));
	send_footer();
}

/**
 * @ingroup widget
 *
 * The Widget sends one or more instances of this message to the PC unsolicited, whenever the
 * Widget receives a changed DMX packet from the DMX port, and the Receive DMX on Change
 * mode (\ref receive_dmx_on_change) is 'Send on data change only' (\ref SEND_ON_DATA_CHANGE_ONLY).
 */
void received_dmx_change_of_state_packet(void)
{
	if ((DMX_PORT_DIRECTION_INP != dmx_port_direction_get()) || (SEND_ALWAYS == receive_dmx_on_change))
		return;

	console_set_cursor(0,6);
	printf("Send changed DMX data to PC\n");
}

/**
 * @ingroup usb_host
 *
 * @return
 */
static uint8_t read_data(void)
{
	while (!FT245RL_data_available());
	return FT245RL_read_data();
}

/**
 * @ingroup widget
 *
 */
void receive_data_from_host(void)
{
	if (FT245RL_data_available())
	{
		uint8_t c = read_data();

		if (AMF_START_CODE == c)
		{
			uint8_t label = read_data();	// Label
			uint8_t lsb = read_data();		// Data length LSB
			uint8_t msb = read_data();		// Data length MSB
			uint16_t data_length = ((uint16_t) ((uint16_t) msb << 8)) | ((uint16_t) lsb);

			uint16_t i;
			for (i = 0; i < data_length; i++)
			{
				widget_data[i] = read_data();
			}

			while (AMF_END_CODE != read_data());

			console_set_cursor(0,5);
			printf("\nL:%d:%d\n", label, data_length);

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
			default:
				break;
			}
		}
	}
}

