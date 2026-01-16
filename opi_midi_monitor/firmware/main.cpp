/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2016-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "h3/hal_watchdog.h"
#include "display.h"
#include "console.h"
#include "flashcodeinstall.h"
#if !defined(NO_EMAC)
#include "network.h"
#include "networkconst.h"
#include "configstore.h"
#endif
#include "midi.h"
#include "midimonitor.h"
#include "software_version.h"
#include "firmwareversion.h"

namespace hal
{
void RebootHandler() {}
} // namespace hal

int main() // NOLINT
{
    hal::Init();
    Display display;
#if !defined(NO_EMAC)
    ConfigStore config_store;
    network::Init();
#endif
    FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
    FlashCodeInstall spiflash_install;

    fw.Print("MIDI Monitor");

    Midi midi;
    midi.SetActiveSense(true);
    midi.Init(midi::Direction::INPUT);

#if !defined(NO_EMAC)
#endif

    console::Clear();
    fw.Print();
    printf("MIDI Monitor, baudrate : %d, interface : %s", midi.GetBaudrate(), EXT_MIDI_UART_NAME);

    MidiMonitor monitor;
    monitor.Init();

    hal::WatchdogInit();

    for (;;)
    {
        hal::WatchdogFeed();
        monitor.Run();
#if !defined(NO_EMAC)
        network::Run();
#endif
        hal::Run();
    }
}
