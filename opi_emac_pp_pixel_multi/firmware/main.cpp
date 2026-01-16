/**
 * @file main.cpp
 */
/* Copyright (C) 2022-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstdio>

#include "h3/hal.h"
#include "h3/hal_watchdog.h"
#include "network.h"
#include "apps/mdns.h"
#include "displayudf.h"
#include "json/displayudfparams.h"
#include "firmware/jamstapl/handleroled.h"
#include "firmware/pixeldmx/show.h"
#include "pp.h"
#include "pixeltype.h"
#include "pixeltestpattern.h"
#include "json/pixeldmxparams.h"
#include "pixeldmxmulti.h"
#if defined(NODE_RDMNET_LLRP_ONLY)
#include "rdmnetdevice.h"
#include "rdmdevice.h"
#include "rdm_e120.h"
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
    PixelPusher::Get()->Stop();
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

    fw.Print("PixelPusher controller 8x 4U");

    PixelPusher pp;

    network::apps::mdns::ServiceRecordAdd(nullptr, network::apps::mdns::Services::kPp, "type=PixelPusher");

    PixelDmxMulti pixeldmx_multi;

    json::PixelDmxParams pixeldmx_params;
    pixeldmx_params.Load();
    pixeldmx_params.Set();

    PixelOutputMulti::Get()->SetJamSTAPLDisplay(new HandlerOled);

    const auto kActivePorts = pixeldmx_multi.GetOutputPorts();

    pp.SetCount(pixeldmx_multi.GetGroups(), kActivePorts, false);

    const auto kTestPattern = common::FromValue<pixelpatterns::Pattern>(ConfigStore::Instance().DmxLedGet(&common::store::DmxLed::test_pattern));

    PixelTestPattern pixeltest_pattern(kTestPattern, kActivePorts);

    pixeldmx_multi.Print();

    pp.SetOutput(&pixeldmx_multi);
    pp.Print();

#if defined(NODE_RDMNET_LLRP_ONLY)
    auto& rdm_device = RdmDevice::Get();
    rdm_device.SetProductCategory(E120_PRODUCT_CATEGORY_FIXTURE);
    rdm_device.SetProductDetail(E120_PRODUCT_DETAIL_LED);
    rdm_device.Init();
    rdm_device.Print();

    RDMNetDevice llrp_only_device;
#endif

    display.SetTitle("PixelPusher %d", kActivePorts);
    display.Set(2, displayudf::Labels::kVersion);
    display.Set(3, displayudf::Labels::kHostname);
    display.Set(4, displayudf::Labels::kIp);
    display.Set(5, displayudf::Labels::kDefaultGateway);

    json::DisplayUdfParams displayudf_params;
    displayudf_params.Load();
    displayudf_params.SetAndShow();

    common::firmware::pixeldmx::Show(7);

    RemoteConfig remote_config(remoteconfig::Output::PIXEL, kActivePorts);

    display.TextStatus("PixelPusher Start", console::Colours::kConsoleYellow);

    pp.Start();

    display.TextStatus("PixelPusher Started", console::Colours::kConsoleGreen);

    hal::WatchdogInit();

    for (;;)
    {
        hal::WatchdogFeed();
        network::Run();
        pp.Run();
        pixeltest_pattern.Run();
        display.Run();
        hal::Run();
    }
}
