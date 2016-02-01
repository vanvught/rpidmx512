/**
 * @file ui_function_reboot.c
 *
 */
/* Copyright (C) 2015, 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>

#include "bw_ui.h"
#include "ui_functions.h"
#include "hardware.h"

/**
 * @ingroup ui
 *
 * @param buttons
 */
void reboot(const char buttons) {
	if (do_ui_cls != (char)0) {
		ui_cls();
		ui_text_line_1("Reboot?", 7);
		ui_text_line_2("Yes           No", BW_UI_MAX_CHARACTERS);
		do_ui_cls = (char)0;
	}
	// No need to check for button 6 pressed (activate Menu)
	if (BUTTON1_PRESSED(buttons)) {
	    ui_text_line_2("Rebooting ....  ", BW_UI_MAX_CHARACTERS);
	    hardware_reboot();
	}
}
