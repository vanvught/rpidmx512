/**
 * @file main.c
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "networkbaremetalmacaddress.h"
#include "ledblink.h"

#include "display.h"

#include "console.h"

#include "identify.h"

#include "dmxgpioparams.h"

#include "rdmresponder.h"
#include "rdmpersonality.h"

#include "rdmdeviceparams.h"
#include "rdmsensorsparams.h"

#include "tlc59711dmxparams.h"
#include "tlc59711dmx.h"

#include "lightsetchain.h"

#if defined (ORANGE_PI)
# include "spiflashinstall.h"
# include "spiflashstore.h"
# include "storerdmdevice.h"
# include "storetlc59711.h"
# include "storerdmdevice.h"
# include "storerdmsensors.h"
#endif

#include "firmwareversion.h"
#include "software_version.h"

#if defined (ORANGE_PI_ONE)
 #include "slushdmx.h"
 #define BOARD_NAME	"Slushengine"
#else
 #include "sparkfundmx.h"
 #define BOARD_NAME "Sparkfun"
 #include "storesparkfundmx.h"
 #include "storemotors.h"
#endif

extern "C" {

void notmain(void) {
	Hardware hw;
	NetworkBaremetalMacAddress nw;
	LedBlink lb;
	Display display(DisplayType::SSD1306);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

#if defined (ORANGE_PI)
	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;
#endif

	fw.Print();

	Identify identify;
	LightSet *pBoard;

	hw.SetLed(HARDWARE_LED_ON);

#if defined (ORANGE_PI_ONE)
	SlushDmx *pSlushDmx = new SlushDmx(false);	// Do not use SPI busy check
	assert(pSlushDmx != 0);
	pSlushDmx->ReadConfigFiles();
	pBoard = pSlushDmx;

#else
	StoreSparkFunDmx storeSparkFunDmx;
	StoreMotors storeMotors;

	struct TSparkFunStores sparkFunStores;
	sparkFunStores.pSparkFunDmxParamsStore = &storeSparkFunDmx;
	sparkFunStores.pModeParamsStore = &storeMotors;
	sparkFunStores.pMotorParamsStore = &storeMotors;
	sparkFunStores.pL6470ParamsStore = &storeMotors;

	SparkFunDmx *pSparkFunDmx = new SparkFunDmx;
	assert(pSparkFunDmx != 0);

	pSparkFunDmx->ReadConfigFiles(&sparkFunStores);
	pSparkFunDmx->SetModeStore(&storeMotors);

	pBoard = pSparkFunDmx;
#endif

#if defined (ORANGE_PI)
	StoreTLC59711 storeTLC59711;
	TLC59711DmxParams pwmledparms(&storeTLC59711);
#else
	TLC59711DmxParams pwmledparms;
#endif

	bool isLedTypeSet = false;

	if (pwmledparms.Load()) {
		if ((isLedTypeSet = pwmledparms.IsSetLedType()) == true) {
			TLC59711Dmx *pTLC59711Dmx = new TLC59711Dmx;
			assert(pTLC59711Dmx != 0);
#if defined (ORANGE_PI)
			pTLC59711Dmx->SetTLC59711DmxStore(&storeTLC59711);
#endif
			pwmledparms.Dump();
			pwmledparms.Set(pTLC59711Dmx);

			display.Printf(7, "%s:%d", pwmledparms.GetLedTypeString(pwmledparms.GetLedType()), pwmledparms.GetLedCount());

			LightSetChain *pChain = new LightSetChain;
			assert(pChain != 0);

			pChain->Add(pBoard, 0);
			pChain->Add(pTLC59711Dmx, 1);
			pChain->Dump();

			pBoard = pChain;
		}
	}

	char aDescription[64];
	snprintf(aDescription, sizeof(aDescription) - 1, "%s%s", BOARD_NAME, isLedTypeSet ? " with TLC59711" : "");

	DmxGpioParams dmxgpioparams;

	if (dmxgpioparams.Load()) {
		dmxgpioparams.Dump();
	}

	bool isSet;
	uint8_t nGpioDataDirection = dmxgpioparams.GetDataDirection(isSet); // Returning default GPIO18

#if defined (ORANGE_PI_ONE)
	if (!isSet) {
		nGpioDataDirection = GPIO_EXT_24;
	}
#endif

	RDMPersonality personality(aDescription, pBoard->GetDmxFootprint());
	RDMResponder dmxrdm(&personality, pBoard, nGpioDataDirection);

#if defined (ORANGE_PI)
	StoreRDMDevice storeRdmDevice;
	RDMDeviceParams rdmDeviceParams(&storeRdmDevice);
	dmxrdm.SetRDMDeviceStore(&storeRdmDevice);

	StoreRDMSensors storeRdmSensors;
	RDMSensorsParams rdmSensorsParams(&storeRdmSensors);
#else
	RDMDeviceParams rdmDeviceParams;
	RDMSensorsParams rdmSensorsParams;
#endif

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

	hw.WatchdogInit();

	dmxrdm.SetOutput(pBoard);
	dmxrdm.Start();

	for(;;) {
		hw.WatchdogFeed();
		dmxrdm.Run();
		identify.Run();
#if defined (ORANGE_PI)
		spiFlashStore.Flash();
#endif
		lb.Run();
	}
}

}
