/*
 * display_timing.h
 */

#ifndef INCLUDE_DISPLAY_TIMING_H_
#define INCLUDE_DISPLAY_TIMING_H_

#include <stdbool.h>

/* Display timings from linux include/video/display_timing.h */
enum display_flags {
	DISPLAY_FLAGS_HSYNC_LOW = 1 << 0,
	DISPLAY_FLAGS_HSYNC_HIGH = 1 << 1,
	DISPLAY_FLAGS_VSYNC_LOW = 1 << 2,
	DISPLAY_FLAGS_VSYNC_HIGH = 1 << 3,

	/* data enable flag */
	DISPLAY_FLAGS_DE_LOW = 1 << 4,
	DISPLAY_FLAGS_DE_HIGH = 1 << 5,
	/* drive data on pos. edge */
	DISPLAY_FLAGS_PIXDATA_POSEDGE = 1 << 6,
	/* drive data on neg. edge */
	DISPLAY_FLAGS_PIXDATA_NEGEDGE = 1 << 7,
	DISPLAY_FLAGS_INTERLACED = 1 << 8,
	DISPLAY_FLAGS_DOUBLESCAN = 1 << 9,
	DISPLAY_FLAGS_DOUBLECLK = 1 << 10,
};

/*
 * A single signal can be specified via a range of minimal and maximal values
 * with a typical value, that lies somewhere inbetween.
 */
struct timing_entry {
	uint32_t min;
	uint32_t typ;
	uint32_t max;
};

/*
 * Single "mode" entry. This describes one set of signal timings a display can
 * have in one setting. This struct can later be converted to struct videomode
 * (see include/video/videomode.h). As each timing_entry can be defined as a
 * range, one struct display_timing may become multiple struct videomodes.
 *
 * Example: hsync active high, vsync active low
 *
 *				    Active Video
 * Video  ______________________XXXXXXXXXXXXXXXXXXXXXX_____________________
 *	  |<- sync ->|<- back ->|<----- active ----->|<- front ->|<- sync..
 *	  |	     |	 porch  |		     |	 porch	 |
 *
 * HSync _|¯¯¯¯¯¯¯¯¯¯|___________________________________________|¯¯¯¯¯¯¯¯¯
 *
 * VSync ¯|__________|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|_________
 */

struct display_timing {
	struct timing_entry pixelclock;

	struct timing_entry hactive; 		/* hor. active video */
	struct timing_entry hfront_porch; 	/* hor. front porch */
	struct timing_entry hback_porch; 	/* hor. back porch */
	struct timing_entry hsync_len; 		/* hor. sync len */

	struct timing_entry vactive; 		/* ver. active video */
	struct timing_entry vfront_porch; 	/* ver. front porch */
	struct timing_entry vback_porch; 	/* ver. back porch */
	struct timing_entry vsync_len; 		/* ver. sync len */

	enum display_flags flags; 			/* display flags */
	bool hdmi_monitor; 					/* is hdmi monitor? */
};

#endif /* _INCLUDE_DISPLAY_TIMING_H_ */
