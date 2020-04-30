/**
 * @file hardware.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
	HARDWARE_LED_OFF,
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

#if defined (BARE_METAL)
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
