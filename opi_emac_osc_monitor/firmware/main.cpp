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

#include <cstdint>

#include "h3/hal.h"
#include "h3/hal_watchdog.h"
#include "hal_boardinfo.h"
#include "emac/network.h"
#include "ip4/ip4_address.h"
#include "console.h"
#include "h3/showsystime.h"
#include "display.h"
#include "oscserver.h"
#include "json/oscserverparams.h"
#include "oscservermsgconst.h"
#include "dmxmonitor.h"
#include "firmwareversion.h"
#include "software_version.h"
#include "flashcodeinstall.h"
#include "configstore.h"
#include "remoteconfig.h"

namespace hal
{
void RebootHandler() {}
} // namespace hal

int main() // NOLINT
{
    hal::Init();
    Display display;
    ConfigStore config_store;
    network::Init();
    FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
    FlashCodeInstall spiflash_install;

    console::Clear();

    fw.Print();

    console::Puts("OSC ");
    console::SetFgColour(console::Colours::kConsoleGreen);
    console::Puts("Real-time DMX Monitor");
    console::SetFgColour(console::Colours::kConsoleWhite);
    console::SetTopRow(2);

    ShowSystime show_systime;

    OscServer oscserver;

    json::OscServerParams oscserver_params;
    oscserver_params.Load();
    oscserver_params.Set();

     DmxMonitor monitor;
    // There is support for HEX output only
    oscserver.SetOutput(&monitor);
    monitor.Cls();
    console::SetTopRow(20);
    console::ClearTopRow();

    oscserver.Print();

    uint8_t text_length;

    display.Printf(1, "OSC Monitor");
    display.Write(2, hal::BoardName(text_length));
    display.Printf(3, "IP: " IPSTR " %c", IP2STR(network::GetPrimaryIp()), network::iface::IsDhcpKnown() ? ( network::iface::Dhcp() ? 'D' : 'S') : ' ');
    display.Printf(4, "In: %d", oscserver.GetPortIncoming());
    display.Printf(5, "Out: %d", oscserver.GetPortOutgoing());

    RemoteConfig remote_config(remoteconfig::Output::MONITOR, 1);

    display.TextStatus(OscServerMsgConst::START, console::Colours::kConsoleYellow);

    oscserver.Start();

    display.TextStatus(OscServerMsgConst::STARTED, console::Colours::kConsoleGreen);

    hal::WatchdogInit();

    for (;;)
    {
        hal::WatchdogFeed();
        network::Run();
        show_systime.Run();
        display.Run();
        hal::Run();
    }
}
