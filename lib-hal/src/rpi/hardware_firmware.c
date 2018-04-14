/**
 * @file hardware_firmware.c
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>

#include <bcm2835_vc.h>

static const char BROADCOM_COPYRIGHT[] __attribute__((aligned(4))) = "Copyright (c) 2012 Broadcom";					///<
static const uint8_t BROADCOM_COPYRIGHT_LENGTH = (sizeof(BROADCOM_COPYRIGHT) / sizeof(BROADCOM_COPYRIGHT[0])) - 1;	///< Length of \ref BROADCOM_COPYRIGHT


int32_t hardware_firmware_get_revision(void) {
	return bcm2835_vc_get_get_firmware_revision();
}

/*@shared@*/const char *hardware_firmware_get_copyright(void) {
	return BROADCOM_COPYRIGHT;
}

uint8_t hardware_firmware_get_copyright_length(void) {
	return BROADCOM_COPYRIGHT_LENGTH;
}

