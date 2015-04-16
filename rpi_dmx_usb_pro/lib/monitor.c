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
#include "widget.h"
#include "widget_params.h"
#include "sys_time.h"
#include "console.h"
#include "hardware.h"

/**
 * @ingroup monitor
 */
void monitor_update(void)
{
	char dir[3][10] =
	{ "Idle", "Output", "Input" };
	struct _widget_params widget_params;

	time_t ltime = 0;
	struct tm *local_time = NULL;

	ltime = sys_time(NULL);
    local_time = localtime(&ltime);

	widget_params_get(&widget_params);

	uint64_t uptime_seconds = hardware_uptime_seconds();

	// line 1
	console_set_cursor(0,1);

	printf("%.2d:%.2d:%.2d uptime : %ld days, %ld:%02ld:%02ld\n",
			local_time->tm_hour, local_time->tm_min, local_time->tm_sec,
			(long int) (uptime_seconds / DAY),
			(long int) (uptime_seconds % DAY) / HOUR,
			(long int) (uptime_seconds % HOUR) / MINUTE,
			(long int) uptime_seconds % MINUTE);

	// line 2
	printf("FW %d.%d BreakTime %d(%d) MaBTime %d(%d) RefreshRate %d\n",
			widget_params.firmware_msb, widget_params.firmware_lsb,
			widget_params.break_time, (int) dmx_output_break_time_get(),
			widget_params.mab_time, (int) dmx_output_mab_time_get(),
			widget_params.refresh_rate);

	// line 3
	console_clear_line(3);

	if (DMX_PORT_DIRECTION_INP == dmx_port_direction_get())
		printf("Input [%s]\n", receive_dmx_on_change_get() == SEND_ALWAYS ? "SEND_ALWAYS" : "SEND_ON_DATA_CHANGE_ONLY");
	else
	{
		printf("%s\n", dir[dmx_port_direction_get()]);
	}

	// line 4 LABEL
	// line 5 Info
	// line 6

	const uint8_t widget_mode = widget_mode_get();

	if (widget_mode == MODE_RDM_SNIFFER)
	{

	} else
	{
	// line 8
	console_set_cursor(0,8);

	printf("01-16 : %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X\n",
			dmx_data[0], dmx_data[1], dmx_data[2], dmx_data[3], dmx_data[4],
			dmx_data[5], dmx_data[6], dmx_data[7], dmx_data[8], dmx_data[9],
			dmx_data[10], dmx_data[11], dmx_data[12], dmx_data[13],
			dmx_data[14], dmx_data[15]);
	printf("17-32 : %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X\n",
			dmx_data[16], dmx_data[17], dmx_data[18], dmx_data[19],
			dmx_data[20], dmx_data[21], dmx_data[22], dmx_data[23],
			dmx_data[24], dmx_data[25], dmx_data[26], dmx_data[27],
			dmx_data[28], dmx_data[29], dmx_data[30], dmx_data[31]);

	// line 11 RDM Data
	}
}
