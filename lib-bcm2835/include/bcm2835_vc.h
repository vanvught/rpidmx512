/**
 * @file bcm2835_vc.h
 *
 */
/* Copyright (C) 2014-2018, 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef BCM2835_VC_H_
#define BCM2835_VC_H_

#include "stdint.h"

/// Unique clock ID
typedef enum {
	BCM2835_VC_CLOCK_ID_RESERVED = 0,	///<
	BCM2835_VC_CLOCK_ID_EMMC = 1,		///<
	BCM2835_VC_CLOCK_ID_UART = 2,		///<
	BCM2835_VC_CLOCK_ID_ARM = 3,		///<
	BCM2835_VC_CLOCK_ID_CORE = 4,		///<
	BCM2835_VC_CLOCK_ID_V3D = 5,		///<
	BCM2835_VC_CLOCK_ID_H264 = 6,		///<
	BCM2835_VC_CLOCK_ID_ISP = 7,		///<
	BCM2835_VC_CLOCK_ID_SDRAM = 8,		///<
	BCM2835_VC_CLOCK_ID_PIXEL = 9,		///<
	BCM2835_VC_CLOCK_ID_PWM = 10		///<
} bcm2835VideoCoreClockId;

/// Unique power ID
typedef enum {
	BCM2835_VC_POWER_ID_SDCARD = 0,	///<
	BCM2835_VC_POWER_ID_UART0 = 1,	///<
	BCM2835_VC_POWER_ID_UART1 = 2,	///<
	BCM2835_VC_POWER_ID_USBHCD = 3,	///<
	BCM2835_VC_POWER_ID_I2C0 = 4,	///<
	BCM2835_VC_POWER_ID_I2C1 = 5,	///<
	BCM2835_VC_POWER_ID_I2C2 = 6,	///<
	BCM2835_VC_POWER_ID_SPI = 7,	///<
	BCM2835_VC_POWER_ID_CCP2TX = 8 	///<
} bcm2835VideoCorePowerId;

/**
 *  Bit 0: 0=off, 1=on
 *  Bit 1: 0=do not wait, 1=wait
 *  Bits 2-31: reserved for future use (set to 0)
 */
typedef enum {
	BCM2835_VC_SET_POWER_STATE_OFF_NO_WAIT = 0,	///<
	BCM2835_VC_SET_POWER_STATE_OFF_WAIT = 2,	///<
	BCM2835_VC_SET_POWER_STATE_ON_NO_WAIT = 1,	///<
	BCM2835_VC_SET_POWER_STATE_ON_WAIT = 3		///<
} bcm2835SetPowerState;

#define BCM2835_VC_SET_POWER_STATE_REQ_OFF		(0 << 0)	///< Bit 0: 0=off
#define BCM2835_VC_SET_POWER_STATE_REQ_ON		(1 << 0)	///< Bit 0: 1=on
#define BCM2835_VC_SET_POWER_STATE_REQ_WAIT		(1 << 1)	///< Bit 1: 1=wait

#define BCM2835_VC_POWER_STATE_RESP_ON			(1 << 0)	///< Bit 0: 0=off, 1=on
#define BCM2835_VC_POWER_STATE_RESP_NODEV		(1 << 1)	///< Bit 1: 0=device exists, 1=device does not exist

