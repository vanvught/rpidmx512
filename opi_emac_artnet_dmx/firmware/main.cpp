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

#include <cstdint>

#include "hal.h"
#include "h3/hal_watchdog.h"
#include "configstore.h"
#include "network.h"
#include "displayudf.h"
#include "json/displayudfparams.h"
#include "json/dmxsendparams.h"
#include "dmxsend.h"
#include "dmxnodenode.h"
#include "dmxnodemsgconst.h"
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
void RebootHandler()
{
    Dmx::Get()->Blackout();
    ArtNetNode::Get()->Stop();
}
} // namespace hal

static constexpr uint32_t kPortIndex = 0;

int main() // NOLINT
{
    hal::Init();
    DisplayUdf display;
    ConfigStore config_store;
    network::Init();
    FirmwareVersion fw(kSoftwareVersion, __DATE__, __TIME__);
    FlashCodeInstall flash_code_install;

    fw.Print("Art-Net 4, Universes: " STR(DMXNODE_PORTS) " DMX/RDM");

    Dmx dmx;

    json::DmxSendParams dmx_params;
    dmx_params.Load();
    dmx_params.Set();

    DmxSend dmx_send;
    dmx_send.Print();

    DmxNodeNode dmx_node_node;
    dmx_node_node.SetOutput(&dmx_send);

    const auto kPortDirection =
        (dmx_node_node.GetPortDirection(kPortIndex) == dmxnode::PortDirection::kOutput ? dmx::PortDirection::kOutput : dmx::PortDirection::kInput);
    dmx.SetPortDirection(kPortIndex, kPortDirection, false);

    const auto kIsRdmEnabled = dmx_node_node.GetRdm();
 
#if defined(NODE_SHOWFILE)
    ShowFile showfile;
    showfile.Print();
#endif

    dmx_node_node.Print();

    const auto kActivePorts = dmx_node_node.GetActiveInputPorts() + dmx_node_node.GetActiveOutputPorts();

    display.SetTitle("Art-Net 4 %s", kPortDirection == dmx::PortDirection::kInput ? "DMX Input" : (kIsRdmEnabled ? "RDM" : "DMX Output"));
    display.Set(2, displayudf::Labels::kIp);
    display.Set(3, displayudf::Labels::kVersion);
    display.Set(4, displayudf::Labels::kHostname);
    display.Set(5, displayudf::Labels::kUniversePortA);

    json::DisplayUdfParams displayudf_params;
    displayudf_params.Load();
    displayudf_params.SetAndShow();

    RemoteConfig remote_config(kIsRdmEnabled ? remoteconfig::Output::RDM : remoteconfig::Output::DMX, kActivePorts);

    display.TextStatus(DmxNodeMsgConst::START, console::Colours::kConsoleYellow);

    dmx_node_node.Start();

    display.TextStatus(DmxNodeMsgConst::STARTED, console::Colours::kConsoleGreen);

    hal::WatchdogInit();

    for (;;)
    {
        hal::WatchdogFeed();
        network::Run();
        dmx_node_node.Run();
#if defined(NODE_SHOWFILE)
        showfile.Run();
#endif
        display.Run();
        hal::Run();
    }
}
