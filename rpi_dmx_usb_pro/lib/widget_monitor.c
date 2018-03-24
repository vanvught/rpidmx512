/**
 * @file widget_monitor.c
 *
 */
/* Copyright (C) 2015-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
#include "console.h"
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>
#include <stdio.h>

#include "console.h"

#include "monitor.h"

#include "sniffer.h"
#include "widget.h"
#include "widget_params.h"

#include "dmx.h"
#include "rdm.h"

#include "rdm_device_info.h"

#include "util.h"

/* RDM START CODE (Slot 0)                                                                                                     */
#define	E120_SC_RDM		0xCC

static uint32_t widget_received_dmx_packet_count_previous = 0;

static uint32_t updates_per_seconde_min = UINT32_MAX;
static uint32_t updates_per_seconde_max = (uint32_t)0;
static uint32_t slots_in_packet_min = UINT32_MAX;
static uint32_t slots_in_packet_max = (uint32_t)0;
static uint32_t slot_to_slot_min = UINT32_MAX;
static uint32_t slot_to_slot_max = (uint32_t)0;
static uint32_t break_to_break_min = UINT32_MAX;
static uint32_t break_to_break_max = (uint32_t)0;

static void monitor_dmx_data(const uint8_t * dmx_data, const int line) {
	uint16_t i;
	uint16_t slots;

	if (DMX_PORT_DIRECTION_INP == dmx_get_port_direction()) {
		const struct _dmx_data *dmx_statistics = (struct _dmx_data *)dmx_data;
		slots = (uint16_t) (dmx_statistics->statistics.slots_in_packet + 1);
	} else {
		slots = dmx_get_send_data_length();
	}

	console_set_cursor(0, line);

	console_puts("01-16 : ");

	if (slots > 33) {
		slots = 33;
	}

	for (i = (uint16_t)1; i < slots; i++) {
		uint8_t d = dmx_data[i];
		if (d == (uint8_t)0) {
			console_puts(" 0");
		} else {
			console_puthex_fg_bg(d, (uint16_t)(d > 92 ? CONSOLE_BLACK : CONSOLE_WHITE), (uint16_t)RGB(d,d,d));
		}
		if (i == (uint16_t)16) {
			console_puts("\n17-32 : ");
		} else {
			(void) console_putc((int) ' ');
		}

	}

	for (; i < (uint16_t)33; i++) {
		console_puts("--");
		if (i == (uint16_t)16) {
			console_puts("\n17-32 : ");
		} else {
			(void) console_putc((int) ' ');
		}
	}
}

/**
 * @ingroup monitor
 *
 */
static void monitor_sniffer(void) {
	const volatile struct _total_statistics *total_statistics = dmx_get_total_statistics();
	const uint32_t total_packets = total_statistics->dmx_packets + total_statistics->rdm_packets;
	const uint8_t *dmx_data = dmx_get_current_data();
	const struct _dmx_data *dmx_statistics = (struct _dmx_data *)dmx_data;
	const uint32_t dmx_updates_per_seconde = dmx_get_updates_per_seconde();
	const volatile struct _rdm_statistics *rdm_statistics = rdm_statistics_get();

	monitor_dmx_data(dmx_data, MONITOR_LINE_DMX_DATA);

	monitor_line(MONITOR_LINE_PACKETS, "Packets : %ld, DMX %ld, RDM %ld\n\n", (long int) total_packets, (long int) total_statistics->dmx_packets, (long int) total_statistics->rdm_packets);

	printf("Discovery          : %ld\n", rdm_statistics->discovery_packets);
	printf("Discovery response : %ld\n", rdm_statistics->discovery_response_packets);
	printf("GET Requests       : %ld\n", rdm_statistics->get_requests);
	printf("SET Requests       : %ld\n", rdm_statistics->set_requests);

	if ((int)dmx_updates_per_seconde != (int)0) {
		updates_per_seconde_min = MIN(dmx_updates_per_seconde, updates_per_seconde_min);
		updates_per_seconde_max = MAX(dmx_updates_per_seconde, updates_per_seconde_max);
		printf("\nDMX updates/sec     %3d     %3d / %d\n\n", (int)dmx_updates_per_seconde, (int)updates_per_seconde_min , (int)updates_per_seconde_max);
	} else {
		console_puts("\nDMX updates/sec --     \n\n");
	}

	if (dmx_updates_per_seconde != 0) {
		slots_in_packet_min = MIN(dmx_statistics->statistics.slots_in_packet, slots_in_packet_min);
		slots_in_packet_max = MAX(dmx_statistics->statistics.slots_in_packet, slots_in_packet_max);
		slot_to_slot_min = MIN(dmx_statistics->statistics.slot_to_slot, slot_to_slot_min);
		slot_to_slot_max = MAX(dmx_statistics->statistics.slot_to_slot, slot_to_slot_max);
		break_to_break_min = MIN(dmx_statistics->statistics.break_to_break, break_to_break_min);
		break_to_break_max = MAX(dmx_statistics->statistics.break_to_break, break_to_break_max);

		printf("Slots in packet     %3d     %3d / %d\n", (int)dmx_statistics->statistics.slots_in_packet, (int)slots_in_packet_min, (int)slots_in_packet_max);
		printf("Slot to slot        %3d     %3d / %d\n", (int)dmx_statistics->statistics.slot_to_slot, (int)slot_to_slot_min, (int)slot_to_slot_max);
		printf("Break to break   %6d  %6d / %d\n", (int)dmx_statistics->statistics.break_to_break, (int)break_to_break_min, (int)break_to_break_max);
	} else {
		console_puts("Slots in packet --     \n");
		console_puts("Slot to slot    --     \n");
		console_puts("Break to break  --     \n");
	}
}

