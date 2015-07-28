/**
 * @file ui_function_clock.c
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

#include <stdio.h>

#include "sys_time.h"
#include "bw_ui.h"
#include "ui_functions.h"

/**
 *
 * @param arg
 * @param buf
 */
inline static void itoa_base10(int arg, char buf[]) {
	char *n = buf + 1;

	buf[0] = '0';
	buf[1] = '0';

	while (arg != 0) {
		*n = (char)'0' + (char)(arg % 10);
		n--;
		arg /= 10;
	}
}

/**
 * @ingroup ui
 *
 * @param buttons
 */
void clock_time(/*@unused@*/const char buttons) {
	time_t ltime = 0;
	struct tm *local_time = NULL;
	static char buf[BW_UI_MAX_CHARACTERS];

	if (do_ui_cls) {
		ui_cls();
		ui_text_line_1("Clock", 5);
		do_ui_cls = (char)0;
		buf[2] = ':';
		buf[5] = ':';
    }

	ltime = sys_time(NULL);
    local_time = localtime(&ltime);

    itoa_base10(local_time->tm_hour, &buf[0]);
    itoa_base10(local_time->tm_min , &buf[3]);
    itoa_base10(local_time->tm_sec , &buf[6]);
    ui_text_line_2(buf, 8);
}
