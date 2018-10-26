/**
 * @file usb.h
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

#ifndef USB_H_
#define USB_H_

#include <stdint.h>
#include <stdbool.h>

#include "ft245rl.h"

extern void usb_send_byte(uint8_t);

/**
 *
 * Wait for data is available in the FIFO which can be read by strobing RD# low, then high again.
 *
 */
inline static const uint8_t usb_read_byte(void) {
	while (!FT245RL_data_available())
		;
	return FT245RL_read_data();
}

inline static const bool usb_read_is_byte_available(void) {
	return FT245RL_data_available();
}

inline static void usb_init(void) {
	FT245RL_init();
}

inline static const bool usb_can_write(void) {
	return FT245RL_can_write();
}

#endif /* USB_H_ */
