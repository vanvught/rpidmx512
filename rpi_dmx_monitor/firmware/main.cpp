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
#include "led.h"
#include "dmx.h"
#include "software_version.h"

extern "C" {

extern void dmx_monitor_init(void);
extern void dmx_monitor(void);

void notmain(void) {

	hardware_init();

	printf("%s Compiled on %s at %s\n", hardware_get_board_model(), __DATE__, __TIME__);
	printf("DMX Real-time Monitor [V%s]", SOFTWARE_VERSION);

	dmx_init();
	dmx_set_port_direction(DMX_PORT_DIRECTION_INP, true);

	DMXMonitor dmx_monitor;

	dmx_monitor.Start();

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

		led_blink();
	}
}

}
