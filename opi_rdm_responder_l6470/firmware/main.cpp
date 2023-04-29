/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "hardware.h"
#include "display.h"
#include "network.h"
#if !defined(NO_EMAC)
# include "networkconst.h"
#endif

#include "rdmresponder.h"
#include "rdmpersonality.h"
#include "rdmdeviceparams.h"
#include "rdmsensorsparams.h"
#if defined (ENABLE_RDM_SUBDEVICES)
# include "rdmsubdevicesparams.h"
#endif

#include "tlc59711dmxparams.h"
#include "tlc59711dmx.h"

#include "lightsetchain.h"

#if !defined(NO_EMAC)
# include "remoteconfig.h"
# include "remoteconfigparams.h"
# include "storeremoteconfig.h"
# include "storenetwork.h"
#endif

#include "flashcodeinstall.h"
#include "configstore.h"
#include "storerdmdevice.h"
#include "storetlc59711.h"
#include "storerdmdevice.h"
#include "storerdmsensors.h"
#if defined (ENABLE_RDM_SUBDEVICES)
# include "storerdmsubdevices.h"
#endif

#include "firmwareversion.h"
#include "software_version.h"

#include "sparkfundmx.h"
#include "storesparkfundmx.h"
#include "storemotors.h"

#include "factorydefaults.h"

void Hardware::RebootHandler() {

}

void main() {
	Hardware hw;
	Display display;
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

	fw.Print();
#if !defined(NO_EMAC)
	nw.Print();
#endif

	LightSet *pBoard;

	StoreSparkFunDmx storeSparkFunDmx;
	StoreMotors storeMotors;

	struct TSparkFunStores sparkFunStores;
	sparkFunStores.pSparkFunDmxParamsStore = &storeSparkFunDmx;
	sparkFunStores.pModeParamsStore = &storeMotors;
	sparkFunStores.pMotorParamsStore = &storeMotors;
	sparkFunStores.pL6470ParamsStore = &storeMotors;

	auto *pSparkFunDmx = new SparkFunDmx;
	assert(pSparkFunDmx != nullptr);

	pSparkFunDmx->ReadConfigFiles(&sparkFunStores);
	pSparkFunDmx->SetModeStore(&storeMotors);

	pBoard = pSparkFunDmx;

	StoreTLC59711 storeTLC59711;
	TLC59711DmxParams pwmledparms(&storeTLC59711);

	bool isLedTypeSet = false;

	if (pwmledparms.Load()) {
		if ((isLedTypeSet = pwmledparms.IsSetLedType()) == true) {
			auto *pTLC59711Dmx = new TLC59711Dmx;
			assert(pTLC59711Dmx != nullptr);
			pTLC59711Dmx->SetTLC59711DmxStore(&storeTLC59711);
			pwmledparms.Dump();
			pwmledparms.Set(pTLC59711Dmx);

			display.Printf(7, "%s:%d", pwmledparms.GetType(pwmledparms.GetLedType()), pwmledparms.GetLedCount());

			auto *pChain = new LightSetChain;
			assert(pChain != nullptr);

			pChain->Add(pBoard, 0);
			pChain->Add(pTLC59711Dmx, 1);
			pChain->Dump();

			pBoard = pChain;
		}
	}

	char aDescription[64];
	snprintf(aDescription, sizeof(aDescription) - 1, "Sparkfun%s", isLedTypeSet ? " with TLC59711" : "");

	RDMPersonality *pRDMPersonalities[1] = { new  RDMPersonality(aDescription, pBoard)};

	RDMResponder rdmResponder(pRDMPersonalities, 1);

	StoreRDMSensors storeRdmSensors;
	RDMSensorsParams rdmSensorsParams(&storeRdmSensors);

	if (rdmSensorsParams.Load()) {
		rdmSensorsParams.Set();
		rdmSensorsParams.Dump();
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
		rdmDeviceParams.Set(&rdmResponder);
		rdmDeviceParams.Dump();
	}

	rdmResponder.Start();
	rdmResponder.Print();

	hw.SetMode(hardware::ledblink::Mode::NORMAL);
	hw.WatchdogInit();

	for(;;) {
		hw.WatchdogFeed();
		rdmResponder.Run();
		configStore.Flash();
		hw.Run();
	}
}
