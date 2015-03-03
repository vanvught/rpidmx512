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

#include "dmx.h"
#include "sys_time.h"
#include "console.h"

extern uint8_t dmx_data[512];
extern uint8_t rdm_data[60];

extern uint8_t rdm_is_mute_get(void);

void monitor_update(void)
{
	time_t ltime = 0;
	struct tm *local_time = NULL;

	ltime = sys_time(NULL);
    local_time = localtime(&ltime);

	console_set_cursor(0,2);

	printf("%.2d:%.2d:%.2d\n", local_time->tm_hour, local_time->tm_min, local_time->tm_sec);

	printf("%s\n", dmx_port_direction_get() == DMX_PORT_DIRECTION_INP ? "Input" : "Output");

	printf("01-16 : %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X\n", dmx_data[0],
				dmx_data[1], dmx_data[2], dmx_data[3], dmx_data[4], dmx_data[5],
				dmx_data[6], dmx_data[7], dmx_data[8], dmx_data[9], dmx_data[10],
				dmx_data[11], dmx_data[12], dmx_data[13], dmx_data[14],
				dmx_data[15]);

	printf("17-32 : %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X\n", dmx_data[16],
				dmx_data[17], dmx_data[18], dmx_data[19], dmx_data[20], dmx_data[21],
				dmx_data[22], dmx_data[23], dmx_data[24], dmx_data[25], dmx_data[26],
				dmx_data[27], dmx_data[28], dmx_data[29], dmx_data[30],
				dmx_data[31]);

	printf("[%d]:RDM data[1..36]:\n", rdm_is_mute_get());

	uint8_t i = 0;
	for (i = 0; i < 9; i++)
	{
		printf("%.2d-%.4d:%.2X  %.2d-%.4d:%.2X %.2d-%.4d:%.2X  %.2d-%.4d:%.2X\n",
					i+1, rdm_data[i], rdm_data[i],
					i+10, rdm_data[i+9], rdm_data[i+9],
					i+19, rdm_data[i+18], rdm_data[i+18],
					i+28, rdm_data[i+27], rdm_data[i+27]);
	}
}
