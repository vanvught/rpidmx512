/**
 * @file sc16is740.h
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef SC16IS740_H_
#define SC16IS740_H_

#include <stdbool.h>

#include "sc16is7x0.h"

#include "device_info.h"

extern void sc16is740_start(device_info_t *);
extern bool sc16is740_is_connected(const device_info_t *);

extern uint8_t sc16is740_reg_read(const device_info_t *, const uint8_t);
extern void sc16is740_reg_write(const device_info_t *, const uint8_t, const uint8_t);

extern void sc16is740_set_baud(const device_info_t *, const int);
extern void sc16is740_set_format(const device_info_t *, int, _serial_parity,  int);

extern bool sc16is740_is_readable(const device_info_t *);
extern int sc16is740_read(const device_info_t *, void *, unsigned);
extern int sc16is740_getc(const device_info_t *);

extern bool sc16is740_is_writable(const device_info_t *);
extern int sc16is740_write(const device_info_t *, const void *, unsigned);
extern int sc16is740_putc(const device_info_t *, const int);

#endif /* SC16IS740_H_ */
