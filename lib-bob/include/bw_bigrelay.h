/**
 * @file bw_bigrelay.h
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

#ifndef BW_BIGRELAY_H_
#define BW_BIGRELAY_H_

#define BW_BIGRELAY_DEFAULT_SLAVE_ADDRESS	0x9C	///< http://www.bitwizard.nl/wiki/index.php/Default_addresses

/// http://www.bitwizard.nl/wiki/index.php/Relay
typedef enum {
	BW_BIGRELAY_0 = 0b00000001,	///< relay 1
	BW_BIGRELAY_1 = 0b00000010,	///< relay 2
	BW_BIGRELAY_2 = 0b00000100,	///< relay 3
	BW_BIGRELAY_3 = 0b00001000,	///< relay 4
	BW_BIGRELAY_4 = 0b00010000,	///< relay 5
	BW_BIGRELAY_5 = 0b00100000	///< relay 6
} bw_spi_bigrelay_Pin;

#endif /* BW_BIGRELAY_H_ */
