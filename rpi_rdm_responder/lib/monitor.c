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

#include "monitor.h"
#include "console.h"
#include "util.h"
#include "dmx.h"
#include "rdm.h"
#include "rdm_device_info.h"
#include "rdm_handle_data.h"
#include "dmx_handle_data.h"

static uint16_t function_count_previous = 0;			///<
static uint16_t dmx_available_count_previous = 0;		///<

void monitor_update(void)
{
	monitor_time_uptime(4);

	monitor_line(MONITOR_LINE_INFO, "%s", dmx_get_port_direction() == DMX_PORT_DIRECTION_INP ? "Input" : "Output");

	const uint16_t dmx_start_address = rdm_device_info_get_dmx_start_address(0);

	console_set_cursor(0,8);
	printf("%.3d-%.3d : ", dmx_start_address, (dmx_start_address + 15) & 0x1FF);

	uint8_t i = 0;
	for (i = 0; i< 16; i++)
	{
		uint16_t index = (dmx_start_address + i <= DMX_UNIVERSE_SIZE) ? (dmx_start_address + i) : (dmx_start_address + i - DMX_UNIVERSE_SIZE);
		uint8_t data = dmx_data[index];
		putchar(TO_HEX((data & 0xF0) >> 4));
		putchar(TO_HEX(data & 0x0F));
		putchar(' ');
	}

	printf("\n%.3d-%.3d : ", (dmx_start_address + 16) & 0x1FF, (dmx_start_address + 31) & 0x1FF);

	for (i = 16; i< 32; i++)
	{
		uint16_t index = (dmx_start_address + i <= DMX_UNIVERSE_SIZE) ? (dmx_start_address + i) : (dmx_start_address + i - DMX_UNIVERSE_SIZE);
		uint8_t data = dmx_data[index];
		putchar(TO_HEX((data & 0xF0) >> 4));
		putchar(TO_HEX(data & 0x0F));
		putchar(' ');
	}

	const struct _total_statistics *total_statistics = total_statistics_get();

	printf("\n\nPackets : DMX %ld, RDM %ld\n", total_statistics->dmx_packets, total_statistics->rdm_packets);
	printf("[%s] \n\n", rdm_is_muted() == 1 ? "Muted" :  "Unmute");

	const uint8_t *rdm_data = rdm_get_current_data();

	for (i = 0; i < 9; i++)
	{
		printf("%.2d-%.4d:%.2X %.2d-%.4d:%.2X %.2d-%.4d:%.2X %.2d-%.4d:%.2X\n",
					i+1,  rdm_data[i],    rdm_data[i],
					i+10, rdm_data[i+9],  rdm_data[i+9],
					i+19, rdm_data[i+18], rdm_data[i+18],
					i+28, rdm_data[i+27], rdm_data[i+27]);
	}

	const struct _dmx_handle_data_statistics *dmx_handle_data_statistics = dmx_handle_data_get_statistics();
	const uint16_t function_count_per_second = dmx_handle_data_statistics->function_count - function_count_previous;
	const uint16_t dmx_available_count_per_second = dmx_handle_data_statistics->dmx_available_count - dmx_available_count_previous;

	monitor_line(MONITOR_LINE_STATS, "%d / %d", function_count_per_second, dmx_available_count_per_second);

	function_count_previous = dmx_handle_data_statistics->function_count;
	dmx_available_count_previous = dmx_handle_data_statistics->dmx_available_count;
}
