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

#include <cstdio>

#include "h3/hal.h"
#include "h3/hal_watchdog.h"
#include "network.h"
#include "displayudf.h"
#include "json/displayudfparams.h"
#include "dmxnodenode.h"
#include "dmxnodemsgconst.h"
#include "rdmpersonality.h"
#include "artnetrdmresponder.h"
#include "tlc59711.h"
#include "json/tlc59711dmxparams.h"
#include "tlc59711dmx.h"
#include "dmxnodechain.h"
#include "flashcodeinstall.h"
#include "configstore.h"
#include "remoteconfig.h"
#include "sparkfundmx.h"
#include "sparkfundmxconst.h"
#if defined(NODE_SHOWFILE)
#include "showfile.h"
#endif
#include "firmwareversion.h"
#include "software_version.h"
#include "common/utils/utils_enum.h"
#include "configurationstore.h"

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

    fw.Print("Art-Net 4 Stepper L6470");

    display.TextStatus(SparkFunDmxConst::MSG_INIT, console::Colours::kConsoleYellow);

    DmxNodeChain dmxnode_chain;

    SparkFunDmx spark_fun_dmx;
    dmxnode_chain.SetSparkfunDmx(&spark_fun_dmx);

    spark_fun_dmx.ReadConfigFiles();

    auto motors_connected = spark_fun_dmx.GetMotorsConnected();

    TLC59711Dmx tlc59711dmx;

    json::Tlc59711DmxParams pwmledparms;
    pwmledparms.Load();
    pwmledparms.Set();

    const auto kType = common::FromValue<tlc59711::Type>(ConfigStore::Instance().DmxLedGet(&common::store::DmxLed::type));
    const auto kIsLedTypeSet = kType != tlc59711::Type::kUndefined;

    char description[64];

    if (kIsLedTypeSet)
    {
        dmxnode_chain.SetTLC59711Dmx(&tlc59711dmx);
        snprintf(description, sizeof(description) - 1, "Sparkfun [%d] with %s [%d]", motors_connected, tlc59711::GetType(tlc59711dmx.GetType()),
                 tlc59711dmx.GetCount());
    }
    else
    {
        snprintf(description, sizeof(description) - 1, "Sparkfun [%d]", motors_connected);
    }

    DmxNodeNode dmxnode_node;

    dmxnode_node.SetLongName(description);
    dmxnode_node.SetOutput(&dmxnode_chain);
	
	auto& rdm_device =rdm::device::Device::Instance();

	rdm_device.SetProductCategory(E120_PRODUCT_CATEGORY_FIXTURE);
	rdm_device.SetProductDetail(E120_PRODUCT_DETAIL_LED);
	rdm_device.Init();
	rdm_device.Print();

    RDMPersonality* rdm_personalities[1] = {new RDMPersonality(description, &dmxnode_chain)};

    ArtNetRdmResponder rdm_responder(rdm_personalities, 1);

    dmxnode_node.SetRdmResponder(&rdm_responder);
    dmxnode_node.Print();

    dmxnode_chain.Print();

#if defined(NODE_SHOWFILE)
    ShowFile showfile;

    {
    }

    showfile.Print();
#endif

    display.SetTitle("Art-Net 4 L6470");
    display.Set(2, displayudf::Labels::kIp);
    display.Set(3, displayudf::Labels::kVersion);
    display.Set(4, displayudf::Labels::kHostname);
    display.Set(5, displayudf::Labels::kDmxStartAddress);

    json::DisplayUdfParams displayudf_params;
    displayudf_params.Load();
    displayudf_params.SetAndShow();

    if (kIsLedTypeSet)
    {
        display.Printf(7, "%s:%d", tlc59711::GetType(tlc59711dmx.GetType()), tlc59711dmx.GetCount());
    }

    RemoteConfig remote_config(remoteconfig::Output::STEPPER, dmxnode_node.GetActiveOutputPorts());

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
