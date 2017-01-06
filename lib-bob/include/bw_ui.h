/**
 * @file bw_ui.h
 *
 */
/* Copyright (C) 2016, 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef BW_UI_H_
#define BW_UI_H_

#include <stdint.h>

#define BW_UI_DEFAULT_SLAVE_ADDRESS		0x94	///< http://www.bitwizard.nl/wiki/index.php/Default_addresses

#define BW_UI_OK 						0
#define BW_UI_ERROR						1

#define BW_UI_MAX_CHARACTERS			16
#define BW_UI_MAX_LINES					2

typedef enum {
	BW_UI_BUTTON1 = 0,	///<
	BW_UI_BUTTON2 = 1,	///<
	BW_UI_BUTTON3 = 2,	///<
	BW_UI_BUTTON4 = 3,	///<
	BW_UI_BUTTON5 = 4,	///<
	BW_UI_BUTTON6 = 5	///<
} BwUiButtons;

#define BUTTON6_PRESSED(x)		((x) & (1 << 0))	//((x) & 0b000001)
#define BUTTON5_PRESSED(x)		((x) & (1 << 1))	//((x) & 0b000010)
#define BUTTON4_PRESSED(x)		((x) & (1 << 2))	//((x) & 0b000100)
#define BUTTON3_PRESSED(x)		((x) & (1 << 3))	//((x) & 0b001000)
#define BUTTON2_PRESSED(x)		((x) & (1 << 4))	//((x) & 0b010000)
#define BUTTON1_PRESSED(x)		((x) & (1 << 5))	//((x) & 0b100000)

#endif /* BW_UI_H_ */
