/**
 * @file widget_params.h
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

#ifndef WIDGET_PARAMS_H_
#define WIDGET_PARAMS_H_

#define DEVICE_TYPE_ID_LENGTH				2	///<

struct _widget_params
{
	uint8_t firmware_lsb;
	uint8_t firmware_msb;
	uint8_t break_time;
	uint8_t mab_time;
	uint8_t refresh_rate;
};

typedef enum
{
	FIRMWARE_NORMAL_DMX = 1,	///< Normal DMX firmware. Supports all messages except Send RDM (label=7), Send RDM Discovery Request(label=11) and receive RDM .
	FIRMWARE_RDM = 2,			///< RDM firmware.This enables the Widget to act as an RDM Controller or RDM responder. Supports all messages except Receive DMX On Change (label=8) and Change Of State Receive (label=9).
	FIRMWARE_RDM_SNIFFER = 3	///< RDM Sniffer firmware. This is for use with the Openlighting RDM packet monitoring application.
} _firmware_version_msb;

extern void widget_params_init(void);
extern void widget_params_get(struct _widget_params *);
extern void widget_params_break_time_set(uint8_t);
extern void widget_params_mab_time_set(uint8_t);
extern void widget_params_refresh_rate_set(uint8_t);
extern const uint8_t * widget_params_get_type_id(void);
extern const uint8_t widget_params_get_type_id_length(void);

#endif /* WIDGET_PARAMS_H_ */
