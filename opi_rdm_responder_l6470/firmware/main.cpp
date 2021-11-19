/**
 * @file main.c
 *
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "hardware.h"
#include "noemac/network.h"
#include "ledblink.h"

#include "display.h"

#include "console.h"

#include "identify.h"

#include "rdmresponder.h"
#include "rdmpersonality.h"

#include "rdmdeviceparams.h"
#include "rdmsensorsparams.h"

#include "tlc59711dmxparams.h"
#include "tlc59711dmx.h"

#include "lightsetchain.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"
#include "storerdmdevice.h"
#include "storetlc59711.h"
#include "storerdmdevice.h"
#include "storerdmsensors.h"

#include "firmwareversion.h"
#include "software_version.h"

#include "sparkfundmx.h"
#include "storesparkfundmx.h"
#include "storemotors.h"

extern "C" {

void notmain(void) {
	Hardware hw;
	Network nw;
	LedBlink lb;
	Display display(DisplayType::SSD1306);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	fw.Print();

	Identify identify;
	LightSet *pBoard;

	hw.SetLed(hardware::LedStatus::ON);

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

	RDMPersonality personality(aDescription, pBoard->GetDmxFootprint());
	RDMResponder dmxrdm(&personality, pBoard);

	StoreRDMDevice storeRdmDevice;
	RDMDeviceParams rdmDeviceParams(&storeRdmDevice);
	dmxrdm.SetRDMDeviceStore(&storeRdmDevice);

	StoreRDMSensors storeRdmSensors;
	RDMSensorsParams rdmSensorsParams(&storeRdmSensors);

	if (rdmDeviceParams.Load()) {
		rdmDeviceParams.Set(&dmxrdm);
		rdmDeviceParams.Dump();
	}

	if (rdmSensorsParams.Load()) {
		rdmSensorsParams.Set();
		rdmSensorsParams.Dump();
	}

	dmxrdm.Init();
	dmxrdm.Print();
	dmxrdm.Start();

	hw.WatchdogInit();

	for(;;) {
		hw.WatchdogFeed();
		dmxrdm.Run();
		identify.Run();
		spiFlashStore.Flash();
		lb.Run();
	}
}

}
