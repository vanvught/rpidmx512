/**
 * @file widget_usb.c
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

#include "widget.h"
#include "usb.h"

void widget_usb_send_header(uint8_t label, uint16_t length) {
	usb_send_byte(AMF_START_CODE);
	usb_send_byte(label);
	usb_send_byte((uint8_t) (length & 0x00FF));
	usb_send_byte((uint8_t) (length >> 8));
}

void widget_usb_send_data(const uint8_t *data, uint16_t length) {
	uint32_t i;
	for (i = 0; i < length; i++) {
		usb_send_byte(data[i]);
	}
}

void widget_usb_send_footer(void) {
	usb_send_byte(AMF_END_CODE);
}

void widget_usb_send_message(uint8_t label, const uint8_t *data, uint16_t length) {
	widget_usb_send_header(label, length);
	widget_usb_send_data(data, length);
	widget_usb_send_footer();
}
