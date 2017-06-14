/**
 * @file monitor_time_uptime.c
 *
 */
/* Copyright (C) 2016, 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stddef.h>
#include <time.h>

#include "console.h"
#include "hardware.h"
#include "sys_time.h"

void monitor_time_uptime(const int line) {
	uint32_t time, days, hours, minutes, seconds;
	time_t ltime;
	struct tm *local_time;

	time = hardware_uptime_seconds();

	ltime = sys_time(NULL);
	local_time = localtime(&ltime);

	console_save_cursor();
	console_set_cursor(0, line);

	days = time / ((uint32_t) 24 * (uint32_t) 3600);
	time -= days * ((uint32_t) 24 * (uint32_t) 3600);
	hours = time / (uint32_t) 3600;
	time -= hours * (uint32_t) 3600;
	minutes = time / (uint32_t) 60;
	seconds = time - minutes * (uint32_t) 60;

	printf("Local time %.2d:%.2d:%.2d, uptime %d days, %02d:%02d:%02d",
			local_time->tm_hour, local_time->tm_min, local_time->tm_sec,
			(int) days, (int) hours, (int) minutes, (int) seconds);

	console_restore_cursor();
}
