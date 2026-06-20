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

#include "ansi_colour.h"
#include "board.h"
#include "console/console_fb.h"
#include "watchdog.h"
#include "network.h"
#include "console.h"
#include "h3/showsystime.h"
#include "displayudf.h"
#include "json/displayudfparams.h"
#include "displayhandler.h"
#include "dmxnodenode.h"
#include "dmxnodemsgconst.h"
#include "timecode.h"
#include "dmxmonitor.h"
#if defined(NODE_SHOWFILE)
#include "showfile.h"
#endif
#include "remoteconfig.h"
#include "flashcodeinstall.h"
#include "configstore.h"
#include "firmware/firmwareversion.h"
#include "software_version.h"

namespace board {
void RebootHandler() {}
} // namespace board

int main() { // NOLINT
    board::Init();
    DisplayUdf display;
    ConfigStore config_store;
    network::Init();
    FirmwareVersion fw(kSoftwareVersion, __DATE__, __TIME__);
    FlashCodeInstall spiflash_install;

    console::Clear();

    fw.Print();

    console::Puts("Art-Net 4 Node ");
    console::SetFgColour(console::Colour::kGreen);
    console::Puts("Real-time DMX Monitor");
    console::SetFgColour(console::Colour::kWhite);
    console::SetTopRow(2);

    DmxNodeNode dmxnode_node;
    ShowSystime show_systime;

    TimeCode timecode;
    timecode.Start();

#if defined(NODE_SHOWFILE)
    ShowFile showfile;
    showfile.Print();
#endif

    DmxMonitor monitor;
    // There is support for HEX output only

    dmxnode_node.SetArtTimeCodeCallbackFunction(TimeCode::StaticCallbackFunction);
    dmxnode_node.SetOutput(&monitor);

    monitor.Cls();
    console::SetTopRow(20);
    console::ClearTopRow();

    dmxnode_node.Print();

    display.SetTitle("Art-Net 4 Monitor");
    display.Set(2, displayudf::Labels::kIp);
    display.Set(3, displayudf::Labels::kHostname);
    display.Set(4, displayudf::Labels::kVersion);

    json::DisplayUdfParams displayudf_params;
    displayudf_params.Load();
    displayudf_params.SetAndShow();

    RemoteConfig remote_config(remoteconfig::Output::MONITOR, 1);

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
        show_systime.Run();
        display.Run();
        board::Run();
    }
}
