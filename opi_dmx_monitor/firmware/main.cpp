/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <algorithm>

#include "hardware.h"
#include "ledblink.h"

#include "console.h"

#include "dmxreceiver.h"

#include "dmxmonitor.h"
#include "dmxmonitorparams.h"

#include "software_version.h"

static constexpr auto TOP_ROW_STATS = 26;

extern "C" {

void notmain(void) {
	Hardware hw;
	LedBlink lb;

	uint32_t nMicrosPrevious = 0;
	uint32_t nUpdatesPerSecondeMin = UINT32_MAX;
	uint32_t nUpdatesPerSecondeMax = 0;
	uint32_t nSlotsInPacketMin = UINT32_MAX;
	uint32_t nSlotsInPacketMax = 0;
	uint32_t nSotToSlotMin = UINT32_MAX;
	uint32_t nSlotToSlotMax = 0;
	uint32_t nBreakToBreakMin = UINT32_MAX;
	uint32_t nBreakToBreakMax = 0;
	int16_t nLength;

	printf("DMX Real-time Monitor [V%s] Orange Pi One Compiled on %s at %s\n", SOFTWARE_VERSION, __DATE__, __TIME__);

	hw.SetLed(HARDWARE_LED_ON);

	DMXMonitor dmxmonitor;

	DMXMonitorParams monitorparams;

	if (monitorparams.Load()) {
		monitorparams.Set(&dmxmonitor);
		monitorparams.Dump();
	}

	dmxmonitor.Cls();

	console_set_cursor(0, TOP_ROW_STATS);
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

		dmxreceiver.Run(nLength);

		const uint32_t nMicrosNow = hw.Micros();

		if (nMicrosNow - nMicrosPrevious > (1000000 / 2)) {
			const uint32_t dmx_updates_per_seconde = dmxreceiver.GetUpdatesPerSecond();

			console_save_cursor();

			if (dmx_updates_per_seconde == 0) {
				console_set_cursor(20, TOP_ROW_STATS);
				console_puts("---");
				console_set_cursor(20, TOP_ROW_STATS + 1);
				console_puts("---");
				console_set_cursor(20, TOP_ROW_STATS + 2);
				console_puts("---");
				console_set_cursor(17, TOP_ROW_STATS + 3);
				console_puts("-------");
			} else {
				const uint8_t *dmx_data = dmxreceiver.GetDmxCurrentData();
				const struct TDmxData *dmx_statistics = reinterpret_cast<const struct TDmxData*>(dmx_data);

				nUpdatesPerSecondeMin = std::min(dmx_updates_per_seconde, nUpdatesPerSecondeMin);
				nUpdatesPerSecondeMax = std::max(dmx_updates_per_seconde, nUpdatesPerSecondeMax);

				nSlotsInPacketMin = std::min(dmx_statistics->Statistics.SlotsInPacket, nSlotsInPacketMin);
				nSlotsInPacketMax = std::max(dmx_statistics->Statistics.SlotsInPacket, nSlotsInPacketMax);

				nSotToSlotMin = std::min(dmx_statistics->Statistics.SlotToSlot, nSotToSlotMin);
				nSlotToSlotMax = std::max(dmx_statistics->Statistics.SlotToSlot, nSlotToSlotMax);

				nBreakToBreakMin = std::min(dmx_statistics->Statistics.BreakToBreak, nBreakToBreakMin);
				nBreakToBreakMax = std::max(dmx_statistics->Statistics.BreakToBreak, nBreakToBreakMax);

				console_set_cursor(20, TOP_ROW_STATS);
				printf("%3d     %3d / %d", dmx_updates_per_seconde, nUpdatesPerSecondeMin, nUpdatesPerSecondeMax);
				console_set_cursor(20, TOP_ROW_STATS + 1);
				printf("%3d     %3d / %d", dmx_statistics->Statistics.SlotsInPacket, nSlotsInPacketMin, nSlotsInPacketMax);
				console_set_cursor(20, TOP_ROW_STATS + 2);
				printf("%3d     %3d / %d", dmx_statistics->Statistics.SlotToSlot, nSotToSlotMin, nSlotToSlotMax);
				console_set_cursor(17, TOP_ROW_STATS + 3);
				printf("%6d  %6d / %d", dmx_statistics->Statistics.BreakToBreak, nBreakToBreakMin, nBreakToBreakMax);
			}

			console_restore_cursor();

			nMicrosPrevious = nMicrosNow;
		}

		lb.Run();
	}
}

}
