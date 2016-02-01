/**
 * @file widget_params.h
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

#ifndef WIDGET_PARAMS_H_
#define WIDGET_PARAMS_H_

#define DEVICE_TYPE_ID_LENGTH	2	///<

typedef enum {
	WIDGET_DEFAULT_FIRMWARE_LSB = 4	///< x.4
} _firmware_version_lsb;

typedef enum {
	FIRMWARE_NORMAL_DMX = 1,		///< Normal DMX firmware. Supports all messages except Send RDM (label=7), Send RDM Discovery Request(label=11) and receive RDM .
	FIRMWARE_RDM = 2,				///< RDM firmware. This enables the Widget to act as an RDM Controller.
	FIRMWARE_RDM_SNIFFER = 3		///< RDM Sniffer firmware. This is for use with the Openlighting RDM packet monitoring application.
} _firmware_version_msb;

typedef enum {
	WIDGET_MIN_BREAK_TIME = 9,		///<
	WIDGET_DEFAULT_BREAK_TIME = 9,	///<
	WIDGET_MAX_BREAK_TIME = 127		///<
} _firmware_break_time;

typedef enum {
	WIDGET_MIN_MAB_TIME = 1,		///<
	WIDGET_DEFAULT_MAB_TIME = 1,	///<
	WIDGET_MAX_MAB_TIME = 127,		///<
} _firmware_mab_time;

typedef enum {
	WIDGET_DEFAULT_REFRESH_RATE = 40///<
} _firmware_refresh_rate;

struct _widget_params {
	uint8_t firmware_lsb;			///< Firmware version LSB. Valid range is 0 to 255.
	uint8_t firmware_msb;			///< Firmware version MSB. Valid range is 0 to 255.
	uint8_t break_time;				///< DMX output break time in 10.67 microsecond units. Valid range is 9 to 127.
	uint8_t mab_time;				///< DMX output Mark After Break time in 10.67 microsecond units. Valid range is 1 to 127.
	uint8_t refresh_rate;			///< DMX output rate in packets per second. Valid range is 1 to 40.
};

struct _widget_params_data {
	/*@shared@*/uint8_t *data;
	uint8_t length;
};

extern void widget_params_init(void);
extern void widget_params_get(/*@out@*/struct _widget_params *);
extern void widget_params_set(/*@out@*/const struct _widget_params *);
extern void widget_params_get_type_id(struct _widget_params_data *);
extern const uint8_t widget_params_get_throttle(void);
extern void widget_params_set_throttle(const uint8_t);

#endif /* WIDGET_PARAMS_H_ */
