/**
 * @file rdm_monitor.c
 *
 */
/* Copyright (C) 2015, 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "console.h"
#include "monitor.h"
#include "util.h"
#include "dmx.h"
#include "rdm.h"
#include "rdm_device_info.h"
#include "rdm_handle_data.h"
#include "dmx_devices.h"

static uint32_t function_count_previous = 0;			///<
static uint32_t run_count_previous = 0;		///<

/**
 * @ingroup rdm
 *
 */
void monitor_update(void) {
	uint8_t i;

	const uint8_t display_level = rdm_device_info_get_ext_mon_level();

	if (display_level == 0) {
		return;
	}

	if (display_level > 1) {
		monitor_time_uptime(MONITOR_LINE_TIME);
		monitor_line(MONITOR_LINE_PORT_DIRECTION, "%s", dmx_get_port_direction() == DMX_PORT_DIRECTION_INP ? "Input" : "Output");

		const uint16_t dmx_start_address = rdm_device_info_get_dmx_start_address(0);

		monitor_line(MONITOR_LINE_DMX_DATA, "%.3d-%.3d : ", dmx_start_address, (dmx_start_address + 15) & 0x1FF);

		const uint8_t *dmx_data = dmx_get_current_data();
		const struct _dmx_data *dmx_statistics = (struct _dmx_data *)dmx_data;
		const uint16_t slots_in_packet = (uint16_t) (dmx_statistics->statistics.slots_in_packet);

		for (i = 0; i < 16; i++) {
			uint16_t index = (dmx_start_address + i <= (uint16_t) DMX_UNIVERSE_SIZE) ?
							(dmx_start_address + i) : (dmx_start_address + i - (uint16_t) DMX_UNIVERSE_SIZE);
			if (i < slots_in_packet) {
				uint8_t data = dmx_data[index];
				if (data == 0) {
					console_puts(" 0");
				} else {
					console_puthex_fg_bg(data, (uint16_t)(data > 92 ? CONSOLE_BLACK : CONSOLE_WHITE), (uint16_t)RGB(data,data,data));
				}
			}
			else {
				console_puts("--");
			}
			(void) console_putc((int) ' ');
		}

		printf("\n%.3d-%.3d : ", (int)((dmx_start_address + 16) & 0x1FF), (int)((dmx_start_address + 31) & 0x1FF));

		for (i = 16; i < 32; i++) {
			uint16_t index = (dmx_start_address + i <= (uint16_t) DMX_UNIVERSE_SIZE) ?
							(dmx_start_address + i) : (dmx_start_address + i - (uint16_t) DMX_UNIVERSE_SIZE);
			if (i < slots_in_packet) {
				uint8_t data = dmx_data[index];
				if (data == 0) {
					console_puts(" 0");
				} else {
					console_puthex_fg_bg(data, (uint16_t)(data > 92 ? CONSOLE_BLACK : CONSOLE_WHITE), (uint16_t)RGB(data,data,data));
				}
			}
			else {
				console_puts("--");
			}
			(void) console_putc((int) ' ');
		}

		const volatile struct _total_statistics *total_statistics = dmx_get_total_statistics();

		monitor_line(MONITOR_LINE_PACKETS, "Packets : DMX %ld, RDM %ld\n", total_statistics->dmx_packets, total_statistics->rdm_packets);

		if (rdm_is_muted()) {
			console_puts("[Muted]");
		} else {
			console_puts("[Unmute]");
		}

		console_set_cursor(0, MONITOR_LINE_RDM_DATA);

		const uint8_t *rdm_data = rdm_get_current_data();

		for (i = 0; i < 9; i++) {
			printf("%.2d-%.4d:%.2X %.2d-%.4d:%.2X %.2d-%.4d:%.2X %.2d-%.4d:%.2X\n",
					(int) i + 1, (int) rdm_data[i], (unsigned int) rdm_data[i], (int) i + 10, (int) rdm_data[i + 9],
					(unsigned int) rdm_data[i + 9], (int) i + 19, (int) rdm_data[i + 18], (unsigned int) rdm_data[i + 18],
					(int) i + 28, (int) rdm_data[i + 27], (unsigned int) rdm_data[i + 27]);
		}
	}

	const volatile struct _dmx_devices_statistics *dmx_handle_data_statistics = dmx_devices_get_statistics();
	const uint32_t run_count_per_second = dmx_handle_data_statistics->run_count - run_count_previous;
	const uint32_t function_count_per_second = dmx_handle_data_statistics->function_count - function_count_previous;
	const uint32_t dmx_updates_per_seconde = dmx_get_updates_per_seconde();

	monitor_line(MONITOR_LINE_STATS, "%ld / %ld[%d]", function_count_per_second, run_count_per_second, dmx_updates_per_seconde);

	function_count_previous = dmx_handle_data_statistics->function_count;
	run_count_previous = dmx_handle_data_statistics->run_count;
}
