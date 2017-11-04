/**
 * @file bw_7fets.h
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef BW_7FETS_H_
#define BW_7FETS_H_

#define BW_7FETS_DEFAULT_SLAVE_ADDRESS	0x88	///< http://www.bitwizard.nl/wiki/index.php/Default_addresses

typedef enum {
	BW_7FETS_PIN_IO0 = (1 << 0),	///< 0b00000001, Pin2 	OUT0
	BW_7FETS_PIN_IO1 = (1 << 1),	///< 0b00000010, Pin4 	OUT1
	BW_7FETS_PIN_IO2 = (1 << 2),	///< 0b00000100, Pin6 	OUT2
	BW_7FETS_PIN_IO3 = (1 << 3),	///< 0b00001000, Pin8 	OUT3
	BW_7FETS_PIN_IO4 = (1 << 4),	///< 0b00010000, Pin10 	OUT4
	BW_7FETS_PIN_IO5 = (1 << 5),	///< 0b00100000, Pin12	OUT5
	BW_7FETS_PIN_IO6 = (1 << 6),	///< 0b01000000	 Pin14	OUT6
} bw_spi_7fets_Pin;

#endif /* BW_7FETS_H_ */
