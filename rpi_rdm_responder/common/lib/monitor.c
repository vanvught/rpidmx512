/**
 * @file monitor.c
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
#include <stdint.h>
#include <stdarg.h>

#include "sys_time.h"
#include "hardware.h"
#include "console.h"

/**
 * @ingroup monitor
 *
 * @param line
 * @param fmt
 */
void monitor_line(const int line, const char *fmt, ...)
{
	va_list va;

	console_clear_line(line);

	if (fmt != NULL) {
		va_start(va, fmt);
		(void) vprintf(fmt, va);
		va_end(va);
		(void) fflush(stdout);
	}
}

/**
 * @ingroup monitor
 *
 * @param line
 */
void monitor_time_uptime(const int line) {
	const uint32_t minute = 60;
	const uint32_t hour = minute * 60;
	const uint32_t day = hour * 24;

	const uint64_t uptime_seconds = hardware_uptime_seconds();

	time_t ltime = 0;
	struct tm *local_time = NULL;

	ltime = sys_time(NULL);
	local_time = localtime(&ltime);

	console_set_cursor(0, line);

	printf("%.2d:%.2d:%.2d uptime : %ld days, %ld:%02ld:%02ld\n",
			local_time->tm_hour, local_time->tm_min, local_time->tm_sec,
			(long int) (uptime_seconds / day),
			(long int) (uptime_seconds % day) / hour,
			(long int) (uptime_seconds % hour) / minute,
			(long int) uptime_seconds % minute);
}

/**
 * @ingroup monitor
 *
 * @param line
 * @param data_length
 * @param data
 */
void monitor_rdm_data(const int line, const uint16_t data_length, const uint8_t *data) {
	uint8_t i;
	console_clear_line(line);

	printf("RDM Packet length : %d\n", (int) data_length);

	for (i = 0; i < 9; i++) {
		printf("%.2d-%.4d:%.2X  %.2d-%.4d:%.2X %.2d-%.4d:%.2X  %.2d-%.4d:%.2X\n",
				(int) i + 1, (int) data[i], (unsigned int) data[i],
				(int) i + 10, (int) data[i + 9], (unsigned int) data[i + 9],
				(int) i + 19, (int) data[i + 18], (unsigned int) data[i + 18],
				(int) i + 28, (int) data[i + 27], (unsigned int) data[i + 27]);
	}
}

/**
 * @ingroup monitor
 *
 * @param line
 * @param data
 */
void monitor_dmx_data(const int line, const uint8_t *data) {
	console_set_cursor(0, line);

	printf("01-16 : %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X\n",
			(unsigned int) data[1], (unsigned int) data[2],
			(unsigned int) data[3], (unsigned int) data[4],
			(unsigned int) data[5], (unsigned int) data[6],
			(unsigned int) data[7], (unsigned int) data[8],
			(unsigned int) data[9], (unsigned int) data[10],
			(unsigned int) data[11], (unsigned int) data[12],
			(unsigned int) data[13], (unsigned int) data[14],
			(unsigned int) data[15], (unsigned int) data[16]);
	printf("17-32 : %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X\n",
			(unsigned int) data[17], (unsigned int) data[18],
			(unsigned int) data[19], (unsigned int) data[20],
			(unsigned int) data[21], (unsigned int) data[22],
			(unsigned int) data[23], (unsigned int) data[24],
			(unsigned int) data[25], (unsigned int) data[26],
			(unsigned int) data[27], (unsigned int) data[28],
			(unsigned int) data[29], (unsigned int) data[30],
			(unsigned int) data[31], (unsigned int) data[32]);
}
