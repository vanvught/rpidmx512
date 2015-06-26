/**
 * @file bw_ui.h
 *
 */
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

#ifndef BW_UI_H_
#define BW_UI_H_

#include <stdint.h>
//#include "device_info.h"

#define BW_UI_DEFAULT_SLAVE_ADDRESS		(uint8_t)0x94

#define BW_UI_OK 						0
#define BW_UI_ERROR						1

#define BW_UI_MAX_CHARACTERS			16
#define BW_UI_MAX_LINES					2

typedef enum
{
	BW_UI_BUTTON1	= 0,	///<
	BW_UI_BUTTON2	= 1,	///<
	BW_UI_BUTTON3	= 2,	///<
	BW_UI_BUTTON4	= 3,	///<
	BW_UI_BUTTON5	= 4,	///<
	BW_UI_BUTTON6	= 5		///<
} BwUiButtons;

#define BUTTON6_PRESSED(x)		((x) & 0b000001)
#define BUTTON5_PRESSED(x)		((x) & 0b000010)
#define BUTTON4_PRESSED(x)		((x) & 0b000100)
#define BUTTON3_PRESSED(x)		((x) & 0b001000)
#define BUTTON2_PRESSED(x)		((x) & 0b010000)
#define BUTTON1_PRESSED(x)		((x) & 0b100000)

#ifdef BW_I2C_UI
#define ui_start 						bw_i2c_ui_start
#define ui_end							bw_i2c_ui_end
#define ui_reinit						bw_i2c_ui_reinit
#define ui_set_cursor					bw_i2c_ui_set_cursor
#define ui_text							bw_i2c_ui_text
#define ui_text_line_1					bw_i2c_ui_text_line_1
#define ui_text_line_2					bw_i2c_ui_text_line_2
#define ui_text_line_3					bw_i2c_ui_text_line_3
#define ui_text_line_4					bw_i2c_ui_text_line_4
#define	 ui_cls							bw_i2c_ui_cls
#define ui_set_contrast					bw_i2c_ui_set_contrast
#define ui_set_backlight				bw_i2c_ui_set_backlight
#define ui_set_backlight_temp			bw_i2c_ui_set_backlight_temp
#define ui_set_startup_message_line_1	bw_i2c_ui_set_startup_message_line_1
#define ui_set_startup_message_line_2	bw_i2c_ui_set_startup_message_line_2
#define ui_set_startup_message_line_3	bw_i2c_ui_set_startup_message_line_3
#define ui_set_startup_message_line_4	bw_i2c_ui_set_startup_message_line_4
#define ui_get_backlight				bw_i2c_ui_get_backlight
#define ui_get_contrast					bw_i2c_ui_get_contrast
#define ui_read_id						bw_i2c_ui_read_id
#define ui_read_button					bw_i2c_ui_read_button
#define ui_read_button_last				bw_i2c_ui_read_button_last
#endif

extern uint8_t ui_start(const uint8_t);
extern void ui_end(void);
extern void ui_reinit(void);
extern void ui_set_cursor(uint8_t, uint8_t);
extern void ui_text(const char *, uint8_t);
extern void ui_text_line_1(const char *, uint8_t);
extern void ui_text_line_2(const char *, uint8_t);
extern void ui_text_line_3(const char *, uint8_t);
extern void ui_text_line_4(const char *, uint8_t);
extern void ui_cls(void);
extern void ui_set_contrast(uint8_t);
extern void ui_set_backlight(uint8_t);
extern void ui_set_backlight_temp(uint8_t);
extern void ui_set_startup_message_line_1(const char *, uint8_t);
extern void ui_set_startup_message_line_2(const char *, uint8_t);
extern void ui_set_startup_message_line_3(const char *, uint8_t);
extern void ui_set_startup_message_line_4(const char *, uint8_t);
extern void ui_get_backlight(uint8_t *);
extern void ui_get_contrast(uint8_t *);
extern void ui_read_id(void);
// UI specific
extern char ui_read_button(const BwUiButtons);
extern char ui_read_button_last(void);

#endif /* BW_UI_H_ */
