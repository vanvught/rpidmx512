/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "hardwarelinux.h"
#include "networklinux.h"
#include "ledblinklinux.h"

#include "artnetnode.h"
#include "artnetparams.h"

#include "slushdmx.h"
#include "sparkfundmx.h"

#include "ipprog.h"

#include "software_version.h"

#include "rdmdeviceresponder.h"
#include "rdmpersonality.h"

#include "identify.h"
#include "artnetrdmresponder.h"

#include "pwmdmx.h"

#include "lightsetchain.h"

enum TBoards {
	BOARD_SLUSH = 0,
	BOARD_SPARKFUN
};

static const char sLongName[] = "Raspberry Pi Art-Net 3 L6470 Stepper Motor Controller";

int main(int argc, char **argv) {
	HardwareLinux hw;
	NetworkLinux nw;
	LedBlinkLinux lbt;
	ArtNetParams artnetparams;
	ArtNetNode node;
	LightSet *pBoard;
	IpProg ipprog;
	uint8_t nTextLength;
	FILE *pFile;
	TBoards board;

	if (argc < 2) {
		printf("Usage: %s ip_address|interface_name\n", argv[0]);
		return -1;
	}

	if ((pFile = fopen("slush.txt", "r")) != NULL) {
		fclose(pFile);
		board = BOARD_SLUSH;
	} else if ((pFile = fopen("sparkfun.txt", "r")) != NULL) {
		fclose(pFile);
		board = BOARD_SPARKFUN;
	} else {
		fprintf(stderr, "File \'slush.txt\' or \'sparkfun.txt\' not found\n");
		return -1;
	}

	printf("[V%s] %s {%s} Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetSysName(nTextLength), hw.GetVersion(nTextLength), __DATE__, __TIME__);

	if (getuid() != 0) {
		fprintf(stderr, "Application is not started as \'root\' (sudo)\n");
		return -1;
	}

	if (nw.Init(argv[1]) < 0) {
		fprintf(stderr, "Not able to start the network\n");
		return -1;
	}

	hw.SetLed(HARDWARE_LED_ON);

	(void) artnetparams.Load();
	artnetparams.Dump();
	artnetparams.Set(&node);

	node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, artnetparams.GetUniverse());

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
	printf("%s : %s\n", sLongName, aDescription);

	if(IsPwmLedEnabled) {
		LightSetChain *pChain = new LightSetChain;
		assert(pChain != 0);
		pChain->Add(pBoard, 0);
		pChain->Add(pwmdmx.GetLightSet(), 1);
		pChain->Dump();
		pBoard = pChain;
	}

	node.SetOutput(pBoard);

	if (getuid() == 0) {
		node.SetIpProgHandler(&ipprog);
	}

	char *params_long_name = (char *)artnetparams.GetLongName();
	if (*params_long_name == 0) {
		node.SetLongName(sLongName);
	}

	RDMPersonality personality(aDescription, pBoard->GetDmxFootprint());
	ArtNetRdmResponder RdmResponder(&personality, pBoard);

	node.SetRdmHandler(&RdmResponder, true);

	Identify identify;

	nw.Print();
	node.Print();
	RdmResponder.GetRDMDeviceResponder()->Print();

	node.Start();

	for (;;) {
		(void) node.HandlePacket();
		identify.Run();
	}

	return 0;
}
