/**
 * @file hardware.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef HARDWARE_H_
#define HARDWARE_H_

#ifdef __cplusplus

enum THardwareLedStatus {
	HARDWARE_LED_OFF = 0,
	HARDWARE_LED_ON,
	HARDWARE_LED_HEARTBEAT,
	HARDWARE_LED_FLASH
};

enum TBootDevice {
	BOOT_DEVICE_UNK,
	BOOT_DEVICE_FEL,	// H3 Only
	BOOT_DEVICE_MMC0,
	BOOT_DEVICE_SPI,	// H3 Only
	BOOT_DEVICE_HDD
};

struct THardwareTime {
	int tm_sec;		///< Seconds.		[0-60]	(1 leap second)
	int tm_min;		///< Minutes.		[0-59]
	int tm_hour;	///< Hours.			[0-23]
	int tm_mday;	///< Day.		 	[1-31]
	int tm_mon;		///< Month.			[0-11]
	int tm_year;	///< Year - 1900
	int tm_wday;	///< Day of week.	[0-6]
	int tm_yday;	///< Days in year.	[0-365]
	int tm_isdst;	///< DST.			[-1/0/1]
};

#if defined (__circle__)
 #include "circle/hardware.h"
#elif defined (BARE_METAL)
 #if defined (H3)
  #include "h3/hardware.h"
 #else
  #include "rpi/hardware.h"
 #endif
#else
 #include "linux/hardware.h"
#endif

#elif defined(BARE_METAL)
 #include "c/hardware.h"
#endif

#endif /* HARDWARE_H_ */
