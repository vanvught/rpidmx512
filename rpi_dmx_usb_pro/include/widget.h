/**
 * @file widget.h
 *
 * @brief DMX USB Pro Widget API Specification 1.44
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

#ifndef WIDGET_H_
#define WIDGET_H_

typedef enum
{
	GET_WIDGET_PARAMS = 3,
	GET_WIDGET_PARAMS_REPLY = 3,
	SET_WIDGET_PARAMS = 4,
	RECEIVED_DMX_PACKET = 5,
	OUTPUT_ONLY_SEND_DMX_PACKET_REQUEST = 6,
	SEND_RDM_PACKET_REQUEST = 7,
	RECEIVE_DMX_ON_CHANGE = 8,
	RECEIVED_DMX_COS_TYPE = 9,
	GET_WIDGET_SN_REQUEST = 10,
	GET_WIDGET_SN_REPLY = 10,
	SEND_RDM_DISCOVERY_REQUEST = 11,
	RDM_TIMEOUT = 12,
	MANUFACTURER_LABEL = 77,						///< https://wiki.openlighting.org/index.php/USB_Protocol_Extensions
	GET_WIDGET_NAME_LABEL = 78						///< https://wiki.openlighting.org/index.php/USB_Protocol_Extensions
} _widget_codes;

typedef enum
{
	AMF_START_CODE = 0x7E,		///< Start of message delimiter
	AMF_END_CODE = 0xE7			///< End of message delimiter
} _application_message_format_codes;

typedef enum
{
	SEND_ALWAYS = 0,				///<
	SEND_ON_DATA_CHANGE_ONLY = 1	///<
} _widget_send_state;

extern uint8_t receive_dmx_on_change_get(void);
extern uint64_t widget_dmx_output_period_get(void);
extern void widget_dmx_output_period_set(uint64_t);

#endif /* WIDGET_H_ */
