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
#include "hardware.h"
#include "monitor.h"
#include "util.h"

/**
 * @ingroup monitor
 */
void monitor_update(void)
{
	const uint32_t minute = 60;
	const uint32_t hour = minute * 60;
	const uint32_t day = hour * 24;

	time_t ltime = 0;
	struct tm *local_time = NULL;

	ltime = sys_time(NULL);
    local_time = localtime(&ltime);

	uint64_t uptime_seconds = hardware_uptime_seconds();

	console_set_cursor(0,1);
	printf("%.2d:%.2d:%.2d uptime : %ld days, %ld:%02ld:%02ld\n\n",
			local_time->tm_hour, local_time->tm_min, local_time->tm_sec,
			(long int) (uptime_seconds / day),
			(long int) (uptime_seconds % day) / hour,
			(long int) (uptime_seconds % hour) / minute,
			(long int) uptime_seconds % minute);

	console_clear_line(3);
	printf("Slots in packet %d\n", (uint16_t)dmx_slots_in_packet_get());
	printf("Slot to slot    %d\n", (uint16_t)dmx_slot_to_slot_get());

}
