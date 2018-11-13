/**
 * @file w5x00.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef W5X00_H_
#define W5X00_H_

#include <stdint.h>

typedef enum {
	W5X00_INIT_OK = 0,
	W5X00_INIT_NOT_ROOT = -1,
	W5X00_INIT_LIB_INIT_FAILED = -2,
	W5X00_INIT_WIZCHIP_INIT_FAILED = -3,
	W5X00_INIT_WIZPHY_GET_FAILED = -4
} W5X00_Init;

struct ip_addr {
    uint32_t addr;
};

typedef struct ip_addr ip_addr_t;

struct ip_info {
    struct ip_addr ip;
    struct ip_addr netmask;
    struct ip_addr gw;
};

#ifdef __cplusplus
extern "C" {
#endif

extern W5X00_Init w5x00_init(uint8_t nSpiChipSelect, uint32_t nSpiSpeedHz, const uint8_t *pMacAddress, struct ip_info *pIpInfo);
extern const char *w5x00_get_chip_name(void);

extern int8_t w5x00_join_group(uint8_t sn, uint32_t ip, uint16_t port);

#ifdef __cplusplus
}
#endif

#endif /* W5X00_H_ */
