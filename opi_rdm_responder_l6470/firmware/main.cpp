/**
 * @file main.c
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "slushdmx.h"
#include "sparkfundmx.h"

#include "tlc59711dmxparams.h"
#include "tlc59711dmx.h"

#include "lightsetchain.h"

#include "software_version.h"

#if defined (ORANGE_PI_ONE)
 #define BOARD_NAME	"Slushengine"
#else
 #define BOARD_NAME "Sparkfun"
#endif

extern "C" {

void notmain(void) {
	Hardware hw;
	NetworkBaremetalMacAddress nw;
	LedBlink lb;
	Display display(DISPLAY_SSD1306);

	Identify identify;
	LightSet *pBoard;

	uint8_t nHwTextLength;
	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetBoardName(nHwTextLength), __DATE__, __TIME__);

	hw.SetLed(HARDWARE_LED_ON);

#if defined (ORANGE_PI_ONE)
	SlushDmx *pSlushDmx = new SlushDmx(false);	// Do not use SPI busy check
	assert(pSlushDmx != 0);
	pSlushDmx->ReadConfigFiles();
	pBoard = pSlushDmx;

#else
	SparkFunDmx *pSparkFunDmx = new SparkFunDmx;
	assert(pSparkFunDmx != 0);
	pSparkFunDmx->ReadConfigFiles();
	pBoard = pSparkFunDmx;
#endif

	TLC59711DmxParams pwmledparms;

	bool isLedTypeSet = false;

	if (pwmledparms.Load()) {
		if ((isLedTypeSet = pwmledparms.IsSetLedType()) == true) {
			TLC59711Dmx *pTLC59711Dmx = new TLC59711Dmx;
			assert(pTLC59711Dmx != 0);
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
		nGpioDataDirection = GPIO_EXT_24;  // RPI_V2_GPIO_P1_24	8 SPI0 CE0
	}
#endif

	RDMPersonality personality(aDescription, pBoard->GetDmxFootprint());
	RDMResponder dmxrdm(&personality, pBoard, nGpioDataDirection, false);

	RDMDeviceParams rdmDeviceParams;
	if (rdmDeviceParams.Load()) {
		rdmDeviceParams.Set((RDMDevice *)&dmxrdm);
		rdmDeviceParams.Dump();
	}

	dmxrdm.Init();
	dmxrdm.Print();

	hw.WatchdogInit();

	dmxrdm.SetOutput(pBoard);
	dmxrdm.Start();

	for(;;) {
		hw.WatchdogFeed();
		dmxrdm.Run();
		lb.Run();
	}
}

}
