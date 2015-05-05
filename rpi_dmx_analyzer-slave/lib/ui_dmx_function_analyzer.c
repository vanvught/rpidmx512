/**
 * @file ui_function_analyzer.c
 *
 */
/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#include "bw_ui.h"
#include "ui_functions.h"
#include "dmx_data.h"
#include "util.h"

/**
 *
 * @param arg
 * @param buf
 */
inline static void itoa_base10(int arg, char buf[]) {
	char *n = buf + 2;

	if (arg == 0) *n = '0';

	while (arg) {
		*n = '0' + (arg % 10);
		n--;
		arg /= 10;
	}
}

/**
 * @ingroup ui
 *
 * @param start_channel
 */
inline static void display_channels(const int start_channel) {
	char text[BW_UI_MAX_CHARACTERS];
	int i = 0;
	int offset = 0;

	for (i = 0; i < 4; i++) {
		offset = i * 4;
		text[offset    ] = ' ';
		text[offset + 1] = ' ';
		text[offset + 3] = ' ';
		itoa_base10(start_channel + i, &text[offset]);
	}

	ui_text_line_1(text, BW_UI_MAX_CHARACTERS);
}

/**
 * @ingroup ui
 * @brief UI function #1
 *
 * @param start_channel
 */
inline static void display_data_decimal(const int start_channel) {
	char text[BW_UI_MAX_CHARACTERS];
	int i = 0;
	int offset = 0;
	const int dmx_data_index = start_channel;

	for (i = 0; i < 4; i++){
		offset = i * 4;
		text[offset    ] = ' ';
		text[offset + 1] = ' ';
		text[offset + 3] = ' ';
		itoa_base10(dmx_data[dmx_data_index + i], &text[offset]);
	}

	text[15] = 'D';
	ui_text_line_2(text, BW_UI_MAX_CHARACTERS);
}

/**
 * @ingroup ui
 * @brief UI function #2
 *
 * @param start_channel
 */
inline static void display_data_hex(const int start_channel) {
	char text[BW_UI_MAX_CHARACTERS];

	int i = 0;
	int offset = 0;
	const int dmx_data_index = start_channel;

	for (i = 0; i < 4; i++) {
		offset = i * 4;
		unsigned char data = dmx_data[dmx_data_index + i];
		text[offset    ] = ' ';
		text[offset + 1] = TO_HEX((data & 0xF0) >> 4);
		text[offset + 2] = TO_HEX(data & 0x0F);
		text[offset + 3] = ' ';
	}

	text[15] = 'H';
	ui_text_line_2(text, BW_UI_MAX_CHARACTERS);
}

/**
 * @ingroup ui
 * @brief UI function #3
 *
 * @param start_channel
 */
inline static void display_data_percentage(const int start_channel) {
	char text[BW_UI_MAX_CHARACTERS];
	int i = 0;
	int offset = 0;
	const int dmx_data_index = start_channel;

	for (i = 0; i < 4; i++){
		offset = i * 4;
		text[offset    ] = ' ';
		text[offset + 1] = ' ';
		text[offset + 3] = ' ';
		itoa_base10(((double) dmx_data[dmx_data_index + i] / 255) * 100, &text[offset]);
	}

	text[15] = '%';
	ui_text_line_2(text, BW_UI_MAX_CHARACTERS);
}

/**
 * @ingroup ui
 *
 * @param buttons
 */
void dmx_analyzer(const char buttons) {
	static int dmx_start_address = 1;
	static int ui_function = 1;
	static char ui_dmx_refresh_channels = 1;
	static char ui_dmx_channels_step = 4;

	if (enter) {
		ui_dmx_channels_step = (ui_dmx_channels_step * 2) & 0x1FF;
		enter = 0;
	}

	if ((BUTTON1_PRESSED(buttons)) && (BUTTON2_PRESSED(buttons))) {
		ui_dmx_channels_step = 4;
	}

	if (BUTTON1_PRESSED(buttons)) {								// button(left), scroll dmx channels to right
		dmx_start_address = (dmx_start_address - ui_dmx_channels_step) & 0x1FF;
		ui_dmx_refresh_channels = 1;
	}

	if (BUTTON2_PRESSED(buttons)) {								// button(right), scroll dmx channels to left
		dmx_start_address = (dmx_start_address + ui_dmx_channels_step) & 0x1FF;
		ui_dmx_refresh_channels = 1;
	}

	if (BUTTON3_PRESSED(buttons)) {								// button(up), change data representation (D, H or %)
		ui_function = ui_function + 1;
		if (ui_function > 3)
			ui_function = 1;
	}

	if (BUTTON4_PRESSED(buttons)) {								// button(down), change data representation (D, H or %)
		ui_function = ui_function - 1;
		if (ui_function < 1)
			ui_function = 3;
	}

	if (ui_dmx_refresh_channels || do_ui_cls) {					// button left, right are pressed, or first call from menu
		display_channels(dmx_start_address);
		ui_dmx_refresh_channels = 0;
		do_ui_cls = 0;
	}

	switch (ui_function)
	{
	case 1:
		display_data_decimal(dmx_start_address);
		break;
	case 2:
		display_data_hex(dmx_start_address);
		break;
	case 3:
		display_data_percentage(dmx_start_address);
		break;
	default:
		break;
	}
}
