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
	NetworkH3emac nw;
	LedBlink lb;
	Display oled;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	fw.Print();

	nw.Init(StoreNetwork::Get());
	nw.SetNetworkStore(StoreNetwork::Get());
	nw.Print();

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

	/*
	 * Debugging System Clock framework for PTP implementation
	 */

	struct timeval tv;
	gettimeofday(&tv, 0);
	printf("get %d %d\n", tv.tv_sec, tv.tv_usec);

	in_addr ip;
	inet_aton("192.168.2.110", &ip);
	NtpClient ntpClient(ip.s_addr);
	ntpClient.Start();
	ntpClient.Print();

	gettimeofday(&tv, 0);
	printf("get %d %d\n", tv.tv_sec, tv.tv_usec);

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
		ntpClient.Run();
	}
}

}
