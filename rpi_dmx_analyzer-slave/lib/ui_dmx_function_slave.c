/**
 * @file ui_function_slave.c
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

#include <dmx_devices.h>
#include <string.h>

#include "bw_ui.h"
#include "ui_functions.h"
#include "tables.h"

extern initializer_t devices_table[];	///<
extern _devices_t devices_connected;	///<

/**
 *
 * @param arg
 * @param buf
 */
inline static void itoa_base10(int arg, char buf[]) {
	char *n = buf + 1;

	buf[0] = '0';
	buf[1] = '0';

	while (arg) {
		*n = '0' + (arg % 10);
		n--;
		arg /= 10;
	}
}

/**
 *
 * @param arg
 * @param buf
 */
inline static void itoa_base10_3(int arg, char buf[]) {
	char *n = buf + 2;

	buf[0] = ' ';
	buf[1] = ' ';
	buf[2] = '0';

	while (arg) {
		*n = '0' + (arg % 10);
		n--;
		arg /= 10;
	}
}

/**
 * @ingroup ui
 *
 * @param buttons
 */
void dmx_slave(const char buttons) {
	static char ui_dmx_slave_refresh_devices = 1;
	static char ui_dmx_slave_function = 0;
	static int devices_connected_index = 0;

	int devices_connected_index_max = devices_connected.elements_count - 1;

	if (enter) {
		ui_dmx_slave_function = ui_dmx_slave_function^1;
		enter = 0;
		do_ui_cls = 1;
		ui_dmx_slave_refresh_devices = 1;
	}

	if (BUTTON3_PRESSED(buttons)) {
		devices_connected_index = devices_connected_index + 1;
		if (devices_connected_index > devices_connected_index_max)
			devices_connected_index = 0;
		ui_dmx_slave_refresh_devices = 1;
	}

	if (BUTTON4_PRESSED(buttons)) {
		devices_connected_index = devices_connected_index - 1;
		if (devices_connected_index < 0)
			devices_connected_index = devices_connected_index_max;
		ui_dmx_slave_refresh_devices = 1;
	}

	if (ui_dmx_slave_function) {
		if (devices_connected.elements_count > 0) {
			if (ui_dmx_slave_refresh_devices) {
				// Line 1
				static char text_1[BW_UI_MAX_CHARACTERS + 1] = {'S', 'P', 'I', ' ', ' '};
				text_1[3] = devices_connected.device_entry[devices_connected_index].dmx_device_info.device_info.chip_select + '0';
				strncpy(&text_1[5],devices_table[devices_connected.device_entry[devices_connected_index].devices_table_index].name, BW_UI_MAX_CHARACTERS - 5);
				int length = strlen(text_1);
				memset(&text_1[length], ' ', BW_UI_MAX_CHARACTERS - length);
				ui_text_line_1(text_1, BW_UI_MAX_CHARACTERS);
				// Line 2
				static char text_2[BW_UI_MAX_CHARACTERS] = {'s', 'a', ':', 'x', ' ', ' ', ' ', 'd', 'm', 'x', ':', ' ', ' ', ' ', 'D'};
				char slave_address = devices_connected.device_entry[devices_connected_index].dmx_device_info.device_info.slave_address;
				int  dmx_start_address = devices_connected.device_entry[devices_connected_index].dmx_device_info.rdm_sub_devices_info->dmx_start_address;
				text_2[4] = TO_HEX((slave_address & 0xF0) >> 4);
				text_2[5] = TO_HEX(slave_address & 0x0F);
				itoa_base10_3(dmx_start_address, &text_2[11]);
				ui_text_line_2(text_2, 15);
				// Disable refresh
				ui_dmx_slave_refresh_devices = 0;
			}
		}
	} else {
		if (do_ui_cls) {
			static char text[BW_UI_MAX_CHARACTERS] = {'D', 'e', 'v', 'i', 'c', 'e', 's', ' ', ':', ' ', ' '};
			ui_cls();
			ui_text_line_1("DMX Slave", 9);
			itoa_base10(devices_connected.elements_count, &text[10]);
			ui_text_line_2(text, 12);
			do_ui_cls = 0;
		}
	}
}
