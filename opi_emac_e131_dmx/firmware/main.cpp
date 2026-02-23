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

#include "h3/hal_watchdog.h"
#include "network.h"
#include "displayudf.h"
#include "json/displayudfparams.h"
#include "displayhandler.h"
#include "json/dmxsendparams.h"
#include "dmxsend.h"
#include "dmxnodenode.h"
#include "dmxnodemsgconst.h"
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
void RebootHandler()
{
    Dmx::Get()->Blackout();
    E131Bridge::Get()->Stop();
}
} // namespace hal

static constexpr uint32_t kPortIndex = 0;

int main() // NOLINT
{
    hal::Init();
    DisplayUdf display;
    ConfigStore config_store;
    network::Init();
    FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
    FlashCodeInstall spiflash_install;

    fw.Print("sACN E1.31 DMX {1 Universe}");

    Dmx dmx;

    json::DmxSendParams dmxparams;
    dmxparams.Load();
    dmxparams.Set();

    DmxSend dmx_send;
    dmx_send.Print();

    DmxNodeNode dmxnode_node;
    dmxnode_node.SetOutput(&dmx_send);

    const auto kPortDirection =
        (dmxnode_node.GetPortDirection(kPortIndex) == dmxnode::PortDirection::kOutput ? dmx::PortDirection::kOutput : dmx::PortDirection::kInput);
    dmx.SetPortDirection(kPortIndex, kPortDirection, false);

    const auto kActivePorts = dmxnode_node.GetActiveInputPorts() + dmxnode_node.GetActiveOutputPorts();

#if defined(NODE_RDMNET_LLRP_ONLY)
    auto& rdm_device = RdmDevice::Get();
    rdm_device.SetProductCategory(E120_PRODUCT_CATEGORY_FIXTURE);
    rdm_device.SetProductDetail(E120_PRODUCT_DETAIL_LED);
    rdm_device.Init();
    rdm_device.Print();

    RDMNetDevice llrp_only_device;
#endif

#if defined(NODE_SHOWFILE)
    ShowFile showfile;
    showfile.Print();
#endif

    dmxnode_node.Print();

    display.SetTitle("sACN E1.31 DMX %s", kPortDirection == dmx::PortDirection::kInput ? "Input" : "Output");
    display.Set(2, displayudf::Labels::kIp);
    display.Set(3, displayudf::Labels::kVersion);
    display.Set(4, displayudf::Labels::kHostname);
    display.Set(5, displayudf::Labels::kUniversePortA);

    json::DisplayUdfParams displayudf_params;
    displayudf_params.Load();
    displayudf_params.SetAndShow();

    RemoteConfig remote_config(remoteconfig::Output::DMX, kActivePorts);

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
        display.Run();
        hal::Run();
    }
}