// Tag VideoCore
#define BCM2835_VC_TAG_GET_FIRMWARE_REV			0x00000001	///<
// Tag Hardware
#define BCM2835_VC_TAG_GET_BOARD_MODEL			0x00010001	///<
#define BCM2835_VC_TAG_GET_BOARD_REV			0x00010002	///<
#define BCM2835_VC_TAG_GET_BOARD_MAC_ADDRESS	0x00010003	///<
#define BCM2835_VC_TAG_GET_BOARD_SERIAL			0x00010004	///<
#define BCM2835_VC_TAG_GET_ARM_MEMORY			0x00010005	///<
#define BCM2835_VC_TAG_GET_VC_MEMORY			0x00010006	///<
#define BCM2835_VC_TAG_GET_CLOCKS				0x00010007	///<
// Tag Clock
#define BCM2835_VC_TAG_GET_CLOCK_STATE			0x00030001	///<
#define BCM2835_VC_TAG_GET_CLOCK_RATE 			0x00030002	///<
#define BCM2835_VC_TAG_GET_MAX_CLOCK_RATE 		0x00030004	///<
#define BCM2835_VC_TAG_GET_MIN_CLOCK_RATE 		0x00030007	///<
#define BCM2835_VC_TAG_GET_TURBO		 		0x00030009	///<
#define BCM2835_VC_TAG_SET_CLOCK_STATE			0x00038001	///<
#define BCM2835_VC_TAG_SET_CLOCK_RATE 			0x00038002	///<
#define BCM2835_VC_TAG_SET_TURBO	 			0x00038009	///<
// Tag Power
#define BCM2835_VC_TAG_GET_POWER_STATE			0x00020001	///<
#define BCM2835_VC_TAG_SET_POWER_STATE			0x00028001	///<
// Tag Temperatures
#define BCM2835_VC_TAG_GET_TEMP					0x00030006	///<
#define BCM2835_VC_TAG_GET_MAX_TEMP				0x0003000A	///<
// Tag Framebuffer
#define BCM2835_VC_TAG_ALLOCATE_BUFFER			0x00040001	///<
#define BCM2835_VC_TAG_RELEASE_BUFFER			0x00048001	///<
#define BCM2835_VC_TAG_BLANK_SCREEN				0x00040002	///<
#define BCM2835_VC_TAG_GET_PHYS_WH				0x00040003	///<
#define BCM2835_VC_TAG_TEST_PHYS_WH				0x00044003	///<
#define BCM2835_VC_TAG_SET_PHYS_WH				0x00048003	///<
#define BCM2835_VC_TAG_GET_VIRT_WH				0x00040004	///<
#define BCM2835_VC_TAG_TEST_VIRT_WH				0x00044004	///<
#define BCM2835_VC_TAG_SET_VIRT_WH				0x00048004	///<
#define BCM2835_VC_TAG_GET_DEPTH				0x00040005	///<
#define BCM2835_VC_TAG_TEST_DEPTH				0x00044005	///<
#define BCM2835_VC_TAG_SET_DEPTH				0x00048005	///<
#define BCM2835_VC_TAG_GET_PIXEL_ORDER			0x00040006	///<
#define BCM2835_VC_TAG_TEST_PIXEL_ORDER			0x00044006	///<
#define BCM2835_VC_TAG_SET_PIXEL_ORDER			0x00048006	///<
#define BCM2835_VC_TAG_GET_ALPHA_MODE			0x00040007	///<
#define BCM2835_VC_TAG_TEST_ALPHA_MODE			0x00044007	///<
#define BCM2835_VC_TAG_SET_ALPHA_MODE			0x00048007	///<
#define BCM2835_VC_TAG_GET_PITCH				0x00040008	///<
#define BCM2835_VC_TAG_GET_VIRT_OFFSET			0x00040009	///<
#define BCM2835_VC_TAG_TEST_VIRT_OFFSET			0x00044009	///<
#define BCM2835_VC_TAG_SET_VIRT_OFFSET			0x00048009	///<
#define BCM2835_VC_TAG_GET_OVERSCAN				0x0004000a	///<
#define BCM2835_VC_TAG_TEST_OVERSCAN			0x0004400a	///<
#define BCM2835_VC_TAG_SET_OVERSCAN				0x0004800a	///<
#define BCM2835_VC_TAG_GET_PALETTE				0x0004000b	///<
#define BCM2835_VC_TAG_TEST_PALETTE				0x0004400b	///<
#define BCM2835_VC_TAG_SET_PALETTE				0x0004800b	///<


#ifdef __cplusplus
extern "C" {
#endif

extern int32_t bcm2835_vc_get_clock_rate(uint32_t);
extern int32_t bcm2835_vc_set_clock_rate(uint32_t, uint32_t);

extern int32_t bcm2835_vc_get_power_state(uint32_t);
extern int32_t bcm2835_vc_set_power_state(uint32_t, uint32_t);

extern int32_t bcm2835_vc_get_temperature(void);
extern int32_t bcm2835_vc_get_temperature_max(void);

extern int32_t bcm2835_vc_get_board_mac_address(/*@out@*/uint8_t *);

extern int32_t bcm2835_vc_get_get_firmware_revision(void);

extern int32_t bcm2835_vc_get_get_board_model(void);
extern int32_t bcm2835_vc_get_get_board_revision(void);

extern int32_t bcm2835_vc_get_memory(const uint32_t);

#ifdef __cplusplus
}
#endif

#endif /* BCM2835_VC_H_ */
