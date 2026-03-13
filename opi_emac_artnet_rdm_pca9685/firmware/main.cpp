/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2023-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "h3/hal_watchdog.h"
#include "network.h"
#include "displayudf.h"
#include "json/displayudfparams.h"
#include "dmxnodenode.h"
#include "dmxnodemsgconst.h"
#include "rdmpersonality.h"
#include "artnetrdmresponder.h"
#include "json/pca9685dmxparams.h"
#include "pca9685dmx.h"
#if defined(NODE_SHOWFILE)
#include "showfile.h"
#endif
#include "flashcodeinstall.h"
#include "configstore.h"
#include "remoteconfig.h"
#include "firmwareversion.h"
#include "software_version.h"

namespace hal
{
void RebootHandler()
{
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

    fw.Print("Art-Net 4 PCA9685");

    Pca9685Dmx pca9685_dmx;

    json::Pca9685DmxParams pca9685_dmx_params;
    pca9685_dmx_params.Load();
    pca9685_dmx_params.Set();

    DmxNodeNode dmxnode_node;
    dmxnode_node.SetRdm(static_cast<uint32_t>(0), true);
    dmxnode_node.SetOutput(pca9685_dmx.GetPCA9685DmxSet());
	
	char description[64];
	snprintf(description, sizeof(description) - 1, "PCA9685");
	RDMPersonality* rdm_personalities[1] = {new RDMPersonality(description, pca9685_dmx.GetPCA9685DmxSet())};

	ArtNetRdmResponder rdm_responder(rdm_personalities, 1);
	rdm_responder.Init();	
	rdm_responder.Print();
	
    dmxnode_node.SetRdmResponder(&rdm_responder);
    dmxnode_node.Print();

    pca9685_dmx.Print();

#if defined(NODE_SHOWFILE)
    ShowFile showfile;
    showfile.Print();
#endif

    display.SetTitle("Art-Net 4 PCA9685");
    display.Set(2, displayudf::Labels::kIp);
    display.Set(3, displayudf::Labels::kVersion);
    display.Set(4, displayudf::Labels::kHostname);
    display.Set(5, displayudf::Labels::kDmxStartAddress);

    json::DisplayUdfParams displayudf_params;
    displayudf_params.Load();
    displayudf_params.SetAndShow();

    RemoteConfig remote_config(remoteconfig::Output::PWM, dmxnode_node.GetActiveOutputPorts());

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
