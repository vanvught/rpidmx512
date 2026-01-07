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
#include "display.h"
#include "oscserver.h"
#include "firmware/pixeldmx/show.h"
#include "json/oscserverparams.h"
#include "oscservermsgconst.h"
#include "pixeltype.h"
#include "pixeltestpattern.h"
#include "json/pixeldmxparams.h"
#include "pixeldmx.h"
#include "handler.h"
#include "remoteconfig.h"
#include "flashcodeinstall.h"
#include "configstore.h"
#include "firmwareversion.h"
#include "software_version.h"
#include "common/utils/utils_enum.h"
#include "configurationstore.h"

namespace hal
{
void RebootHandler()
{
    PixelDmx::Get().Blackout();
}
} // namespace hal

int main() // NOLINT
{
    hal::Init();
    Display display;
    ConfigStore config_store;
    network::Init();
    FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
    FlashCodeInstall spiflash_install;

    fw.Print("OSC Server Pixel controller {1x Universe}");

    OscServer oscserver;

    json::OscServerParams oscserver_params;
    oscserver_params.Load();
    oscserver_params.Set();

    PixelDmx pixeldmx;

    json::PixelDmxParams pixeldmx_params;
    pixeldmx_params.Load();
    pixeldmx_params.Set();

    const auto kTestPattern = common::FromValue<pixelpatterns::Pattern>(ConfigStore::Instance().DmxLedGet(&common::store::DmxLed::test_pattern));

    PixelTestPattern pixeltest_pattern(kTestPattern, 1);

    display.Printf(7, "%s:%d G%d", pixel::GetType(pixeldmx.GetType()), pixeldmx.GetCount(), pixeldmx.GetGroupingCount());

    oscserver.SetOutput(&pixeldmx);
    oscserver.SetOscServerHandler(new Handler(&pixeldmx));

    oscserver.Print();
    pixeldmx.Print();

    uint8_t text_length;

    display.Printf(1, "OSC Pixel 1");
    display.Write(2, hal::BoardName(text_length));
    display.Printf(3, "IP: " IPSTR " %c", IP2STR(network::GetPrimaryIp()), network::iface::IsDhcpKnown() ? ( network::iface::Dhcp() ? 'D' : 'S') : ' ');
    display.Printf(4, "In: %d", oscserver.GetPortIncoming());
    display.Printf(5, "Out: %d", oscserver.GetPortOutgoing());
    
    common::firmware::pixeldmx::Show(7);

    RemoteConfig remote_config(remoteconfig::Output::PIXEL, 1);

    display.TextStatus(OscServerMsgConst::START, console::Colours::kConsoleYellow);

    oscserver.Start();

    display.TextStatus(OscServerMsgConst::STARTED, console::Colours::kConsoleGreen);

    hal::WatchdogInit();

    for (;;)
    {
        hal::WatchdogFeed();
        network::Run();
        pixeltest_pattern.Run();
        display.Run();
        hal::Run();
    }
}
