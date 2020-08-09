#ifndef ORANGE_PI_ONE
// #define ORANGE_PI_ONE
#endif

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

/*
 *
 */
#include <h3/shell.h>
/*
 *
 */
#include "ptpclient.h"
/*
 * Debugging System Clock framework for PTP implementation
 */
#include <arpa/inet.h>
#include <netinet/in.h>
#include "ntpclient.h"
/*
 *
 */
#include "hwclock.h"
#include "../lib-hal/rtc/rtc.h"


/*
 * Debugging HDMI support Orange Pi One
 */
#ifdef ORANGE_PI_ONE
#include "h3.h"
extern "C" {
#include "../lib/display.h"
extern int uart0_printf(const char* fmt, ...);
#define printf uart0_printf
extern void debug_dump(const void *packet, uint16_t len);
}
#endif

extern "C" {

void notmain(void) {
#ifdef ORANGE_PI_ONE
//	display_init();
#endif
	Hardware hw;
	HwClock hwClock;
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

	/*
	 * Debugging System Clock framework for PTP implementation
	 */

	NtpClient ntpClient(nw.GetNtpServerIp());
	ntpClient.SetNtpClientDisplay(&networkHandlerOled);
	ntpClient.Start();
	ntpClient.Print();

	if (ntpClient.GetStatus() != NtpClientStatus::STOPPED) {
		printf("Set RTC from System Clock");
		hwClock.SysToHc();
	} else {
		printf("Set System Clock from RTC");
		hwClock.HcToSys();
	}

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
	Shell shell;

	/*
	 * Debugging HDMI support Orange Pi One
	 */
#if 0
	printf("LCD0\n");
	printf("GCTL         %p\n", H3_LCD0->GCTL);
	printf("GINT0        %p\n", H3_LCD0->GINT0);
	printf("GINT1        %p\n", H3_LCD0->GINT1);
	printf("TCON1_CTL    %p\n", H3_LCD0->TCON1_CTL);
	printf("TCON1_BASIC0 %p\n", H3_LCD0->TCON1_BASIC0);
	printf("TCON1_BASIC1 %p\n", H3_LCD0->TCON1_BASIC1);
	printf("TCON1_BASIC2 %p\n", H3_LCD0->TCON1_BASIC2);
	printf("TCON1_BASIC3 %p\n", H3_LCD0->TCON1_BASIC3);
	printf("TCON1_BASIC4 %p\n", H3_LCD0->TCON1_BASIC4);
	printf("TCON1_BASIC5 %p\n", H3_LCD0->TCON1_BASIC5);
#endif

	int nPrevSeconds = 60; // Force initial update
	display.ClearLine(0);
	display.ClearLine(1);

	for (;;) {
		nw.Run();
		remoteConfig.Run();
		spiFlashStore.Flash();
		lb.Run();

		/*
		 *
		 */
		shell.Run();

		/*
		 * Debugging System Clock framework for PTP implementation
		 */
		// ntpClient.Run();

		/*
		 *
		 */

		time_t ltime;
		struct tm *tm;
		ltime = time(nullptr);
		tm = localtime(&ltime);

		if (tm->tm_sec != nPrevSeconds) {
			nPrevSeconds = tm->tm_sec;
			struct tm rtc;
			rtc_get_date_time(&rtc);

			display.Printf(1, "%.2d:%.2d:%.2d", tm->tm_hour, tm->tm_min, tm->tm_sec);
			display.Printf(2, "%.2d:%.2d:%.2d", rtc.tm_hour, rtc.tm_min, rtc.tm_sec);
		}
	}
}

}
