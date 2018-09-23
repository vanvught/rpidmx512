/**
 * @file console.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef RPI_CONSOLE_H_
#define RPI_CONSOLE_H_

// some RGB color definitions
typedef enum {
	CONSOLE_BLACK = 0x0000,		///<   0,   0,   0
	CONSOLE_BLUE = 0x001F,		///<   0,   0, 255
	CONSOLE_GREEN = 0x07E0,		///<   0, 255,   0
	CONSOLE_CYAN = 0x07FF,		///<   0, 255, 255
	CONSOLE_RED = 0xF800,		///< 255,   0,   0
	CONSOLE_YELLOW = 0xFFE0,	///< 255, 255,   0
	CONSOLE_WHITE = 0xFFFF,		///< 255, 255, 255
} _console_colors;

#endif /* RPI_CONSOLE_H_ */
