#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "debug.h"

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

#include "jamstapl.h"
#include "jamstaplutil.h"

#include "debug.h"

static const char SOFTWARE_VERSION[] = "0.0";

extern uint32_t PIXEL8X4_PROGRAM;

extern "C" {

extern uint32_t getPIXEL8X4_SIZE();

void notmain(void) {
	Hardware hw;
	NetworkH3emac nw;
	LedBlink lb;
	Display display(0,4);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	fw.Print();
	display.PrintInfo();

	NetworkHandlerOled networkHandlerOled;

	nw.SetNetworkDisplay(&networkHandlerOled);
	nw.SetNetworkStore(StoreNetwork::Get());
	nw.Init(StoreNetwork::Get());
	nw.Print();

	networkHandlerOled.ShowIp();

	RemoteConfig remoteConfig(remoteconfig::Node::RDMNET_LLRP_ONLY, remoteconfig::Output::CONFIG, 0);

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if (remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	JamSTAPL jbc(reinterpret_cast<uint8_t*>(&PIXEL8X4_PROGRAM), getPIXEL8X4_SIZE(), true);
	jbc.SetJamSTAPLDisplay(&networkHandlerOled);
	jbc.CheckCRC(true);
	jbc.PrintInfo();
	jbc.CheckIdCode();

	if (jbc.GetExitCode() == 0) {
		jbc.ReadUsercode();
		if (jbc.GetExitCode() == 0) {
			display.Printf(2, "%s: %X", jbc.GetExportIntegerKey(), jbc.GetExportIntegerInt());
			jbc.Program();
		}
	}

	for (;;) {
		nw.Run();
		remoteConfig.Run();
		spiFlashStore.Flash();
		lb.Run();
	}
}

}
