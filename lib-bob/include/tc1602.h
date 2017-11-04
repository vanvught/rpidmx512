/**
 * @file tc1602.h
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

#ifndef TC1602_H_
#define TC1602_H_

#define TC1602_RS 			(1<<0)	///< 0b00000001 # Register select bit
#define TC1602_RW 			(1<<1)	///< 0b00000010 # Read/Write bit
#define TC1602_EN			(1<<2)	///< 0b00000100 # Enable bit
#define TC1602_BACKLIGHT	(1<<3)	///< 0b00001000 # Backlight bit
	#define TC1602_NOBACKLIGHT	0x00

#define TC1602_LINE_1		0x80	///< LCD RAM address for the 1st line
#define TC1602_LINE_2		0xC0	///< LCD RAM address for the 2nd line

#define EXEC_TIME_CMD		37		///< 37us
#define EXEC_TIME_REG		43		///< 43us
#define EXEC_TIME_CLS		1520	///< 1.52ms

/// Instruction Code
/// http://www.oppod.com/upload/download/20111224105943_57765.pdf
/// 1. Clear Display
#define TC1602_IC_CLS			(1<<0)	///< Clear all the display data by writing "20H" (space code) to all DDRAM address.
/// 2. Return Home
#define TC1602_IC_RETURNHOME	(1<<1)	///< Return cursor to its original site and return display to its original status, if shifted. Contents of DDRAM do not change.
/// 3. Entry Mode Set
#define TC1602_IC_ENTRY_MODE	(1<<2)	///< Set the moving direction of cursor and display.
	#define TC1602_IC_ENTRY_MODE_SH		(1<<0)	///< Shift of entire display
	#define TC1602_IC_ENTRY_MODE_DEC		0	///< cursor/blink moves to left and DDRAM address is decreased by 1.
	#define TC1602_IC_ENTRY_MODE_INC	(1<<1)	///< cursor/blink moves to right and DDRAM address is increased by 1.
/// 4. Display ON/OFF Control
#define TC1602_IC_DISPLAY		(1<<3)	///< Set display(D), cursor(C), and blinking of cursor(B) on/off control bit
 	#define TC1602_IC_DISPLAY_BLINK_OFF		0	///< Cursor blink is off.
	#define TC1602_IC_DISPLAY_BLINK_ON	(1<<0)	///< Cursor blink is on, that performs alternate between all the high data and display character at the cursor position.
	#define TC1602_IC_DISPLAY_CURSOR_OFF	0	///< Cursor is disappeared in current display, but I/D register remains its data.
	#define TC1602_IC_DISPLAY_CURSOR_ON	(1<<1)	///< Cursor is turned on.
 	#define TC1602_IC_DISPLAY_OFF			0	///< The display is turned off, but display data is remained in DDRAM.
	#define TC1602_IC_DISPLAY_ON		(1<<2)	///< The entire display is turned on.
/// 6. Function Set
#define TC1602_IC_FUNC			(1<<5)	///< Set interface data length, numbers of display lines, display font type
	#define TC1602_IC_FUNC_4BIT			0	///< 4-bit bus mode with MPU.
	#define TC1602_IC_FUNC_8BIT		(1<<4)	///< 8-bit bus mode with MPU.
	#define TC1602_IC_FUNC_1LINE		0	///< 1-line display mode.
	#define TC1602_IC_FUNC_2LINE	(1<<3)	///< 2-line display mode is set.
	#define TC1602_IC_FUNC_5x8DOTS		0	///< 5 x 8 dots format display mode.
	#define TC1602_IC_FUNC_5x11DOTS	(1<<2)	///< 5 x11 dots format display mode.

#endif /* TC1602_H_ */
