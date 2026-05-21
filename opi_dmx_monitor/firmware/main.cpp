/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdio>
#include <cstdint>
#include <algorithm>

#include "h3/hal.h"
#include "h3/console_fb.h"
#include "watchdog.h"
#include "timing.h"
#include "network.h"
#include "displayudf.h"
#include "console.h"
#include "dmxreceiver.h"
#include "dmxmonitor.h"
#include "json/dmxmonitorparams.h"
#include "dmx.h"
#if !defined(NO_EMAC)
#include "network.h"
#include "remoteconfig.h"
#include "configstore.h"
#endif
#include "software_version.h"

static constexpr auto kTopRowStats = 26;

namespace hal {
void RebootHandler() {}
} // namespace hal

int main() // NOLINT
{
    hal::Init();
    DisplayUdf display;
#if !defined(NO_EMAC)
    ConfigStore config_store;
    network::Init();
#endif

    console::Clear();

    uint32_t micros_previous = 0;
    uint32_t nUpdatesPerSecondeMin = UINT32_MAX;
    uint32_t nUpdatesPerSecondeMax = 0;
    uint32_t nSlotsInPacketMin = UINT16_MAX;
    uint32_t nSlotsInPacketMax = 0;
    uint32_t nSotToSlotMin = UINT32_MAX;
    uint32_t nSlotToSlotMax = 0;
    uint32_t break_to_break_min = UINT32_MAX;
    uint32_t break_to_break_max = 0;
    int16_t length;

    printf("DMX Real-time Monitor [V%s] Orange Pi One Compiled on %s at %s\n", kSoftwareVersion, __DATE__, __TIME__);

    DmxMonitor dmxMonitor;

    json::DmxMonitorParams dmx_monitor_params;
    dmx_monitor_params.Load();
    dmx_monitor_params.Set();

#if !defined(NO_EMAC)
    RemoteConfig remote_config(remoteconfig::Output::MONITOR);
#endif

    dmxMonitor.Cls();

    console::SetCursor(0, kTopRowStats);
    console::Puts("DMX updates/sec\n");
    console::Puts("Slots in packet\n");
    console::Puts("Slot to slot\n");
    console::Puts("Break to break");

    DMXReceiver dmxreceiver(&dmxMonitor);
    dmxreceiver.Start();

    watchdog::Init();

    for (;;) {
        watchdog::Feed();

        dmxreceiver.Run(length);

        const auto kMicrosNow = timing::Micros();

        if (kMicrosNow - micros_previous > (1000000 / 2)) {
            micros_previous = kMicrosNow;

            const auto kDmxUpdatesPerSeconde = dmxreceiver.GetUpdatesPerSecond(0);

            console::SaveCursor();

            if (kDmxUpdatesPerSeconde == 0) {
                console::SetCursor(20, kTopRowStats);
                console::Puts("---");
                console::SetCursor(20, kTopRowStats + 1);
                console::Puts("---");
                console::SetCursor(20, kTopRowStats + 2);
                console::Puts("---");
                console::SetCursor(17, kTopRowStats + 3);
                console::Puts("-------");
            } else {
                const auto* dmx_data = dmxreceiver.GetDmxCurrentData(0);
                const auto* dmx_statistics = reinterpret_cast<const struct Data*>(dmx_data);

                nUpdatesPerSecondeMin = std::min(kDmxUpdatesPerSeconde, nUpdatesPerSecondeMin);
                nUpdatesPerSecondeMax = std::max(kDmxUpdatesPerSeconde, nUpdatesPerSecondeMax);

                nSlotsInPacketMin = std::min(dmx_statistics->statistics.slots_in_packet, nSlotsInPacketMin);
                nSlotsInPacketMax = std::max(dmx_statistics->statistics.slots_in_packet, nSlotsInPacketMax);

                nSotToSlotMin = std::min(dmx_statistics->statistics.slot_to_slot, nSotToSlotMin);
                nSlotToSlotMax = std::max(dmx_statistics->statistics.slot_to_slot, nSlotToSlotMax);

                break_to_break_min = std::min(dmx_statistics->statistics.break_to_break, break_to_break_min);
                break_to_break_max = std::max(dmx_statistics->statistics.break_to_break, break_to_break_max);

                console::SetCursor(20, kTopRowStats);
                printf("%3d     %3d / %d", kDmxUpdatesPerSeconde, nUpdatesPerSecondeMin, nUpdatesPerSecondeMax);
                console::SetCursor(20, kTopRowStats + 1);
                printf("%3d     %3d / %d", dmx_statistics->statistics.slots_in_packet, nSlotsInPacketMin, nSlotsInPacketMax);
                console::SetCursor(20, kTopRowStats + 2);
                printf("%3d     %3d / %d", dmx_statistics->statistics.slot_to_slot, nSotToSlotMin, nSlotToSlotMax);
                console::SetCursor(17, kTopRowStats + 3);
                printf("%6d  %6d / %d", dmx_statistics->statistics.break_to_break, break_to_break_min, break_to_break_max);
            }

            console::RestoreCursor();
        }

#if !defined(NO_EMAC)
        network::Run();
#endif
        hal::Run();
    }
}
