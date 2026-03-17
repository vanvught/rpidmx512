/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#pragma GCC optimize("-fprefetch-loop-arrays")

#include <cstdint>

#include "h3/hal.h"
#include "h3/hal_watchdog.h"
#include "network.h"
#include "displayudf.h"
#include "json/displayudfparams.h"
#include "dmxnodenode.h"
#include "dmxnodemsgconst.h"
#include "firmware/pixeldmx/show.h"
#include "pixeltype.h"
#include "pixeltestpattern.h"
#include "json/pixeldmxparams.h"
#include "pixeldmx.h"
#if defined(NODE_RDMNET_LLRP_ONLY)
#include "rdmnetdevice.h"
#endif
#if defined(NODE_SHOWFILE)
#include "showfile.h"
#endif
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
    E131Bridge::Get()->Stop();
}
} // namespace hal

int main() // NOLINT
{
    hal::Init();
    DisplayUdf display;
    ConfigStore config_store;
    network::Init();
    FirmwareVersion fw(kSoftwareVersion, __DATE__, __TIME__);
    FlashCodeInstall spiflash_install;

    fw.Print("sACN Pixel controller {1x 4 Universes}");

    DmxNodeNode dmxnode_node;

    // Pixel - 4 Universes
    PixelDmx pixeldmx;

    json::PixelDmxParams pixeldmx_params;
    pixeldmx_params.Load();
    pixeldmx_params.Set();

    const auto kTestPattern = common::FromValue<pixelpatterns::Pattern>(ConfigStore::Instance().DmxLedGet(&common::store::DmxLed::test_pattern));

    PixelTestPattern pixeltest_pattern(kTestPattern, 1);

    if (PixelTestPattern::Get()->GetPattern() != pixelpatterns::Pattern::kNone)
    {
        dmxnode_node.SetOutput(nullptr);
    }
    else
    {
        dmxnode_node.SetOutput(&pixeldmx);
    }

#if defined(NODE_RDMNET_LLRP_ONLY)
    RDMNetDevice llrp_only_device;
	llrp_only_device.Print();
#endif

#if defined(NODE_SHOWFILE)
    ShowFile showfile;
    showfile.Print();
#endif

    dmxnode_node.Print();
    pixeldmx.Print();

    display.SetTitle("sACN Pixel 1x4U");
    display.Set(2, displayudf::Labels::kVersion);
    display.Set(3, displayudf::Labels::kHostname);
    display.Set(4, displayudf::Labels::kIp);

    json::DisplayUdfParams displayudf_params;
    displayudf_params.Load();
    displayudf_params.SetAndShow();

    common::firmware::pixeldmx::Show(7);

    RemoteConfig remote_config(remoteconfig::Output::PIXEL, dmxnode_node.GetActiveOutputPorts());

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
        pixeltest_pattern.Run();
        display.Run();
        hal::Run();
    }
}