/**
 * @ingroup monitor
 */
void monitor_update(void) {
	const uint8_t widget_mode = widget_get_mode();
	struct _widget_params widget_params;
	const uint8_t display_level = rdm_device_info_get_ext_mon_level();

	if (widget_mode != MODE_RDM_SNIFFER && display_level == 0) {
		return;
	}

	widget_params_get(&widget_params);

	if (display_level > 1) {
		monitor_time_uptime(MONITOR_LINE_TIME);
	}

	console_clear_line(MONITOR_LINE_INFO);

	if (widget_mode == MODE_RDM_SNIFFER) {
		monitor_sniffer();
	} else {
		console_clear_line(MONITOR_LINE_WIDGET_PARMS);

		printf("Firmware %d.%d BreakTime %d(%d) MaBTime %d(%d) RefreshRate %d(%d)",
				(int)widget_params.firmware_msb, (int)widget_params.firmware_lsb,
				(int)widget_params.break_time, (int) dmx_get_output_break_time(),
				(int)widget_params.mab_time, (int) dmx_get_output_mab_time(),
				(int)widget_params.refresh_rate, (int) (1E6 / dmx_get_output_period()));

		console_clear_line(MONITOR_LINE_PORT_DIRECTION);

		if (DMX_PORT_DIRECTION_INP == dmx_get_port_direction()) {
			const uint8_t receive_dmx_on_change = widget_get_receive_dmx_on_change();

			if (receive_dmx_on_change == SEND_ALWAYS) {
				const uint32_t throttle = widget_get_received_dmx_packet_period();
				const uint32_t widget_received_dmx_packet_count = widget_get_received_dmx_packet_count();

				console_puts("Input [SEND_ALWAYS]");

				if (throttle != (uint32_t) 0) {
					printf(", Throttle %d", (int) (1E6 / throttle));
				}

				monitor_line(MONITOR_LINE_STATS, "DMX packets per second to host : %d", widget_received_dmx_packet_count - widget_received_dmx_packet_count_previous);
				widget_received_dmx_packet_count_previous = widget_received_dmx_packet_count;
			} else {
				console_puts("Input [SEND_ON_DATA_CHANGE_ONLY]");
				console_clear_line(MONITOR_LINE_STATS);
			}
		} else {
			console_puts("Output");
			console_clear_line(MONITOR_LINE_STATS);
		}

		const uint8_t *dmx_data = dmx_get_current_data();
		monitor_dmx_data(dmx_data, MONITOR_LINE_DMX_DATA);
	}
}

/**
 * @ingroup monitor
 *
 * @param line
 * @param data_length
 * @param data
 * @param is_sent
 */
void monitor_rdm_data(const int line, const uint16_t data_length, const uint8_t *data, bool is_sent) {
	uint16_t l;
	uint8_t *p = (uint8_t *) data;
	bool is_rdm_command = (*p == E120_SC_RDM);
	const uint8_t display_level = rdm_device_info_get_ext_mon_level();

	monitor_line(line, "RDM [%s], l:%d\n", is_sent ? "Sent" : "Received", (int) data_length);

	if (display_level == 0) {
		return;
	}

	for (l = 0; l < MIN(data_length, 28); l++) {
		console_puthex(*p++);
		(void) console_putc((int) ' ');
	}

	while (l++ < (uint16_t)26) {
		console_puts("   ");
	}

	if (is_rdm_command) {
		if (data_length >= 24) {
			struct _rdm_command *cmd = (struct _rdm_command *) (data);
			monitor_line(line + 2, "tn:%d, cc:%.2x, pid:%d, l:%d", (int)cmd->transaction_number,
					(unsigned int)cmd->command_class, (int)((cmd->param_id[0] << 8) + cmd->param_id[1]), (int) cmd->param_data_length);
			console_clear_line(line + 4);
			console_clear_line(line + 3);
			if (cmd->param_data_length != 0) {
				uint8_t i = MIN(cmd->param_data_length, 24);
				uint8_t j;

				p = &cmd->param_data[0];

				for (j = 0 ; j < i; j++) {
					console_puthex(*p++);
					(void) console_putc((int) ' ');
				}

				p = &cmd->param_data[0];
				console_set_cursor(0, line + 4);

				for (j = 0 ; j < i; j++) {
					if (isprint((int) *p)) {
						(void) console_putc((int) *p);
					} else {
						(void) console_putc((int) '.');
					}
					(void) console_puts("  ");
					p++;
				}
			}
		}
	} else {
		console_clear_line(line + 2);
		console_clear_line(line + 3);
		console_clear_line(line + 4);
	}
}


