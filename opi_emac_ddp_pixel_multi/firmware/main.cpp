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
#include "ddpdisplay.h"
#include "pixeltype.h"
#include "pixeltestpattern.h"
#include "json/pixeldmxparams.h"
#include "pixeldmxmulti.h"
#include "firmware/jamstapl/handleroled.h"
#include "firmware/pixeldmx/show.h"
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
    DdpDisplay::Get()->Stop();
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

    fw.Print("DDP Pixel controller 8x 4U");

    DdpDisplay ddpdisplay;

    PixelDmxMulti pixeldmx_multi;

    json::PixelDmxParams pixeldmx_params;
    pixeldmx_params.Load();
    pixeldmx_params.Set();

    PixelOutputMulti::Get()->SetJamSTAPLDisplay(new HandlerOled);

    const auto kActivePorts = pixeldmx_multi.GetOutputPorts();

    ddpdisplay.SetCount(pixeldmx_multi.GetGroups(), pixeldmx_multi.GetLedsPerPixel(), kActivePorts);

    const auto kTestPattern = common::FromValue<pixelpatterns::Pattern>(ConfigStore::Instance().DmxLedGet(&common::store::DmxLed::test_pattern));

    PixelTestPattern pixeltest_pattern(kTestPattern, kActivePorts);

    ddpdisplay.SetOutput(&pixeldmx_multi);
    ddpdisplay.Print();

#if defined(NODE_RDMNET_LLRP_ONLY)
    auto& rdm_device =rdm::device::Device::Instance();
    rdm_device.SetProductCategory(E120_PRODUCT_CATEGORY_FIXTURE);
    rdm_device.SetProductDetail(E120_PRODUCT_DETAIL_LED);

	RDMNetDevice llrp_only_device;
	llrp_only_device.Print();
#endif

    display.SetTitle("DDP Pixel %d", kActivePorts);
    display.Set(2, displayudf::Labels::kVersion);
    display.Set(3, displayudf::Labels::kHostname);
    display.Set(4, displayudf::Labels::kIp);
    display.Set(5, displayudf::Labels::kDefaultGateway);
    display.Set(6, displayudf::Labels::kBoardname);

    json::DisplayUdfParams displayudf_params;
    displayudf_params.Load();
    displayudf_params.SetAndShow();

    common::firmware::pixeldmx::Show(7);

    RemoteConfig remote_config(remoteconfig::Output::PIXEL, kActivePorts);

    display.TextStatus("DDP Display Start", console::Colours::kConsoleYellow);

    ddpdisplay.Start();

    display.TextStatus("DDP Display Started", console::Colours::kConsoleGreen);

    hal::WatchdogInit();

    for (;;)
    {
        hal::WatchdogFeed();
        network::Run();
        pixeltest_pattern.Run();
        display.Run();
        hal::Run();
    }
}
