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
#include <cstdio>

#include "h3/hal.h"
#include "h3/hal_watchdog.h"
#include "network.h"
#include "console.h"
#include "h3/showsystime.h"
#include "displayudf.h"
#include "json/displayudfparams.h"
#include "displayhandler.h"
#include "dmxnodenode.h"
#include "dmxnodemsgconst.h"
#include "dmxmonitor.h"
#include "dmxnode.h"
#if defined(NODE_RDMNET_LLRP_ONLY)
#include "rdmnetdevice.h"
#include "rdmdevice.h"
#include "rdm_e120.h"
#endif
#if defined(NODE_SHOWFILE)
#include "showfile.h"
#endif
#include "remoteconfig.h"
#include "flashcodeinstall.h"
#include "configstore.h"
#include "firmwareversion.h"
#include "software_version.h"

namespace hal
{
void RebootHandler() {}
} // namespace hal

int main() // NOLINT
{
    hal::Init();
    DisplayUdf display;
    ConfigStore config_store;
    network::Init();
    FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
    FlashCodeInstall spiflash_install;

    console::Clear();

    fw.Print();

    console::Puts("sACN E1.31 ");
    console::SetFgColour(console::Colours::kConsoleGreen);
    console::Puts("Real-time DMX Monitor");
    console::SetFgColour(console::Colours::kConsoleWhite);
    console::SetTopRow(2);

    DmxNodeNode dmxnode_node;
    ShowSystime show_systime;

#if defined(NODE_RDMNET_LLRP_ONLY)
    auto& rdm_device = rdm::device::Device::Instance();
    rdm_device.SetProductCategory(E120_PRODUCT_CATEGORY_DATA_DISTRIBUTION);
    rdm_device.SetProductDetail(E120_PRODUCT_DETAIL_ETHERNET_NODE);

	RDMNetDevice llrp_only_device;
	llrp_only_device.Print();
#endif

#if defined(NODE_SHOWFILE)
    ShowFile showfile;
    showfile.Print();
#endif

    DmxMonitor monitor;
    // There is support for HEX output only

    dmxnode_node.SetOutput(&monitor);

    monitor.Cls();
    console::SetTopRow(20);
    console::ClearTopRow();

    dmxnode_node.Print();

    display.SetTitle("sACN E1.31 Monitor");
    display.Set(2, displayudf::Labels::kIp);
    display.Set(3, displayudf::Labels::kHostname);
    display.Set(4, displayudf::Labels::kVersion);

    json::DisplayUdfParams displayudf_params;
    displayudf_params.Load();
    displayudf_params.SetAndShow();

    RemoteConfig remote_config(remoteconfig::Output::MONITOR, 0);

    display.TextStatus(DmxNodeMsgConst::START, console::Colours::kConsoleYellow);

    dmxnode_node.Start();

    display.TextStatus(DmxNodeMsgConst::STARTED, console::Colours::kConsoleGreen);

    hal::WatchdogInit();

    for (;;)
    {
        hal::WatchdogFeed();
        network::Run();
        dmxnode_node.Run();
#if defined(NODE_SHOWFILE)
        showfile.Run();
#endif
        show_systime.Run();
        display.Run();
        hal::Run();
    }
}
