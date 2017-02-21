/**
 * @file max7219.h
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef MAX7219_H_
#define MAX7219_H_

#define MAX7219_OK					0
#define MAX7219_ERROR				1

// https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf

#define MAX7219_REG_NOOP			0x00	///<
#define MAX7219_REG_DIGIT0			0x01	///<
#define MAX7219_REG_DIGIT1			0x02	///<
#define MAX7219_REG_DIGIT2			0x03	///<
#define MAX7219_REG_DIGIT3			0x04	///<
#define MAX7219_REG_DIGIT4			0x05	///<
#define MAX7219_REG_DIGIT5			0x06	///<
#define MAX7219_REG_DIGIT6			0x07	///<
#define MAX7219_REG_DIGIT7			0x08	///<
#define MAX7219_REG_DECODE_MODE		0x09	///<
	#define MAX7219_DECODE_MODE_CODEB	0xFF	///< Code B decode for digits 7–0
#define MAX7219_REG_INTENSITY		0x0A	///<
#define MAX7219_REG_SCAN_LIMIT		0x0B	///<
#define MAX7219_REG_SHUTDOWN		0x0C	///<
	#define MAX7219_SHUTDOWN_MODE		0x00	///<
	#define MAX7219_SHUTDOWN_NORMAL_OP	0x01	///<
#define MAX7219_REG_DISPLAY_TEST	0x0F	///<

#define MAX7219_CHAR_NEGATIVE		0x0A	///<
#define MAX7219_CHAR_E				0x0B	///<
#define MAX7219_CHAR_H				0x0C	///<
#define MAX7219_CHAR_L				0x0D	///<
#define MAX7219_CHAR_P				0x0E	///<
#define MAX7219_CHAR_BLANK			0x0F	///<


#endif /* MAX7219_H_ */
