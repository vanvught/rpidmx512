#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "hardware.h"
#include "networkh3emac.h"
#include "ledblink.h"

#include "display.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"
#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "storenetwork.h"
#include "storeremoteconfig.h"

#include "networkhandleroled.h"

#include "firmwareversion.h"

static const char SOFTWARE_VERSION[] = "0.0";

#include "h3/shell.h"

#include "rgbpanel.h"

#include "e131bridge.h"
#include "reboot.h"

#define COLUMNS 64
#define ROWS 	32

extern "C" {

void notmain(void) {
	Hardware hw;
	NetworkH3emac nw;
	LedBlink lb;
	Display display(0,4);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	Shell shell;

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	fw.Print();

	NetworkHandlerOled networkHandlerOled;

	nw.SetNetworkDisplay(&networkHandlerOled);
	nw.SetNetworkStore(StoreNetwork::Get());
	nw.Init(StoreNetwork::Get());
	nw.Print();

	networkHandlerOled.ShowIp();

	RemoteConfig remoteConfig(REMOTE_CONFIG_RDMNET_LLRP_ONLY, REMOTE_CONFIG_MODE_CONFIG, 0);

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if (remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	/*
	 *
	 */

	RgbPanel matrix(COLUMNS,ROWS);
	matrix.Start();
	matrix.Print();

	uint32_t n = 0;

	for (uint32_t nRow = 0; nRow < ROWS; nRow++) {
		for (uint32_t nColomn = 0; nColomn < COLUMNS; nColomn++) {
			switch (n) {
				case 0:
					matrix.SetPixel(nColomn, nRow, 0xFF, 0, 0);
					break;
				case 1:
					matrix.SetPixel(nColomn, nRow, 0, 0xFF, 0);
					break;
				case 2:
					matrix.SetPixel(nColomn, nRow, 0, 0, 0xFF);
					break;
				default:
					break;
			}
		}

		n++;

		if (n == 3) {
			n = 0;
		}
	}

	matrix.Cls();
//	matrix.PutString("Hello", 0xF0, 0xFF, 0xF0);
//	matrix.ClearLine(2);
	matrix.SetColon(':', 1, 0, 0xF0, 0x0, 0x0);
	matrix.SetColon(':', 3, 0, 0xF0, 0x0, 0x0);
	matrix.SetColon('.', 5, 0, 0xF0, 0x0, 0x0);
	matrix.PutString("00000000", 0x70, 0x70, 0x70);
	matrix.TextLine(3, "SMPTE", 6, 0x0F, 0x0F, 0x0);
	matrix.TextLine(4, "Layer A", 7, 0x0F, 0x0F, 0x0);
//	matrix.TextLine(3, "World!", 6, 0, 0x0F, 0xF0);
//	matrix.SetCursorPos(7, 3);
//	matrix.PutChar(':', 0xF0, 0xF0, 0xF0);
	matrix.Show();

//	matrix.Dump();

	E131Bridge bridge;

	for (uint32_t i = 0; i < matrix.GetUniverses(); i++) {
		bridge.SetUniverse(i, E131_OUTPUT_PORT, i + 1);
	}

	bridge.SetOutput(&matrix);
	bridge.SetDirectUpdate(true);
	bridge.Print();

	Reboot reboot;
	hw.SetRebootHandler(&reboot);

	display.ClearLine(0);

	int nPrevSeconds = 60; // Force initial update
	uint32_t nUpdatesPrevious = 0;
	uint32_t nFpsPrevious = 0;
	uint32_t nShowCounterPrevious = 0;

	for (;;) {
		nw.Run();
		remoteConfig.Run();
		spiFlashStore.Flash();
		//lb.Run();
		shell.Run();
		bridge.Run();

		/*
		 * Debugging
		 */

		const auto ltime = time(nullptr);
		const auto *tm = localtime(&ltime);

		if (tm->tm_sec != nPrevSeconds) {
			nPrevSeconds = tm->tm_sec;

			const auto nUpdates = matrix.GetUpdatesCounter();
			const auto nFps = nUpdates - nUpdatesPrevious;
			nUpdatesPrevious = nUpdates;

			const auto nShowCounter = matrix.GetShowCounter();
			const auto nShowUpdates = nShowCounter - nShowCounterPrevious;
			nShowCounterPrevious = nShowCounter;

			if ((nFpsPrevious != nFps) || (nShowCounterPrevious != nShowCounter)) {
				nFpsPrevious = nFps;
				display.Printf(1, "%6u %6u", nFps, nShowUpdates);
			}
		}
	}
}

}
