/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "h3/hal_watchdog.h"
#include "network.h"
#include "displayudf.h"
#include "json/displayudfparams.h"
#include "dmxnodenode.h"
#include "dmxnodemsgconst.h"
#include "artnetcontroller.h"
#include "artnetoutput.h"
#include "dmxnode.h"
#if defined(NODE_RDMNET_LLRP_ONLY)
#include "rdmnetdevice.h"
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
    E131Bridge::Get()->Stop();
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

    fw.Print("sACN E1.31 -> Art-Net");

    DmxNodeNode dmxnode_node;

    dmxnode_node.SetDisableSynchronize(true);

    ArtNetController controller;
    ArtNetOutput artnetOutput;

    dmxnode_node.SetOutput(&artnetOutput);
    dmxnode_node.Print();

    controller.Print();

    display.SetTitle("sACN E1.31 Art-Net %d", dmxnode_node.GetActiveOutputPorts());
    display.Set(2, displayudf::Labels::kIp);

    json::DisplayUdfParams displayudf_params;
    displayudf_params.Load();
    displayudf_params.SetAndShow();

    RemoteConfig remote_config(remoteconfig::Output::ARTNET, dmxnode_node.GetActiveOutputPorts());

#if defined(NODE_RDMNET_LLRP_ONLY)
	RDMNetDevice llrp_only_device;
	llrp_only_device.Print();
#endif

    display.TextStatus(DmxNodeMsgConst::START, console::Colours::kConsoleYellow);

    dmxnode_node.Start();
    controller.Start();

    display.TextStatus(DmxNodeMsgConst::STARTED, console::Colours::kConsoleGreen);

    hal::WatchdogInit();

    for (;;)
    {
        hal::WatchdogFeed();
        network::Run();
        dmxnode_node.Run();
        controller.Run();
        display.Run();
        hal::Run();
    }
}
