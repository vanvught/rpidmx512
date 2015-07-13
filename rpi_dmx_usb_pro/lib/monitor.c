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
#include "console.h"
#include "monitor.h"
#include "sniffer.h"
#include "util.h"

static uint32_t dmx_packets_previous = 0;						///<
static uint32_t widget_received_dmx_packet_count_previous = 0;	///<

/**
 * @ingroup monitor
 */
void monitor_update(void) {
	char dir[3][10] = { "Idle", "Output", "Input" };
	struct _widget_params widget_params;

	widget_params_get(&widget_params);

	monitor_time_uptime(MONITOR_LINE_TIME);

	const uint8_t widget_mode = widget_get_mode();

	if (widget_mode == MODE_RDM_SNIFFER) {
		monitor_dmx_data(MONITOR_LINE_DMX_DATA, dmx_data);

		const struct _total_statistics *total_statistics = total_statistics_get();
		const uint32_t total_packets = total_statistics->dmx_packets + total_statistics->rdm_packets;

		console_clear_line(MONITOR_LINE_PACKETS);
		printf("Packets : %ld, DMX %ld, RDM %ld\n\n", total_packets, total_statistics->dmx_packets, total_statistics->rdm_packets);

		const struct _rdm_statistics *rdm_statistics = rdm_statistics_get();

		printf("Discovery          : %ld\n", rdm_statistics->discovery_packets);
		printf("Discovery response : %ld\n", rdm_statistics->discovery_response_packets);
		printf("GET Requests       : %ld\n", rdm_statistics->get_requests);
		printf("SET Requests       : %ld\n", rdm_statistics->set_requests);

		const uint16_t dmx_updates_per_second = total_statistics->dmx_packets - dmx_packets_previous;

		printf("\nDMX updates/sec %d  \n\n", dmx_updates_per_second);

		if (dmx_updates_per_second != 0) {
			printf("Slots in packet %d      \n",(uint16_t) dmx_get_slots_in_packet());
			printf("Slot to slot    %d      \n", (uint16_t) dmx_get_slot_to_slot());
			printf("Break to break  %ld     \n", dmx_get_break_to_break());
		} else {
			printf("Slots in packet --     \n");
			printf("Slot to slot    --     \n");
			printf("Break to break  --     \n");
		}
		dmx_packets_previous = total_statistics->dmx_packets;
	} else {
		console_clear_line(MONITOR_LINE_WIDGET_PARMS);

		printf("Firmware %d.%d BreakTime %d(%d) MaBTime %d(%d) RefreshRate %d(%d)\n",
				widget_params.firmware_msb, widget_params.firmware_lsb,
				widget_params.break_time, (int) dmx_get_output_break_time(),
				widget_params.mab_time, (int) dmx_get_output_mab_time(),
				widget_params.refresh_rate, (int)(1E6 / dmx_get_output_period()));

		console_clear_line(MONITOR_LINE_PORT_DIRECTION);

		if (DMX_PORT_DIRECTION_INP == dmx_get_port_direction()) {
			const uint8_t receive_dmx_on_change = widget_get_receive_dmx_on_change();
			if (receive_dmx_on_change == SEND_ALWAYS) {
				printf("Input [SEND_ALWAYS]\n");

				const uint32_t widget_received_dmx_packet_count = widget_get_received_dmx_packet_count();
				monitor_line(MONITOR_LINE_STATS, "%d", widget_received_dmx_packet_count - widget_received_dmx_packet_count_previous);
				widget_received_dmx_packet_count_previous = widget_received_dmx_packet_count;
			} else {
				printf("Input [SEND_ON_DATA_CHANGE_ONLY]\n");
			}
		} else {
			printf("%s\n", dir[dmx_get_port_direction()]);
		}

		monitor_dmx_data(MONITOR_LINE_DMX_DATA, dmx_data);
	}
}
