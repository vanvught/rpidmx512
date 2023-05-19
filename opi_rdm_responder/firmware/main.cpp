/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
# include "networkconst.h"
#endif

#include "displayudf.h"
#include "display_timeout.h"

#include "rdmresponder.h"
#include "rdmpersonality.h"
#include "rdmdeviceparams.h"
#include "rdmsensorsparams.h"
#if defined (ENABLE_RDM_SUBDEVICES)
# include "rdmsubdevicesparams.h"
#endif

#include "factorydefaults.h"

#include "pixeldmxparams.h"
#include "ws28xxdmx.h"
#include "pixeldmxstartstop.h"
#include "pixeldmxparamsrdm.h"
#include "pixeltestpattern.h"

#if !defined(NO_EMAC)
# include "remoteconfig.h"
# include "remoteconfigparams.h"
# include "storeremoteconfig.h"
# include "storenetwork.h"
#endif

#include "flashcodeinstall.h"
#include "configstore.h"
#include "storerdmdevice.h"
#include "storepixeldmx.h"
#include "storerdmdevice.h"
#include "storerdmsensors.h"
#if defined (ENABLE_RDM_SUBDEVICES)
# include "storerdmsubdevices.h"
#endif
#include "storedisplayudf.h"

#include "firmwareversion.h"
#include "software_version.h"

#include "is_config_mode.h"

void Hardware::RebootHandler() {
	WS28xx::Get()->Blackout();
}

void main() {
	config_mode_init();
	Hardware hw;
	DisplayUdf display;
	ConfigStore configStore;
#if !defined(NO_EMAC)
	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);
	StoreNetwork storeNetwork;
	Network nw(&storeNetwork);
	display.TextStatus(NetworkConst::MSG_NETWORK_STARTED, Display7SegmentMessage::INFO_NONE, CONSOLE_GREEN);
#else
	Network nw;
#endif
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashCodeInstall spiFlashInstall;

	const auto isConfigMode = is_config_mode();

	fw.Print("RDM Responder");
#if !defined(NO_EMAC)
	nw.Print();
#endif

	PixelDmxConfiguration pixelDmxConfiguration;

	StorePixelDmx storePixelDmx;
	PixelDmxParams pixelDmxParams(&storePixelDmx);

	if (pixelDmxParams.Load()) {
		pixelDmxParams.Dump();
		pixelDmxParams.Set(&pixelDmxConfiguration);
	}

	/*
	 * DMX Footprint = (Channels per Pixel * Groups) <= 512 (1 Universe)
	 * Groups = Led count / Grouping count
	 *
	 * Channels per Pixel * (Led count / Grouping count) <= 512
	 * Channels per Pixel * Led count <= 512 * Grouping count
	 *
	 * Led count <= (512 * Grouping count) / Channels per Pixel
	 */

	uint32_t nLedsPerPixel;
	pixeldmxconfiguration::PortInfo portInfo;

	pixelDmxConfiguration.Validate(1 , nLedsPerPixel, portInfo);

	if (pixelDmxConfiguration.GetUniverses() > 1) {
		const auto nCount = (512U * pixelDmxConfiguration.GetGroupingCount()) / nLedsPerPixel;
		pixelDmxConfiguration.SetCount(nCount);
	}

	WS28xxDmx pixelDmx(pixelDmxConfiguration);
	pixelDmx.SetWS28xxDmxStore(&storePixelDmx);

	PixelDmxStartStop pixelDmxStartStop;
	pixelDmx.SetPixelDmxHandler(&pixelDmxStartStop);

	const auto nTestPattern = static_cast<pixelpatterns::Pattern>(pixelDmxParams.GetTestPattern());
	PixelTestPattern pixelTestPattern(nTestPattern, 1);

	PixelDmxParamsRdm pixelDmxParamsRdm(&storePixelDmx);

	char aDescription[rdm::personality::DESCRIPTION_MAX_LENGTH];
	snprintf(aDescription, sizeof(aDescription) - 1U, "%s:%u G%u [%s]",
			PixelType::GetType(pixelDmxConfiguration.GetType()),
			pixelDmxConfiguration.GetCount(),
			pixelDmxConfiguration.GetGroupingCount(),
			PixelType::GetMap(pixelDmxConfiguration.GetMap()));

	RDMPersonality *personalities[2] = { new RDMPersonality(aDescription, &pixelDmx), new RDMPersonality("Config mode", &pixelDmxParamsRdm) };

	RDMResponder rdmResponder(personalities, 2);

	StoreRDMSensors storeRdmSensors;
	RDMSensorsParams rdmSensorsParams(&storeRdmSensors);

	if (rdmSensorsParams.Load()) {
		rdmSensorsParams.Dump();
		rdmSensorsParams.Set();
	}

#if defined (ENABLE_RDM_SUBDEVICES)
	StoreRDMSubDevices storeRdmSubDevices;
	RDMSubDevicesParams rdmSubDevicesParams(&storeRdmSubDevices);

	if (rdmSubDevicesParams.Load()) {
		rdmSubDevicesParams.Dump();
		rdmSubDevicesParams.Set();
	}
#endif

	rdmResponder.Init();

	StoreRDMDevice storeRdmDevice;
	RDMDeviceParams rdmDeviceParams(&storeRdmDevice);
	rdmResponder.SetRDMDeviceStore(&storeRdmDevice);

	if (rdmDeviceParams.Load()) {
		rdmDeviceParams.Dump();
		rdmDeviceParams.Set(&rdmResponder);
	}

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

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if (remoteConfigParams.Load()) {
		remoteConfigParams.Dump();
		remoteConfigParams.Set(&remoteConfig);
	}

	while (configStore.Flash())
		;
#endif

	display.SetTitle("RDM Responder Pixel 1");
	display.Set(2, displayudf::Labels::VERSION);
	display.Set(6, displayudf::Labels::DMX_START_ADDRESS);
	display.Printf(7, "%s:%d G%d %s",
			PixelType::GetType(pixelDmxConfiguration.GetType()),
			pixelDmxConfiguration.GetCount(),
			pixelDmxConfiguration.GetGroupingCount(),
			PixelType::GetMap(pixelDmxConfiguration.GetMap()));

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	if (displayUdfParams.Load()) {
		displayUdfParams.Dump();
		displayUdfParams.Set(&display);
	}

	display.Show();

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
		configStore.Flash();
#if !defined(NO_EMAC)
		nw.Run();
		remoteConfig.Run();
#endif
		if (__builtin_expect((PixelTestPattern::GetPattern() != pixelpatterns::Pattern::NONE), 0)) {
			pixelTestPattern.Run();
		}
		display.Run();
		hw.Run();
	}
}
