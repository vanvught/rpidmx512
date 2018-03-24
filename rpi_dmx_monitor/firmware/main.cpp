/**
 * @file main.c
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "hardwarebaremetal.h"
#include "ledblinkbaremetal.h"

#include "console.h"

#include "dmxreceiver.h"
#include "dmxmonitor.h"

#include "software_version.h"

#ifndef MAX
 #define MAX(a,b)	(((a) > (b)) ? (a) : (b))
 #define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif

extern "C" {

void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {}
void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {}

void notmain(void) {
	HardwareBaremetal hw;
	LedBlinkBaremetal lb;
	uint8_t nHwTextLength;
	uint32_t nMicrosPrevious = (uint32_t) 0;
	uint32_t nUpdatesPerSecondeMin = UINT32_MAX;
	uint32_t nUpdatesPerSecondeMax = (uint32_t) 0;
	uint32_t nSlotsInPacketMin = UINT32_MAX;
	uint32_t nSlotsInPacketMax = (uint32_t) 0;
	uint32_t nSotToSlotMin = UINT32_MAX;
	uint32_t nSlotToSlotMax = (uint32_t) 0;
	uint32_t nBreakToBreakMin = UINT32_MAX;
	uint32_t nBreakToBreakMax = (uint32_t) 0;
	int16_t nLength;

	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetBoardName(nHwTextLength), __DATE__, __TIME__);
	printf("DMX Real-time Monitor");

	hw.SetLed(HARDWARE_LED_ON);

	DMXMonitor dmxmonitor;
	dmxmonitor.Cls();

	console_set_cursor(0, 22);
	console_puts("DMX updates/sec\n");
	console_puts("Slots in packet\n");
	console_puts("Slot to slot\n");
	console_puts("Break to break");

	hw.WatchdogInit();

	DMXReceiver dmxreceiver;
	dmxreceiver.SetOutput(&dmxmonitor);
	dmxreceiver.Start();

	for(;;) {

		hw.WatchdogFeed();

		(void) dmxreceiver.Run(nLength);

		const uint32_t nMicrosNow = hw.Micros();

		if (nMicrosNow - nMicrosPrevious > (uint32_t) (1E6 / 2)) {
			const uint8_t *dmx_data = dmxreceiver.GetDmxCurrentData();
			const struct TDmxData *dmx_statistics = (struct TDmxData *)dmx_data;
			const uint32_t dmx_updates_per_seconde = dmxreceiver.GetUpdatesPerSecond();

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
				nUpdatesPerSecondeMin = MIN(dmx_updates_per_seconde, nUpdatesPerSecondeMin);
				nUpdatesPerSecondeMax = MAX(dmx_updates_per_seconde, nUpdatesPerSecondeMax);
				nSlotsInPacketMin = MIN(dmx_statistics->Statistics.SlotsInPacket, nSlotsInPacketMin);
				nSlotsInPacketMax = MAX(dmx_statistics->Statistics.SlotsInPacket, nSlotsInPacketMax);
				nSotToSlotMin = MIN(dmx_statistics->Statistics.SlotToSlot, nSotToSlotMin);
				nSlotToSlotMax = MAX(dmx_statistics->Statistics.SlotToSlot, nSlotToSlotMax);
				nBreakToBreakMin = MIN(dmx_statistics->Statistics.BreakToBreak, nBreakToBreakMin);
				nBreakToBreakMax = MAX(dmx_statistics->Statistics.BreakToBreak, nBreakToBreakMax);
				console_set_cursor(20, 22);
				printf("%3d     %3d / %d", (int) dmx_updates_per_seconde, (int) nUpdatesPerSecondeMin, (int) nUpdatesPerSecondeMax);
				console_set_cursor(20, 23);
				printf("%3d     %3d / %d", (int) dmx_statistics->Statistics.SlotsInPacket, (int) nSlotsInPacketMin, (int) nSlotsInPacketMax);
				console_set_cursor(20, 24);
				printf("%3d     %3d / %d", (int) dmx_statistics->Statistics.SlotToSlot, (int) nSotToSlotMin, (int) nSlotToSlotMax);
				console_set_cursor(17, 25);
				printf("%6d  %6d / %d", (int) dmx_statistics->Statistics.BreakToBreak, (int) nBreakToBreakMin, (int) nBreakToBreakMax);
			}

			nMicrosPrevious = nMicrosNow;
		}

		lb.Run();
	}
}

}
