#ifndef ORANGE_PI_ONE
 #define ORANGE_PI_ONE
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
 * Debugging HDMI support Orange Pi One
 */
#ifdef ORANGE_PI_ONE
#include "h3.h"
#include "device/fb.h"

#define NK_ASSERT
#define NK_INCLUDE_FIXED_TYPES
//#define NK_INCLUDE_STANDARD_IO
//#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_SOFTWARE_FONT
#define NK_RAWFB_IMPLEMENTATION
#define NK_IMPLEMENTATION

#include "../include/nuklear.h"
#include "../include/nuklear_rawfb.h"


extern "C" {
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
	Display display(0,4);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

  	/* GUI */
    struct rawfb_context *rawfb;
    void *fb = NULL;
    rawfb_pl pl;
    unsigned char tex_scratch[512 * 512];
    rawfb = nk_rawfb_init(fb, tex_scratch, FB_WIDTH, FB_HEIGHT, FB_PITCH, pl);
    if (!rawfb) 
		uart0_printf("Failed nk_rawfb_init()!\n");

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


	for (;;) {
		nw.Run();
		remoteConfig.Run();
		spiFlashStore.Flash();
		lb.Run();

		
        /* GUI */
        if (nk_begin(&rawfb->ctx, "Demo", nk_rect(50, 50, 200, 200),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|
            NK_WINDOW_CLOSABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
            enum {EASY, HARD};
            static int op = EASY;
            static int property = 20;

            nk_layout_row_static(&rawfb->ctx, 30, 80, 1);
            if (nk_button_label(&rawfb->ctx, "button"))
                uart0_printf("button pressed\n");
            nk_layout_row_dynamic(&rawfb->ctx, 30, 2);
            if (nk_option_label(&rawfb->ctx, "easy", op == EASY)) op = EASY;
            if (nk_option_label(&rawfb->ctx, "hard", op == HARD)) op = HARD;
            nk_layout_row_dynamic(&rawfb->ctx, 25, 1);
            nk_property_int(&rawfb->ctx, "Compression:", 0, &property, 100, 10, 1);
        }
        nk_end(&rawfb->ctx);
        if (nk_window_is_closed(&rawfb->ctx, "Demo")) break;

        /* -------------- EXAMPLES ---------------- */
        #ifdef INCLUDE_CALCULATOR
          calculator(&rawfb->ctx);
        #endif
        #ifdef INCLUDE_OVERVIEW
          overview(&rawfb->ctx);
        #endif
        #ifdef INCLUDE_NODE_EDITOR
          node_editor(&rawfb->ctx);
        #endif
        /* ----------------------------------------- */

        /* Draw framebuffer */
        nk_rawfb_render(rawfb, nk_rgb(30,30,30), 1);

	}
}

}
