/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "ledblink.h"

#include "displayudf.h"
#include "displayrdm.h"

#include "identify.h"
#include "factorydefaults.h"

#include "rdmresponder.h"
#include "rdmpersonality.h"

#include "rdmdeviceparams.h"
#include "rdmsensorsparams.h"
#include "rdmsubdevicesparams.h"

#include "ws28xxdmxparams.h"
#include "ws28xxdmx.h"
#include "pixeldmxparamsrdm.h"
#include "pixeltestpattern.h"

#if !defined(NO_EMAC)
# include "remoteconfig.h"
# include "remoteconfigparams.h"
# include "storeremoteconfig.h"
# include "storenetwork.h"
#endif

#include "spiflashinstall.h"
#include "spiflashstore.h"
#include "storerdmdevice.h"
#include "storews28xxdmx.h"
#include "storerdmdevice.h"
#include "storerdmsensors.h"
#include "storerdmsubdevices.h"
#include "storedisplayudf.h"

#include "firmwareversion.h"
#include "software_version.h"

#include "is_config_mode.h"

extern "C" {

void notmain(void) {
	config_mode_init();
	Hardware hw;
	Network nw;
	LedBlink lb;
	DisplayUdf display;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	const auto isConfigMode = is_config_mode();

	fw.Print();

#if !defined(NO_EMAC)
	StoreNetwork storeNetwork;
	nw.SetNetworkStore(&storeNetwork);
	nw.Init(&storeNetwork);
	nw.Print();
#endif

	PixelDmxConfiguration pixelDmxConfiguration;

	StoreWS28xxDmx storeWS28xxDmx;
	WS28xxDmxParams ws28xxparms(&storeWS28xxDmx);

	if (ws28xxparms.Load()) {
		ws28xxparms.Set(&pixelDmxConfiguration);
		ws28xxparms.Dump();
	}

	DisplayRdm displayRdm;

	WS28xxDmx pixelDmx(pixelDmxConfiguration);
	pixelDmx.SetLightSetDisplay(&displayRdm);
	pixelDmx.SetWS28xxDmxStore(&storeWS28xxDmx);

	const auto nTestPattern = static_cast<pixelpatterns::Pattern>(ws28xxparms.GetTestPattern());
	PixelTestPattern pixelTestPattern(nTestPattern);

	LightSet *pLightSet;

	if (isConfigMode) {
		auto *pPixelDmxParamsRdm = new PixelDmxParamsRdm(&storeWS28xxDmx);
		assert(pPixelDmxParamsRdm != nullptr);

		pLightSet = pPixelDmxParamsRdm;
	} else {
		pLightSet = &pixelDmx;
	}

	StoreRDMSensors storeRdmSensors;
	RDMSensorsParams rdmSensorsParams(&storeRdmSensors);

	if (rdmSensorsParams.Load()) {
		rdmSensorsParams.Set();
		rdmSensorsParams.Dump();
	}

	StoreRDMSubDevices storeRdmSubDevices;
	RDMSubDevicesParams rdmSubDevicesParams(&storeRdmSubDevices);

	if (rdmSubDevicesParams.Load()) {
		rdmSubDevicesParams.Set();
		rdmSubDevicesParams.Dump();
	}

	Identify identify;

	char aDescription[RDM_PERSONALITY_DESCRIPTION_MAX_LENGTH];
	snprintf(aDescription, sizeof(aDescription) -1, "%s:%u G%u [%s]",
			PixelType::GetType(pixelDmxConfiguration.GetType()),
			pixelDmxConfiguration.GetCount(),
			pixelDmxConfiguration.GetGroupingCount(),
			PixelType::GetMap(pixelDmxConfiguration.GetMap()));

	RDMPersonality personality(aDescription, pLightSet->GetDmxFootprint());

	RDMResponder rdmResponder(&personality, pLightSet);
	rdmResponder.Init();

	StoreRDMDevice storeRdmDevice;
	RDMDeviceParams rdmDeviceParams(&storeRdmDevice);

	if (rdmDeviceParams.Load()) {
		rdmDeviceParams.Set(&rdmResponder);
		rdmDeviceParams.Dump();
	}

	rdmResponder.SetRDMDeviceStore(&storeRdmDevice);

	FactoryDefaults factoryDefaults;
	rdmResponder.SetRDMFactoryDefaults(&factoryDefaults);
	rdmResponder.Start();
	rdmResponder.DmxDisableOutput(nTestPattern != pixelpatterns::Pattern::NONE);

	rdmResponder.Print();
	pLightSet->Print();

#if !defined(NO_EMAC)
	RemoteConfig remoteConfig(remoteconfig::Node::RDMNET_LLRP_ONLY, remoteconfig::Output::PIXEL);
	RemoteConfigParams remoteConfigParams(new StoreRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
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
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
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

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		rdmResponder.Run();
		spiFlashStore.Flash();
		identify.Run();
#if !defined(NO_EMAC)
		nw.Run();
		remoteConfig.Run();
#endif
		lb.Run();
		display.Run();
		if (__builtin_expect((PixelTestPattern::GetPattern() != pixelpatterns::Pattern::NONE), 0)) {
			pixelTestPattern.Run();
		}
	}
}

}
