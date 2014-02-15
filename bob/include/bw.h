/* Copyright (C) 2014 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#ifndef BW_H_
#define BW_H_

#include <bcm2835.h>

/// write ports
#define BW_PORT_WRITE_DISPLAY_DATA			0x00
#define BW_PORT_WRITE_COMMAND				0x01
#define BW_PORT_WRITE_STARTUPMESSAGE_LINE1	0x08
#define BW_PORT_WRITE_STARTUPMESSAGE_LINE2	0x09
#define BW_PORT_WRITE_STARTUPMESSAGE_LINE3	0x0a
#define BW_PORT_WRITE_STARTUPMESSAGE_LINE4	0x0b
#define BW_PORT_WRITE_CLEAR_SCREEN			0x10
#define BW_PORT_WRITE_MOVE_CURSOR			0x11
#define BW_PORT_WRITE_SET_CONTRAST			0x12
#define BW_PORT_WRITE_SET_BACKLIGHT_TEMP	0x13
#define BW_PORT_WRITE_REINIT_LCD			0x14
#define BW_PORT_WRITE_SET_BACKLIGHT			0x17
#define BW_PORT_WRITE_CHANGE_SLAVE_ADDRESS	0xf0
//
#define BW_PORT_WRITE_SET_ALL_OUTPUTS		0x10
#define BW_PORT_WRITE_SET_OUTPUT_IO0		0x20
#define BW_PORT_WRITE_SET_OUTPUT_IO1		0x21
#define BW_PORT_WRITE_SET_OUTPUT_IO2		0x22
#define BW_PORT_WRITE_SET_OUTPUT_IO3		0x23
#define BW_PORT_WRITE_SET_OUTPUT_IO4		0x24
#define BW_PORT_WRITE_SET_OUTPUT_IO5		0x25
#define BW_PORT_WRITE_SET_OUTPUT_IO6		0x26
#define BW_PORT_WRITE_IO_DIRECTION			0x30

/// read ports
#define BW_PORT_READ_ID_STRING				0x01
#define BW_PORT_READ_EEPROM_SN				0x02
#define BW_PORT_READ_CURRENT_CONTRAST		0x12
#define BW_PORT_READ_CURRENT_BACKLIGHT		0x13
#define BW_PORT_READ_POWERUP_BACKLIGHT		0x17
#define BW_PORT_READ_BUTTON_SINCE_LAST		0x31
#define BW_PORT_READ_BUTTON_1				0x40
#define BW_PORT_READ_BUTTON_2				0x41
#define BW_PORT_READ_BUTTON_3				0x42
#define BW_PORT_READ_BUTTON_4				0x43
#define BW_PORT_READ_BUTTON_5				0x44
#define BW_PORT_READ_BUTTON_6				0x45

#define BW_SPI0			BCM2835_SPI_CS0
#define BW_SPI1			BCM2835_SPI_CS1

#endif /* BW_H_ */
