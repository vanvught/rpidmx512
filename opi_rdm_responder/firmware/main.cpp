/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "hardware.h"
#include "network.h"
#if !defined(NO_EMAC)
# include "net/apps/mdns.h"
#endif

#include "displayudf.h"
#include "displayudfparams.h"

#include "rdmresponder.h"
#include "rdmpersonality.h"
#include "rdmdeviceparams.h"
#include "rdmsensorsparams.h"
#if defined (CONFIG_RDM_ENABLE_SUBDEVICES)
# include "rdmsubdevicesparams.h"
#endif

#include "factorydefaults.h"

#include "pixeldmxparams.h"
#include "pixeldmxparamsrdm.h"
#include "pixeltestpattern.h"

#include "ws28xxdmx.h"

#if !defined(NO_EMAC)
# include "remoteconfig.h"
# include "remoteconfigparams.h"
#endif

#include "flashcodeinstall.h"
#include "configstore.h"

#include "firmwareversion.h"
#include "software_version.h"

#include "is_config_mode.h"

namespace hal {
void reboot_handler() {
	WS28xx::Get()->Blackout();
}
}  // namespace hal

int main() {
	config_mode_init();
	Hardware hw;
	DisplayUdf display;
	ConfigStore configStore;
	Network nw;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashCodeInstall spiFlashInstall;

	const auto isConfigMode = is_config_mode();

	fw.Print("RDM Responder");

	PixelDmxConfiguration pixelDmxConfiguration;

	PixelDmxParams pixelDmxParams;
	pixelDmxParams.Load();
	pixelDmxParams.Set();

	WS28xxDmx pixelDmx;

	const auto nTestPattern = static_cast<pixelpatterns::Pattern>(pixelDmxParams.GetTestPattern());
	PixelTestPattern pixelTestPattern(nTestPattern, 1);

	PixelDmxParamsRdm pixelDmxParamsRdm;

	char aDescription[rdm::personality::DESCRIPTION_MAX_LENGTH];
	snprintf(aDescription, sizeof(aDescription) - 1U, "%s:%u G%u [%s]",
			pixel::pixel_get_type(pixelDmxConfiguration.GetType()),
			pixelDmxConfiguration.GetCount(),
			pixelDmxConfiguration.GetGroupingCount(),
			pixel::pixel_get_map(pixelDmxConfiguration.GetMap()));

	RDMPersonality *personalities[2] = {
			new RDMPersonality(aDescription, &pixelDmx),
			new RDMPersonality("Config mode", &pixelDmxParamsRdm)
	};

	RDMResponder rdmResponder(personalities, 2);

	rdmResponder.SetProductCategory(E120_PRODUCT_CATEGORY_FIXTURE);
	rdmResponder.SetProductDetail(E120_PRODUCT_DETAIL_LED);

	RDMSensorsParams rdmSensorsParams;

	rdmSensorsParams.Load();
	rdmSensorsParams.Set();

#if defined (CONFIG_RDM_ENABLE_SUBDEVICES)
	RDMSubDevicesParams rdmSubDevicesParams;

	rdmSubDevicesParams.Load();
	rdmSubDevicesParams.Set();
#endif

	rdmResponder.Init();

	RDMDeviceParams rdmDeviceParams;
	rdmDeviceParams.Load();
	rdmDeviceParams.Set(&rdmResponder);

	rdmResponder.Start();
	rdmResponder.DmxDisableOutput(!isConfigMode && (nTestPattern != pixelpatterns::Pattern::NONE));

	rdmResponder.Print();

	if (isConfigMode) {
		pixelDmxParamsRdm.Print();
	} else {
		pixelDmx.Print();
	}

	if (isConfigMode) {
		puts("Config mode");
	} else if (nTestPattern != pixelpatterns::Pattern::NONE) {
		printf("Test pattern : %s [%u]\n", PixelPatterns::GetName(nTestPattern), static_cast<uint32_t>(nTestPattern));
	}

#if !defined(NO_EMAC)
	RemoteConfig remoteConfig(remoteconfig::Node::RDMRESPONDER, remoteconfig::Output::PIXEL);

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load();
	remoteConfigParams.Set(&remoteConfig);
#endif

	display.SetTitle("RDM Responder Pixel 1");
	display.Set(2, displayudf::Labels::VERSION);
	display.Set(6, displayudf::Labels::DMX_START_ADDRESS);

	DisplayUdfParams displayUdfParams;

	displayUdfParams.Load();
	displayUdfParams.Set(&display);

	display.Show();
	display.Printf(7, "%s:%d G%d %s",
			pixel::pixel_get_type(pixelDmxConfiguration.GetType()),
			pixelDmxConfiguration.GetCount(),
			pixelDmxConfiguration.GetGroupingCount(),
			pixel::pixel_get_map(pixelDmxConfiguration.GetMap()));

	if (isConfigMode) {
		display.ClearLine(3);
		display.ClearLine(4);
		display.Write(4, "Config Mode");
		display.ClearLine(5);
	} else if (nTestPattern != pixelpatterns::Pattern::NONE) {
		display.ClearLine(6);
		display.Printf(6, "%s:%u", PixelPatterns::GetName(nTestPattern), static_cast<uint32_t>(nTestPattern));
	}

	hw.SetMode(hardware::ledblink::Mode::NORMAL);
	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		rdmResponder.Run();
#if !defined(NO_EMAC)
		nw.Run();
#endif
		pixelTestPattern.Run();
		display.Run();
		hw.Run();
	}
}
