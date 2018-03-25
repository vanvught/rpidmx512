/**
 * @file main.c
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "hardwarebaremetal.h"
#include "networkbaremetal.h"
#include "ledblinkbaremetal.h"

#include "console.h"

#include "identify.h"

#include "dmxgpioparams.h"

#include "rdmresponder.h"
#include "rdmpersonality.h"

#include "slushdmx.h"
#include "sparkfundmx.h"

#include "pwmdmx.h"

#include "lightsetchain.h"

#include "software_version.h"

enum TBoards {
	BOARD_SLUSH = 0,
	BOARD_SPARKFUN
};

extern "C" {

void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {}
void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {}

void notmain(void) {
	HardwareBaremetal hw;
	NetworkBaremetal nw;
	LedBlinkBaremetal lb;
	Identify identify;
	uint8_t nHwTextLength;
	LightSet *pBoard;
	TBoards board;
	FILE *pFile;

	if ((pFile = fopen("slush.txt", "r")) != NULL) {
		fclose(pFile);
		board = BOARD_SLUSH;
	} else if ((pFile = fopen("sparkfun.txt", "r")) != NULL) {
		fclose(pFile);
		board = BOARD_SPARKFUN;
	} else {
		console_error("File \'slush.txt\' or \'sparkfun.txt\' not found");
		for(;;);
	}

	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetBoardName(nHwTextLength), __DATE__, __TIME__);

	hw.SetLed(HARDWARE_LED_ON);

	if (board == BOARD_SLUSH) {
		SlushDmx *pSlushDmx = new SlushDmx(false);	// Do not use SPI busy check
		assert(pSlushDmx != 0);
		pSlushDmx->ReadConfigFiles();
		pBoard = pSlushDmx;
	} else {
		SparkFunDmx *pSparkFunDmx = new SparkFunDmx;
		assert(pSparkFunDmx != 0);
		pSparkFunDmx->ReadConfigFiles();
		pBoard = pSparkFunDmx;
	}

	PwmDmx pwmdmx;
	pwmdmx.Dump();

	bool IsChipSet;
	const bool IsChipTLC59711 = (pwmdmx.GetPwmDmxChip(IsChipSet) == PWMDMX_CHIP_TLC59711);
	const bool IsPwmLedEnabled = (pwmdmx.GetLightSet() != 0);

	char aDescription[64];
	snprintf(aDescription, sizeof(aDescription) - 1, "%s %s",
			board == BOARD_SLUSH ? "Slushengine" : "Sparkfun",
			IsPwmLedEnabled ? (IsChipTLC59711 ? "with PWM Led TLC59711" : "with PWM Led PCA9685") : "");

	if(IsPwmLedEnabled) {
		LightSetChain *pChain = new LightSetChain;
		assert(pChain != 0);
		pChain->Add(pBoard, 0);
		pChain->Add(pwmdmx.GetLightSet(), 1);
		pChain->Dump();
		pBoard = pChain;
	}

	DmxGpioParams dmxgpioparams;
	dmxgpioparams.Dump();

	bool isSet;
	uint8_t nGpioDataDirection = dmxgpioparams.GetDataDirection(isSet); // Returning default GPIO18

	if ((!isSet) && (board == BOARD_SLUSH)) {
		nGpioDataDirection = 8;  // RPI_V2_GPIO_P1_24	8 SPI0 CE0
	}

	RDMPersonality personality(aDescription, pBoard->GetDmxFootprint());
	RDMResponder dmxrdm(&personality, pBoard, nGpioDataDirection, false);

	dmxrdm.GetRDMDeviceResponder()->Print();

	console_set_top_row(12);

	hw.WatchdogInit();

	dmxrdm.SetOutput(pBoard);
	dmxrdm.Start();

	for(;;) {
		hw.WatchdogFeed();
		(void) dmxrdm.Run();
		lb.Run();
	}
}

}
