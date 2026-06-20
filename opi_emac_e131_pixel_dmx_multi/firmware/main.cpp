/**
 * @file main.cpp
 */
/* Copyright (C) 2021-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "board.h"
#include "watchdog.h"
#include "network.h"
#include "displayudf.h"
#include "json/displayudfparams.h"
#include "dmxnodenode.h"
#include "dmxnodemsgconst.h"
#include "pixeltype.h"
#include "pixeltestpattern.h"
#include "json/pixeldmxparams.h"
#include "pixeldmxmulti.h"
#include "firmware/jamstapl/handleroled.h"
#include "json/dmxsendparams.h"
#include "firmware/pixeldmx/show.h"
#include "dmxsend.h"
#include "dmxnodewith4.h"
#if defined(NODE_SHOWFILE)
#include "showfile.h"
#endif
#include "remoteconfig.h"
#include "flashcodeinstall.h"
#include "configstore.h"
#include "firmware/firmwareversion.h"
#include "software_version.h"
#include "common/utils/utils_enum.h"

namespace board {
void RebootHandler() {
    PixelDmxMulti::Get().Blackout();
    Dmx::Get()->Blackout();
    E131Bridge::Get()->Stop();
}
} // namespace board

int main() // NOLINT
{
    board::Init();
    DisplayUdf display;
    ConfigStore config_store;
    network::Init();
    FirmwareVersion fw(kSoftwareVersion, __DATE__, __TIME__);
    FlashCodeInstall spiflash_install;

    fw.Print("sACN Pixel controller {8x 4 Universes} / 2x DMX");

    DmxNodeNode dmxnode_node;

    // Pixel - 32 Universes
    PixelDmxMulti pixeldmx_multi;
    PixelTestPattern pixeltest_pattern(pixelpatterns::Pattern::kNone, 8);

    json::PixelDmxParams pixeldmx_params;
    pixeldmx_params.Load();
    pixeldmx_params.Set();

    PixelOutputMulti::Get()->SetJamSTAPLDisplay(new HandlerOled);

    // DMX - 2 Universes
    Dmx dmx;

    json::DmxSendParams dmxparams;
    dmxparams.Load();
    dmxparams.Set();

    uint32_t dmx_universes = 0;

    for (uint32_t port_index = dmxnode::kDmxportOffset; port_index < dmxnode::kMaxPorts; port_index++) {
        const auto kDmxPortIndex = port_index - dmxnode::kDmxportOffset;

        if (dmxnode_node.PortDirection(port_index) == dmxnode::Direction::kOutput) {
            dmx.SetPortDirection(kDmxPortIndex, dmx::Direction::kOutput, false);
            dmx_universes++;
        }
    }

    DmxSend dmx_send;

    // DmxNodeWith4
    DmxNodeWith4<CONFIG_DMXNODE_DMX_PORT_OFFSET> dmxNodeWith4((pixeltest_pattern.GetPattern() != pixelpatterns::Pattern::kNone) ? nullptr : &pixeldmx_multi, (dmx_universes != 0) ? &dmx_send : nullptr);
    dmxNodeWith4.Print();

    dmxnode_node.SetOutput(&dmxNodeWith4);
	dmxnode_node.Print();
	
#if defined(NODE_SHOWFILE)
    ShowFile showfile;
    showfile.Print();
#endif

    dmxnode_node.Print();

    display.SetTitle("sACN Pixel %dx%d", dmxnode_node.GetActiveOutputPorts(), pixeldmx_multi.GetCount());
    display.Set(2, displayudf::Labels::kVersion);
    display.Set(3, displayudf::Labels::kHostname);
    display.Set(4, displayudf::Labels::kIp);

    json::DisplayUdfParams displayudf_params;
    displayudf_params.Load();
    displayudf_params.SetAndShow();

    common::firmware::pixeldmx::Show(7, pixeltest_pattern.GetPattern());

    RemoteConfig remote_config(remoteconfig::Output::PIXEL, dmxnode_node.GetActiveOutputPorts());

    display.TextStatus(DmxNodeMsgConst::START, ansi::Colours::Colour::kYellow);

    dmxnode_node.Start();

    display.TextStatus(DmxNodeMsgConst::STARTED, ansi::Colours::Colour::kGreen);

    watchdog::Init();

    for (;;) {
        watchdog::Feed();
        network::Run();
        dmxnode_node.Run();
#if defined(NODE_SHOWFILE)
        showfile.Run();
#endif
        pixeltest_pattern.Run();
        display.Run();
        board::Run();
    }
}
