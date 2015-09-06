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
#include <stdint.h>
#include <stdarg.h>

#include "sys_time.h"
#include "hardware.h"
#include "util.h"
#include "console.h"
#include "monitor.h"
#include "dmx.h"
#if defined(DMX_SLAVE)
#elif defined(RDM_CONTROLLER) || defined(LOGIC_ANALYZER)
#include "sniffer.h"
#elif defined(RDM_RESPONDER)
#endif
#if defined(RDM_CONTROLLER) || defined(RDM_RESPONDER)
#include "rdm_device_info.h"
#endif

/**
 * @ingroup monitor
 *
 * @param line
 * @param fmt
 */
void monitor_line(const int line, const char *fmt, ...) {
	va_list va;

	console_clear_line(line);

	if (fmt != NULL) {
		va_start(va, fmt);
		(void) vprintf(fmt, va);
		va_end(va);
	}
}

/**
 * @ingroup monitor
 *
 * @param line
 */
void monitor_time_uptime(const int line) {
	const uint32_t minute = 60;
	const uint32_t hour = minute * 60;
	const uint32_t day = hour * 24;

	const uint64_t uptime_seconds = hardware_uptime_seconds();

	time_t ltime = 0;
	struct tm *local_time = NULL;

	ltime = sys_time(NULL);
	local_time = localtime(&ltime);

	console_set_cursor(0, line);

	printf("Local time %.2d:%.2d:%.2d, uptime %ld days, %02ld:%02ld:%02ld\n",
			local_time->tm_hour, local_time->tm_min, local_time->tm_sec,
			(long int) (uptime_seconds / day),
			(long int) (uptime_seconds % day) / hour,
			(long int) (uptime_seconds % hour) / minute,
			(long int) uptime_seconds % minute);
}

/**
 * @ingroup monitor
 *
 * @param line
 * @param data_length
 * @param data
 */
void monitor_rdm_data(const int line, const uint16_t data_length, const uint8_t *data) {
	uint8_t i;
	console_clear_line(line);

	printf("RDM Packet length : %d\n", (int) data_length);

	for (i = 0; i < 9; i++) {
		printf("%.2d-%.4d:%.2X  %.2d-%.4d:%.2X %.2d-%.4d:%.2X  %.2d-%.4d:%.2X\n",
				(int) i + 1, (int) data[i], (unsigned int) data[i],
				(int) i + 10, (int) data[i + 9], (unsigned int) data[i + 9],
				(int) i + 19, (int) data[i + 18], (unsigned int) data[i + 18],
				(int) i + 28, (int) data[i + 27], (unsigned int) data[i + 27]);
	}
}

/**
 * @ingroup monitor
 *
 * @param line
 * @param data
 */
void monitor_dmx_data(const int line) {
	uint16_t i;
	uint16_t slots;
	const uint8_t *dmx_data = dmx_get_data();

	if (DMX_PORT_DIRECTION_INP == dmx_get_port_direction()) {
		const volatile struct _dmx_statistics *dmx_statistics = dmx_get_statistics();
		slots = (uint16_t) (dmx_statistics->slots_in_packet + 1);
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

static uint32_t updates_per_seconde_min = UINT32_MAX;
static uint32_t updates_per_seconde_max = (uint32_t)0;
static uint32_t slots_in_packet_min = UINT32_MAX;
static uint32_t slots_in_packet_max = (uint32_t)0;
static uint32_t slot_to_slot_min = UINT32_MAX;
static uint32_t slot_to_slot_max = (uint32_t)0;
static uint32_t break_to_break_min = UINT32_MAX;
static uint32_t break_to_break_max = (uint32_t)0;

/**
 * @ingroup monitor
 *
 */
void monitor_sniffer(void) {
	const volatile struct _total_statistics *total_statistics = dmx_get_total_statistics();
	const uint32_t total_packets = total_statistics->dmx_packets + total_statistics->rdm_packets;

	monitor_dmx_data(MONITOR_LINE_DMX_DATA);
	console_clear_line(MONITOR_LINE_PACKETS);
	printf("Packets : %ld, DMX %ld, RDM %ld\n\n", (long int) total_packets,
			(long int) total_statistics->dmx_packets, (long int) total_statistics->rdm_packets);

#if defined(RDM_CONTROLLER) || defined(LOGIC_ANALYZER)
	const volatile struct _rdm_statistics *rdm_statistics = rdm_statistics_get();

	printf("Discovery          : %ld\n", rdm_statistics->discovery_packets);
	printf("Discovery response : %ld\n", rdm_statistics->discovery_response_packets);
	printf("GET Requests       : %ld\n", rdm_statistics->get_requests);
	printf("SET Requests       : %ld\n", rdm_statistics->set_requests);
#endif
	const volatile struct _dmx_statistics *dmx_statistics = dmx_get_statistics();

	if ((int)dmx_statistics->updates_per_seconde != (int)0) {
		updates_per_seconde_min = MIN(dmx_statistics->updates_per_seconde, updates_per_seconde_min);
		updates_per_seconde_max = MAX(dmx_statistics->updates_per_seconde, updates_per_seconde_max);
		printf("\nDMX updates/sec      %2d      %2d / %2d\n\n", (int)dmx_statistics->updates_per_seconde, (int)updates_per_seconde_min , (int)updates_per_seconde_max);
	} else {
		printf("\nDMX updates/sec --     \n\n");
	}

	if (dmx_statistics->updates_per_seconde != 0) {
		slots_in_packet_min = MIN(dmx_statistics->slots_in_packet, slots_in_packet_min);
		slots_in_packet_max = MAX(dmx_statistics->slots_in_packet, slots_in_packet_max);
		slot_to_slot_min = MIN(dmx_statistics->slot_to_slot, slot_to_slot_min);
		slot_to_slot_max = MAX(dmx_statistics->slot_to_slot, slot_to_slot_max);
		break_to_break_min = MIN(dmx_statistics->break_to_break, break_to_break_min);
		break_to_break_max = MAX(dmx_statistics->break_to_break, break_to_break_max);

		printf("Slots in packet     %3d     %3d / %d\n", (int)dmx_statistics->slots_in_packet, (int)slots_in_packet_min, (int)slots_in_packet_max);
		printf("Slot to slot        %3d     %3d / %d\n", (int)dmx_statistics->slot_to_slot, (int)slot_to_slot_min, (int)slot_to_slot_max);
		printf("Break to break %8ld  %6d / %d\n", (long int)dmx_statistics->break_to_break, (int)break_to_break_min, (int)break_to_break_max);
	} else {
		console_puts("Slots in packet --     \n");
		console_puts("Slot to slot    --     \n");
		console_puts("Break to break  --     \n");
	}
}

#if defined(RDM_CONTROLLER) || defined(RDM_RESPONDER)
void monitor_print_root_device_label(void) {
	struct _rdm_device_info_data rdm_device_info_label;
	rdm_device_info_get_label(0, &rdm_device_info_label);
	console_putsn((const char *)rdm_device_info_label.data, (int) rdm_device_info_label.length);
}
#endif
