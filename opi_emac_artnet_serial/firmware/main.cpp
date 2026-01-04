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

#include <cstdint>

#include "h3/hal_watchdog.h"
#include "network.h"
#include "displayudf.h"
#include "json/displayudfparams.h"
#include "dmxnodenode.h"
#include "dmxnodemsgconst.h"
#include "dmxserial.h"
#include "json/dmxserialparams.h"
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
    ArtNetNode::Get()->Stop();
}
} // namespace hal

int main()
{
    hal::Init();
    DisplayUdf display;
    ConfigStore config_store;
    network::Init();
    FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
    FlashCodeInstall spiflash_install;

    fw.Print("Art-Net 4 Node Serial [UART/SPI/I2C] {1 Universe}");

    DmxSerial dmx_serial;

    json::DmxSerialParams dmx_serial_params;
    dmx_serial_params.Load();
    dmx_serial_params.Set();

    DmxNodeNode dmx_node_node;
    dmx_node_node.SetOutput(&dmx_serial);
    dmx_node_node.Print();

    dmx_serial.Init();
    dmx_serial.Print();

#if defined(NODE_SHOWFILE)
    ShowFile showfile;

    {
    }

    showfile.Print();
#endif

    display.SetTitle("Art-Net 4 %s", dmx_serial.GetType());
    display.Set(2, displayudf::Labels::kIp);
    display.Set(3, displayudf::Labels::kVersion);
    display.Set(4, displayudf::Labels::kHostname);

    json::DisplayUdfParams displayudf_params;
    displayudf_params.Load();
    displayudf_params.SetAndShow();

    const auto kFilesCount = dmx_serial.GetFilesCount();

    if (kFilesCount == 0)
    {
        display.Printf(7, "No files [SDCard?]");
    }
    else
    {
        display.Printf(7, "Channel%s: %d", kFilesCount == 1 ? "" : "s", kFilesCount);
    }

    RemoteConfig remote_config(remoteconfig::Output::SERIAL, dmx_node_node.GetActiveOutputPorts());

#if defined(NODE_RDMNET_LLRP_ONLY)
    auto& rdm_device = RdmDevice::Get();
    rdm_device.SetProductCategory(E120_PRODUCT_CATEGORY_DATA_DISTRIBUTION);
    rdm_device.SetProductDetail(E120_PRODUCT_DETAIL_ETHERNET_NODE);
    rdm_device.Init();
    rdm_device.Print();

    RDMNetDevice llrp_only_device;
#endif

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
