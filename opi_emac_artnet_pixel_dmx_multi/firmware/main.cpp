/**
 * @file main.cpp
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "h3/hal.h"
#include "h3/hal_watchdog.h"
#include "network.h"
#include "displayudf.h"
#include "json/displayudfparams.h"
#include "dmxnodenode.h"
#include "dmxnodemsgconst.h"
#include "artnettriggerhandler.h"
#include "pixeltype.h"
#include "pixeltestpattern.h"
#include "json/pixeldmxparams.h"
#include "pixeldmxmulti.h"
#include "firmware/jamstapl/handleroled.h"
#include "firmware/pixeldmx/show.h"
#include "json/dmxsendparams.h"
#include "dmxsend.h"
#include "dmxnodewith4.h"
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
    PixelDmxMulti::Get().Blackout();
    Dmx::Get()->Blackout();
    ArtNetNode::Get()->Stop();
}
} // namespace hal

int main() // NOLINT
{
    hal::Init();
    DisplayUdf display;
    ConfigStore config_store;
    network::Init();
    FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
    FlashCodeInstall spiflash_install;

    fw.Print("Art-Net 4 Pixel controller {8x 4 Universes} / 2x DMX");

    DmxNodeNode dmxnode_node;

    // Pixel - 32 Universes
    PixelDmxMulti pixeldmx_multi;

    json::PixelDmxParams pixeldmx_params;
    pixeldmx_params.Load();
    pixeldmx_params.Set();

    PixelOutputMulti::Get()->SetJamSTAPLDisplay(new HandlerOled);

    const auto kPixelOutputPorts = pixeldmx_multi.GetOutputPorts();
    const auto kTestPattern = common::FromValue<pixelpatterns::Pattern>(ConfigStore::Instance().DmxLedGet(&common::store::DmxLed::test_pattern));

    PixelTestPattern pixeltest_pattern(kTestPattern, kPixelOutputPorts);

    // DMX - 2 Universes

    Dmx dmx;

    json::DmxSendParams dmxparams;
    dmxparams.Load();
    dmxparams.Set();

    uint32_t dmx_universes = 0;

    for (uint32_t port_index = dmxnode::kDmxportOffset; port_index < dmxnode::kMaxPorts; port_index++)
    {
        const auto kDmxPortIndex = port_index - dmxnode::kDmxportOffset;

        if (dmxnode_node.GetPortDirection(port_index) == dmxnode::PortDirection::kOutput)
        {
            dmx.SetPortDirection(kDmxPortIndex, dmx::PortDirection::kOutput, false);
            dmx_universes++;
        }
    }

    DmxSend dmx_send;

    // DmxNodeWith4

    DmxNodeWith4<CONFIG_DMXNODE_DMX_PORT_OFFSET> dmxNodeWith4(
        (PixelTestPattern::Get()->GetPattern() != pixelpatterns::Pattern::kNone) ? nullptr : &pixeldmx_multi, (dmx_universes != 0) ? &dmx_send : nullptr);
    dmxNodeWith4.Print();

    ArtNetTriggerHandler artnet_trigger_handler(&dmxNodeWith4, &pixeldmx_multi);

    dmxnode_node.SetOutput(&dmxNodeWith4);
    dmxnode_node.Print();

#if defined(NODE_SHOWFILE)
    ShowFile showfile;
    showfile.Print();
#endif

    display.SetTitle("ArtNet 4 Pixel %dx%d", kPixelOutputPorts, pixeldmx_multi.GetCount());
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
