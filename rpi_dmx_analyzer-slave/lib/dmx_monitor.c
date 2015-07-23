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

#include <dmx_devices.h>
#include <stdio.h>

#include "dmx.h"
#include "console.h"
#include "monitor.h"

static uint32_t function_count_previous = 0;		///<
static uint32_t dmx_available_count_previous = 0;	///<

/**
 * @ingroup monitor
 */
void monitor_update(void)
{
	monitor_time_uptime(MONITOR_LINE_TIME);
	monitor_sniffer();

	const struct _dmx_devices_statistics *dmx_devices_statistics = dmx_devices_get_statistics();
	const uint32_t function_count_per_second = dmx_devices_statistics->function_count - function_count_previous;
	const uint32_t dmx_available_count_per_second = dmx_devices_statistics->dmx_available_count - dmx_available_count_previous;

	monitor_line(MONITOR_LINE_STATS, "%ld / %ld", function_count_per_second, dmx_available_count_per_second);

	function_count_previous = dmx_devices_statistics->function_count;
	dmx_available_count_previous = dmx_devices_statistics->dmx_available_count;
}
