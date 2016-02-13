/**
 * @file widget.h
 *
 * @brief DMX USB Pro Widget API Specification 1.44
 *
 * https://wiki.openlighting.org/index.php/USB_Protocol_Extensions
 *
 */
/* Copyright (C) 2015, 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef WIDGET_H_
#define WIDGET_H_

#include <stdint.h>

typedef enum {
	GET_WIDGET_PARAMS = 3,						///< Get Widget Parameters Request
	GET_WIDGET_PARAMS_REPLY = 3,				///< Get Widget Parameters Reply
	SET_WIDGET_PARAMS = 4,						///< Set Widget Parameters Request
	RECEIVED_DMX_PACKET = 5,					///< Received DMX Packet
	OUTPUT_ONLY_SEND_DMX_PACKET_REQUEST = 6,	///< Output Only Send DMX Packet Request
	SEND_RDM_PACKET_REQUEST = 7,				///< Send RDM Packet Request
	RECEIVE_DMX_ON_CHANGE = 8,					///< Receive DMX on Change
	RECEIVED_DMX_COS_TYPE = 9,					///< Received DMX Change Of State Packet
	GET_WIDGET_SN_REQUEST = 10,					///< Get Widget Serial Number Request
	GET_WIDGET_SN_REPLY = 10,					///< Get Widget Serial Number Reply
	SEND_RDM_DISCOVERY_REQUEST = 11,			///< Send RDM Discovery Request
	RDM_TIMEOUT = 12,							///< https://github.com/OpenLightingProject/ola/blob/master/plugins/usbpro/EnttecUsbProWidget.cpp#L353
	MANUFACTURER_LABEL = 77,					///< https://wiki.openlighting.org/index.php/USB_Protocol_Extensions
	GET_WIDGET_NAME_LABEL = 78					///< https://wiki.openlighting.org/index.php/USB_Protocol_Extensions
} _widget_codes;

typedef enum {
	AMF_START_CODE = 0x7E,						///< Start of message delimiter
	AMF_END_CODE = 0xE7							///< End of message delimiter
} _application_message_format_codes;

typedef enum {
	SEND_ALWAYS = 0,							///< The widget will always send (default)
	SEND_ON_DATA_CHANGE_ONLY = 1				///< Requests the Widget to send a DMX packet to the host only when the DMX values change on the input port
} _widget_send_state;

typedef enum {
	MODE_DMX_RDM = 0,							///< Both DMX (\ref FIRMWARE_NORMAL_DMX)and RDM (\ref FIRMWARE_RDM) firmware enabled.
	MODE_DMX = 1,								///< DMX (\ref FIRMWARE_NORMAL_DMX) firmware enabled
	MODE_RDM = 2,								///< RDM (\ref FIRMWARE_RDM) firmware enabled.
	MODE_RDM_SNIFFER = 3						///< RDM Sniffer firmware enabled.
} _widget_mode;

extern const _widget_send_state widget_get_receive_dmx_on_change(void);
extern const _widget_mode widget_get_mode(void);
extern void widget_set_mode(const _widget_mode);
extern const uint32_t widget_get_received_dmx_packet_period(void);
extern void widget_set_received_dmx_packet_period(uint32_t);
extern const uint32_t widget_get_received_dmx_packet_count(void);
// poll table
extern void widget_receive_data_from_host(void);
extern void widget_received_dmx_packet(void);
extern void widget_received_dmx_change_of_state_packet(void);
extern void widget_received_rdm_packet(void);
extern void widget_rdm_timeout(void);
extern void widget_sniffer_rdm(void);
extern void widget_sniffer_dmx(void);
extern void widget_sniffer_fill_transmit_buffer(void);

#endif /* WIDGET_H_ */
