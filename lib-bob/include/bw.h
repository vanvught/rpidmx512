/**
 * @file bw.h
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

#ifndef BW_H_
#define BW_H_

#define BW_LCD_DEFAULT_SLAVE_ADDRESS		0x82	///< http://www.bitwizard.nl/wiki/index.php/Default_addresses
 	#define BW_LCD_MAX_CHARACTERS			16
	#define BW_LCD_MAX_LINES				2

#define BW_RELAY_DEFAULT_SLAVE_ADDRESS		0x8E	///< http://www.bitwizard.nl/wiki/index.php/Default_addresses
	typedef enum { 	 /// http://www.bitwizard.nl/wiki/index.php/Relay
		BW_RELAY_0 = (1 << 0),	///< 0b00000001, relay 1
		BW_RELAY_1 = (1 << 1)	///< 0b00000010, relay 2
	} bw_spi_relay_Pin;

#define BW_UI_DEFAULT_SLAVE_ADDRESS			0x94	///< http://www.bitwizard.nl/wiki/index.php/Default_addresses
	#define BW_UI_MAX_CHARACTERS			16
	#define BW_UI_MAX_LINES					2

	typedef enum {
		BW_UI_BUTTON1 = 0,//!< BW_UI_BUTTON1
		BW_UI_BUTTON2 = 1,//!< BW_UI_BUTTON2
		BW_UI_BUTTON3 = 2,//!< BW_UI_BUTTON3
		BW_UI_BUTTON4 = 3,//!< BW_UI_BUTTON4
		BW_UI_BUTTON5 = 4,//!< BW_UI_BUTTON5
		BW_UI_BUTTON6 = 5 //!< BW_UI_BUTTON6
	} BwUiButtons;

	#define BW_BUTTON6_PRESSED(x)		((x) & (1 << 0))	//((x) & 0b000001)
	#define BW_BUTTON5_PRESSED(x)		((x) & (1 << 1))	//((x) & 0b000010)
	#define BW_BUTTON4_PRESSED(x)		((x) & (1 << 2))	//((x) & 0b000100)
	#define BW_BUTTON3_PRESSED(x)		((x) & (1 << 3))	//((x) & 0b001000)
	#define BW_BUTTON2_PRESSED(x)		((x) & (1 << 4))	//((x) & 0b010000)
	#define BW_BUTTON1_PRESSED(x)		((x) & (1 << 5))	//((x) & 0b100000)

#define BW_PORT_WRITE_DISPLAY_DATA			0x00	///< display data
#define BW_PORT_WRITE_COMMAND				0x01	///< write data as command
#define BW_PORT_WRITE_STARTUPMESSAGE_LINE1	0x08	///< Set startup message line 1
#define BW_PORT_WRITE_STARTUPMESSAGE_LINE2	0x09	///< Set startup message line 2
#define BW_PORT_WRITE_STARTUPMESSAGE_LINE3	0x0a	///< Set startup message line 3
#define BW_PORT_WRITE_STARTUPMESSAGE_LINE4	0x0b	///< Set startup message line 4
#define BW_PORT_WRITE_CLEAR_SCREEN			0x10	///< any data clears the screen
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

#define BW_PORT_WRITE_ADC_SET_CHANNEL0		0x70
#define BW_PORT_WRITE_ADC_SET_CHANNEL1		0x71
#define BW_PORT_WRITE_ADC_SET_CHANNELS_READ	0x80
#define BW_PORT_WRITE_ADC_SET_SAMPLES		0x81
#define BW_PORT_WRITE_ADC_SET_SHIFT			0x82

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

#define BW_PORT_READ_ADC_CHANNEL0			0x60
#define BW_PORT_READ_ADC_CHANNEL1			0x61

#define BW_PORT_READ_ADC_CHANNEL0_AVG		0x68
#define BW_PORT_READ_ADC_CHANNEL1_AVG 		0x69

#define BW_PORT_WRITE_DIMMER				0x10	///< intensity

#define BW_ID_STRING_LENGTH					20

#endif /* BW_H_ */
