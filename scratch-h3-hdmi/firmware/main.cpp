
#ifndef ORANGE_PI_ONE
# define ORANGE_PI_ONE
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
 * Debugging HDMI support Orange Pi One
 */
#ifdef ORANGE_PI_ONE
extern "C" {
extern int uart0_printf(const char* fmt, ...);
#define printf uart0_printf
}

#include "display_timing.h"
struct display_timing default_timing;

#endif

extern "C" {
#ifdef ORANGE_PI_ONE
void h3_de2_dump(void);
void h3_hdmi_phy_dump(void);
void h3_lcdc_dump(void);
void h3_de2_init(struct display_timing *timing, uint32_t fbbase);

int console_init(void);
#endif

void notmain(void) {
#ifdef ORANGE_PI_ONE
	memset(&default_timing, 0, sizeof(struct display_timing));

	default_timing.hdmi_monitor = false;
	default_timing.pixelclock.typ = 32000000;
	default_timing.hactive.typ = 800;
	default_timing.hback_porch.typ = 40;
	default_timing.hfront_porch.typ = 40;
	default_timing.hsync_len.typ = 48;
	default_timing.vactive.typ = 480;
	default_timing.vback_porch.typ = 29;
	default_timing.vfront_porch.typ = 13;
	default_timing.vsync_len.typ = 3;
	default_timing.flags = static_cast<display_flags>(DISPLAY_FLAGS_HSYNC_LOW | DISPLAY_FLAGS_VSYNC_LOW);

	h3_de2_init(&default_timing, 0x5E000000);

	console_init();

	const struct display_timing *edid = &default_timing;
	printf("pixelclock: %d\n", edid->pixelclock.typ);
	printf("h: %d %d %d %d\n", edid->hactive.typ, edid->hback_porch.typ, edid->hfront_porch, edid->hsync_len.typ);
	printf("v: %d %d %d %d\n", edid->vactive.typ, edid->vback_porch.typ, edid->vfront_porch, edid->vsync_len.typ);
	printf("flags: %d\n", edid->flags);

	h3_de2_dump();
#endif
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

	RemoteConfig remoteConfig(REMOTE_CONFIG_ARTNET, REMOTE_CONFIG_MODE_DMX, 0);

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if (remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	int nPrevSeconds = 60; // Force initial update

	display.ClearLine(0);
	display.ClearLine(1);

	for (;;) {
		nw.Run();
		remoteConfig.Run();
		spiFlashStore.Flash();
		lb.Run();
		shell.Run();

		time_t ltime;
		struct tm *tm;
		ltime = time(nullptr);
		tm = localtime(&ltime);

		if (tm->tm_sec != nPrevSeconds) {
			nPrevSeconds = tm->tm_sec;
			display.Printf(1, "%.2d:%.2d:%.2d", tm->tm_hour, tm->tm_min, tm->tm_sec);
		}
	}
}

}
