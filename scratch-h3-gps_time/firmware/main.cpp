#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
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

#include <h3/shell.h>

#include "ntpclient.h"
/**
 *
 */
#include "gps.h"
#include "gpsparams.h"
#include "storegps.h"

extern "C" {

void notmain(void) {
	Hardware hw;
	NetworkH3emac nw;
	LedBlink lb;
	Display display(0,4);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	fw.Print();

	NetworkHandlerOled networkHandlerOled;

	nw.SetNetworkDisplay(&networkHandlerOled);
	nw.SetNetworkStore(StoreNetwork::Get());
	nw.Init(StoreNetwork::Get());
	nw.Print();

	networkHandlerOled.ShowIp();

	NtpClient ntpClient(nw.GetNtpServerIp());
	ntpClient.SetNtpClientDisplay(&networkHandlerOled);
	ntpClient.Start();
	ntpClient.Print();

	RemoteConfig remoteConfig(REMOTE_CONFIG_LTC, REMOTE_CONFIG_MODE_TIMECODE, 0);

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if (remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;


	Shell shell;

	/*
	 *
	 */

	GPSParams gpsParams(new StoreGPS);

	if (gpsParams.Load()) {
		gpsParams.Dump();
	}

	GPS gps(gpsParams.GetUtcOffset());
	gps.SetGPSDisplay(&networkHandlerOled);
	gps.Print();

	int nPrevSeconds = 60; // Force initial update

	display.ClearLine(0);
	display.ClearLine(1);

	for (;;) {
		nw.Run();
		remoteConfig.Run();
		spiFlashStore.Flash();
		lb.Run();
		shell.Run();
		ntpClient.Run();

		/*
		 *
		 */
		gps.Run();

		/*
		 *
		 */
		time_t ltime;
		struct tm *tm;
		ltime = time(nullptr);
		tm = localtime(&ltime);

		if (tm->tm_sec != nPrevSeconds) {
			nPrevSeconds = tm->tm_sec;

			display.Printf(1, "%.2d:%.2d:%.2d (NTP)", tm->tm_hour, tm->tm_min, tm->tm_sec);

			const struct tm *gps_time = gps.GetLocalDateTime();
			display.Printf(2, "%.2d:%.2d:%.2d (GPS)", gps_time->tm_hour, gps_time->tm_min, gps_time->tm_sec);
		}
	}
}

}
