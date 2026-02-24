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

#include <cstdio>
#include <cstdint>

#include "h3/hal_watchdog.h"
#include "displayudf.h"
#include "json/displayudfparams.h"
#include "rdmresponder.h"
#include "rdmpersonality.h"
#include "json/pixeldmxparams.h"
#include "pixeldmxparamsrdm.h"
#include "pixeltestpattern.h"
#include "pixeldmx.h"
#include "firmware/pixeldmx/show.h"
#if !defined(NO_EMAC)
#include "network.h"
#include "remoteconfig.h"
#endif
#include "flashcodeinstall.h"
#include "configstore.h"
#include "firmwareversion.h"
#include "software_version.h"
#include "is_config_mode.h"
#include "common/utils/utils_enum.h"
#include "configurationstore.h"

namespace hal
{
void RebootHandler()
{
    PixelDmx::Get().Blackout();
}
} // namespace hal

int main() // NOLINT
{
    config_mode_init();
    hal::Init();
    DisplayUdf display;
    ConfigStore config_store;
#if !defined(NO_EMAC)    
    network::Init();
#endif    
    FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
    FlashCodeInstall spiflash_install;

    const auto kIsConfigMode = is_config_mode();

    fw.Print("RDM Responder");

    PixelDmx pixeldmx;

    json::PixelDmxParams pixeldmx_params;
    pixeldmx_params.Load();
    pixeldmx_params.Set();

    const auto kTestPattern = common::FromValue<pixelpatterns::Pattern>(ConfigStore::Instance().DmxLedGet(&common::store::DmxLed::test_pattern));

    PixelTestPattern pixel_test_pattern(kTestPattern, 1);

    PixelDmxParamsRdm pixeldmx_paramsrdm;
	
	auto& rdm_device =rdm::device::Device::Instance();
	rdm_device.SetProductCategory(E120_PRODUCT_CATEGORY_FIXTURE);
	rdm_device.SetProductDetail(E120_PRODUCT_DETAIL_LED);

#if defined(CONFIG_RDM_MANUFACTURER_PIDS_SET)
    static constexpr auto kPersonalityCount = static_cast<uint32_t>(pixel::Type::UNDEFINED);
    RDMPersonality* personalities[kPersonalityCount];

    for (uint32_t index = 0; index < kPersonalityCount; index++)
    {
        const auto* description = pixel::GetType(static_cast<pixel::Type>(index));
        personalities[index] = new RDMPersonality(description, &pixeldmx);
    }

    RDMResponder rdm_responder(personalities, kPersonalityCount, static_cast<uint32_t>(pixeldmx.GetType()) + 1U);
#else
    char description[rdm::personality::DESCRIPTION_MAX_LENGTH];
    pixeldmx::paramsdmx::SetPersonalityDescription(description);

    RDMPersonality* personalities[2] = {new RDMPersonality(description, &pixeldmx), new RDMPersonality("Config mode", &pixeldmx_paramsrdm)};
    RDMResponder rdm_responder(personalities, 2);
#endif
    rdm_responder.Init();
    rdm_responder.Start();
    rdm_responder.DmxDisableOutput(!kIsConfigMode && (kTestPattern != pixelpatterns::Pattern::kNone));
    rdm_responder.Print();

    if (kIsConfigMode)
    {
        pixeldmx_paramsrdm.Print();
    }
    else
    {
        pixeldmx.Print();
    }

    if (kIsConfigMode)
    {
        puts("Config mode");
    }
    else if (kTestPattern != pixelpatterns::Pattern::kNone)
    {
        printf("Test pattern : %s [%u]\n", PixelPatterns::GetName(kTestPattern), static_cast<uint32_t>(kTestPattern));
    }

#if !defined(NO_EMAC)
    RemoteConfig remote_config(remoteconfig::Output::PIXEL);
#endif

    display.SetTitle("RDM Responder Pixel 1");
    display.Set(2, displayudf::Labels::kVersion);
    display.Set(6, displayudf::Labels::kDmxStartAddress);

    json::DisplayUdfParams displayudf_params;
    displayudf_params.Load();
    displayudf_params.SetAndShow();

    common::firmware::pixeldmx::Show(7);

    if (kIsConfigMode)
    {
        display.ClearLine(3);
        display.ClearLine(4);
        display.Write(4, "Config Mode");
        display.ClearLine(5);
    }

    hal::statusled::SetMode(hal::statusled::Mode::NORMAL);
    hal::WatchdogInit();

    for (;;)
    {
        hal::WatchdogFeed();
        rdm_responder.Run();
#if !defined(NO_EMAC)
        network::Run();
#endif
        pixel_test_pattern.Run();
        display.Run();
        hal::Run();
    }
}
