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

#include <stdio.h>

#include "bw_i2c_ui.h"
#include "ui_functions.h"
#include "dmx.h"

#ifdef DEBUG
#include <bcm2835.h>
#define DEBUG_START(x)		uint64_t st_now = bcm2835_st_read();
#define DEBUG_END(x)		printf("%s -> %d\n", (x), (int)(bcm2835_st_read() - st_now));
#else
#define DEBUG_START(x)
#define DEBUG_END(x)
#endif

char button;
char enter = (char)0;
int ui_function_menu = 1;
int ui_function_active = 1;

char do_ui_cls = (char)1;

extern void dmx_slave(const char);
extern void dmx_analyzer(const char);
extern void clock_time(const char);
extern void reboot(const char);

struct _ui_function {
	const char *msg;
	void (*f)(char);
} const ui_functions[] = {
		{ "DMX512 Slave   ", dmx_slave},
		{ "DMX512 Analyzer", dmx_analyzer},
		{ "Clock          ", clock_time},
		{ "Reboot         ", reboot}
};

void ui_lcd_refresh(void) {
	DEBUG_START("ui_lcd_refresh");

	if (ui_function_menu == -1) {
		if (BUTTON3_PRESSED(button)) {
			ui_function_active += 1;
			if (ui_function_active > (int)(sizeof(ui_functions) / sizeof(ui_functions[0])) - 1)
				ui_function_active = 0;
		}

		if (BUTTON4_PRESSED(button)) {
			ui_function_active -= 1;
			if (ui_function_active < 0)
				ui_function_active = (int)(sizeof(ui_functions) / sizeof(ui_functions[0])) - 1;
		}

		if(enter) {
			ui_function_menu = ui_function_active;
			enter = (char)0;
			do_ui_cls = (char)1;
		} else if (button) {
			const char *text = ui_functions[ui_function_active].msg;
			if (do_ui_cls) {
				bw_i2c_ui_cls();
				bw_i2c_ui_text_line_1("Function :", 10);
				do_ui_cls = (char)0;
			}
			bw_i2c_ui_text_line_2(text, 15);
		}
		button = (char)0;
	}

	if (ui_function_menu >= 0) {
		ui_functions[ui_function_active].f(button);
		button = (char)0;
	}

	DEBUG_END("ui_lcd_refresh");
}

void ui_buttons_update(void) {
	DEBUG_START("ui_buttons_update");

	char buttons = bw_i2c_ui_read_button_last();

	if (buttons) {
		enter = BUTTON5_PRESSED(buttons);
		if (BUTTON6_PRESSED(buttons)) {
			ui_function_menu = -1;
			do_ui_cls = 1;
		}
		button = button | buttons;
	}

	DEBUG_END("ui_buttons_update");
}
