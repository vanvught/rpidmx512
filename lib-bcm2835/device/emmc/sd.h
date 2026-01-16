/**
 * @file sd.h
 *
 */
/* Copyright (C) 2015, 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
 * Based on
 * https://github.com/jncronin/rpi-boot/blob/master/emmc.c
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

#ifndef SD_H_
#define SD_H_

#include <stdint.h>
#include <stddef.h>

#define SD_OK                0
#define SD_ERROR             -1
#define SD_TIMEOUT           -2
#define SD_BUSY              -3
#define SD_NO_RESP           -5
#define SD_ERROR_RESET       -6
#define SD_ERROR_CLOCK       -7
#define SD_ERROR_VOLTAGE     -8
#define SD_ERROR_APP_CMD     -9
#define SD_CARD_CHANGED      -10
#define SD_CARD_ABSENT       -11
#define SD_CARD_REINSERTED   -12

extern int sd_card_init(void);
extern int sd_read(uint8_t *, size_t, uint32_t);
#ifdef SD_WRITE_SUPPORT
extern int sd_write(uint8_t *, size_t, uint32_t);
#endif

#endif /* SD_H_ */
