/**
 * @file main.c
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdbool.h>

#include "dmxmonitor.h"
#include "hardware.h"
#include "console.h"
#include "led.h"
#include "dmx.h"

#include "software_version.h"

extern "C" {

void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {}
void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {}

#if !defined(UINT32_MAX)
  #define UINT32_MAX  ((uint32_t)-1)
#endif

void notmain(void) {
	uint32_t micros_previous = 0;

	uint32_t updates_per_seconde_min = UINT32_MAX;
	uint32_t updates_per_seconde_max = (uint32_t)0;
	uint32_t slots_in_packet_min = UINT32_MAX;
	uint32_t slots_in_packet_max = (uint32_t)0;
	uint32_t slot_to_slot_min = UINT32_MAX;
	uint32_t slot_to_slot_max = (uint32_t)0;
	uint32_t break_to_break_min = UINT32_MAX;
	uint32_t break_to_break_max = (uint32_t)0;

	hardware_init();

	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hardware_board_get_model(), __DATE__, __TIME__);
	printf("DMX Real-time Monitor");

	dmx_init();
	dmx_set_port_direction(DMX_PORT_DIRECTION_INP, true);

	DMXMonitor dmx_monitor;

	dmx_monitor.Start();

	console_set_cursor(0, 22);
	console_puts("DMX updates/sec\n");
	console_puts("Slots in packet\n");
	console_puts("Slot to slot\n");
	console_puts("Break to break");

	hardware_watchdog_init();

	for(;;) {
		hardware_watchdog_feed();

		if (dmx_get_updates_per_seconde() == 0) {
			dmx_monitor.SetData(0, NULL, 0);
		} else {
			const uint8_t *p = dmx_get_available();
			if (p != 0) {
				const struct _dmx_data *dmx_statistics = (struct _dmx_data *) p;
				const uint16_t length = (uint16_t) (dmx_statistics->statistics.slots_in_packet);
				dmx_monitor.SetData(0, ++p, length); // Skip DMX START CODE
			}
		}

		const uint32_t micros_now = hardware_micros();

		if (micros_now - micros_previous > (uint32_t) (1E6 / 2)) {
			const uint8_t *dmx_data = dmx_get_current_data();
			const struct _dmx_data *dmx_statistics = (struct _dmx_data *)dmx_data;
			const uint32_t dmx_updates_per_seconde = dmx_get_updates_per_seconde();

			if (dmx_updates_per_seconde == 0) {
				console_set_cursor(20, 22);
				console_puts("---");
				console_set_cursor(20, 23);
				console_puts("---");
				console_set_cursor(20, 24);
				console_puts("---");
				console_set_cursor(17, 25);
				console_puts("-------");
			} else {
				updates_per_seconde_min = MIN(dmx_updates_per_seconde, updates_per_seconde_min);
				updates_per_seconde_max = MAX(dmx_updates_per_seconde, updates_per_seconde_max);
				slots_in_packet_min = MIN(dmx_statistics->statistics.slots_in_packet, slots_in_packet_min);
				slots_in_packet_max = MAX(dmx_statistics->statistics.slots_in_packet, slots_in_packet_max);
				slot_to_slot_min = MIN(dmx_statistics->statistics.slot_to_slot, slot_to_slot_min);
				slot_to_slot_max = MAX(dmx_statistics->statistics.slot_to_slot, slot_to_slot_max);
				break_to_break_min = MIN(dmx_statistics->statistics.break_to_break, break_to_break_min);
				break_to_break_max = MAX(dmx_statistics->statistics.break_to_break, break_to_break_max);
				console_set_cursor(20, 22);
				printf("%3d     %3d / %d", (int)dmx_updates_per_seconde, (int)updates_per_seconde_min , (int)updates_per_seconde_max);
				console_set_cursor(20, 23);
				printf("%3d     %3d / %d", (int)dmx_statistics->statistics.slots_in_packet, (int)slots_in_packet_min, (int)slots_in_packet_max);
				console_set_cursor(20, 24);
				printf("%3d     %3d / %d", (int)dmx_statistics->statistics.slot_to_slot, (int)slot_to_slot_min, (int)slot_to_slot_max);
				console_set_cursor(17, 25);
				printf("%6d  %6d / %d", (int)dmx_statistics->statistics.break_to_break, (int)break_to_break_min, (int)break_to_break_max);
			}

			micros_previous = micros_now;
		}

		led_blink();
	}
}

}
