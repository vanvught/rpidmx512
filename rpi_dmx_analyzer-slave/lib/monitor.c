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
#include "console.h"
#include "monitor.h"
#include "dmx_devices.h"


static uint16_t function_count_previous = 0;							///<
static uint16_t dmx_available_count_previous = 0;						///<

/**
 * @ingroup monitor
 */
void monitor_update(void)
{
	monitor_time_uptime(3);

	monitor_dmx_data(5, dmx_data);

	console_clear_line(8);
	const struct _total_statistics *total_statistics = total_statistics_get();
	const uint32_t total_packets = total_statistics->dmx_packets + total_statistics->rdm_packets;
	printf("Packets : %ld, DMX %ld, RDM %ld\n\n", total_packets, total_statistics->dmx_packets, total_statistics->rdm_packets);

	console_clear_line(14);
	printf("Slots in packet %d\n", (uint16_t)dmx_slots_in_packet_get());
	printf("Slot to slot    %d\n", (uint16_t)dmx_slot_to_slot_get());

	const struct _dmx_devices_statistics *dmx_devices_statistics = dmx_devices_get_statistics();
	const uint16_t function_count_per_second = dmx_devices_statistics->function_count - function_count_previous;
	const uint16_t dmx_available_count_per_second = dmx_devices_statistics->dmx_available_count - dmx_available_count_previous;

	monitor_line(25, "%d / %d", function_count_per_second, dmx_available_count_per_second);

	function_count_previous = dmx_devices_statistics->function_count;
	dmx_available_count_previous = dmx_devices_statistics->dmx_available_count;
}
