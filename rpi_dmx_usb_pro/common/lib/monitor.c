/**
 * @file monitor_debug.c
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

#include "console.h"

/**
 * @ingroup monitor
 *
 * @param line
 * @param fmt
 */
void monitor_line(const uint8_t line, const char *fmt, ...)
{
	va_list va;

	console_clear_line(line);

	if (fmt != NULL)
	{
		va_start(va, fmt);
		vprintf(fmt, va);
		va_end(va);
		fflush(stdout);
	}
}

/**
 * @ingroup monitor
 *
 * @param line
 * @param data_length
 * @param data
 */
void monitor_rdm_data(const uint8_t line, const uint16_t data_length, const uint8_t *data)
{
	uint8_t i;
	console_clear_line(line);

	printf("RDM Packet length : %d\n", data_length);

	for (i = 0; i < 9; i++)
	{
		printf("%.2d-%.4d:%.2X  %.2d-%.4d:%.2X %.2d-%.4d:%.2X  %.2d-%.4d:%.2X\n",
				i+1, data[i], data[i], i+10, data[i+9], data[i+9], i+19, data[i+18], data[i+18], i+28, data[i+27], data[i+27]);
	}
}

void monitor_dmx_data(const uint8_t line, const uint8_t *data)
{
	console_set_cursor(0,line);

	printf("01-16 : %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X\n",
			data[1], data[2], data[3], data[4], data[5],
			data[6], data[7], data[8], data[9], data[10],
			data[11], data[12], data[13], data[14], data[15], data[16]);
	printf("17-32 : %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X\n",
			data[17], data[18], data[19], data[20], data[21],
			data[22], data[23], data[24], data[25], data[26],
			data[27], data[28], data[29], data[30], data[31], data[32]);
}
