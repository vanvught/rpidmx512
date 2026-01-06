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
#include <cassert>

#include "h3/hal_watchdog.h"
#include "emac/network.h"
#include "display.h"
#include "apps/mdns.h"
#include "oscclient.h"
#include "json/oscclientparams.h"
#include "oscclientmsgconst.h"
#include "buttonsset.h"
#include "buttonsgpio.h"
#include "buttonsmcp.h"
#include "flashcodeinstall.h"
#include "configstore.h"
#include "remoteconfig.h"
#include "firmwareversion.h"
#include "software_version.h"
#include "displayhandler.h"

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

    fw.Print("OSC Client");

    OscClient osc_client;

    json::OscClientParams osc_client_params;
    osc_client_params.Load();
    osc_client_params.Set();

    osc_client.Print();

    ButtonsSet* buttons_set;

    auto* buttons_mcp = new ButtonsMcp(&osc_client);
    assert(buttons_mcp != nullptr);

    if (buttons_mcp->Start())
    {
        buttons_set = static_cast<ButtonsSet*>(buttons_mcp);
        osc_client.SetLedHandler(buttons_mcp);
    }
    else
    {
        delete buttons_mcp;

        auto* buttons_gpio = new ButtonsGpio(&osc_client);
        assert(buttons_gpio != nullptr);

        buttons_gpio->Start();

        buttons_set = static_cast<ButtonsSet*>(buttons_gpio);
        osc_client.SetLedHandler(buttons_gpio);
    }

    RemoteConfig remote_config(remoteconfig::Output::OSC, buttons_set->GetButtonsCount());


    for (uint32_t i = 1; i < 7; i++)
    {
        display.ClearLine(i);
    }

    display.Write(1, "Eth OSC Client");
    display.Printf(2, "%s.local",  network::iface::GetHostName());
    display.Printf(3, "IP: " IPSTR " %c", IP2STR(net::GetPrimaryIp()), network::iface::IsDhcpKnown() ? ( network::iface::IsDhcpUsed() ? 'D' : 'S') : ' ');
    display.Printf(4, "S : " IPSTR, IP2STR(osc_client.GetServerIP()));
    display.Printf(5, "O : %d", osc_client.GetPortOutgoing());
    display.Printf(6, "I : %d", osc_client.GetPortIncoming());

    display.TextStatus(OscClientMsgConst::START, console::Colours::kConsoleYellow);

    osc_client.Start();

    display.TextStatus(OscClientMsgConst::STARTED, console::Colours::kConsoleGreen);

    hal::statusled::SetMode(hal::statusled::Mode::NORMAL);
    hal::WatchdogInit();

    for (;;)
    {
        hal::WatchdogFeed();
        network::Run();
        osc_client.Run();
        buttons_set->Run();
        display.Run();
        hal::Run();
    }
}
