/**
 * @file network.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 *
 * Art-Net 3 Protocol Release V1.4 Document Revision 1.4bk 23/1/2016
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef NETWORK_H_
#define NETWORK_H_

#include <stdint.h>

#define NETWORK_MAC_SIZE	6

#ifdef __cplusplus
extern "C" {
#endif

extern const bool network_get_macaddr(/*@out@*/const uint8_t *);
extern const uint32_t network_get_ip(void);
extern const uint32_t network_get_netmask(void);
extern const uint32_t network_get_bcast(void);
extern /*@null@*/const char *network_get_hostname(void);
extern bool network_is_dhcp_used(void);

extern void network_begin(uint16_t);
extern uint16_t network_recvfrom(const uint8_t *, const uint16_t, uint32_t *, uint16_t *);
extern void network_sendto(const uint8_t *, const uint16_t, const uint32_t, const uint16_t);
extern void network_joingroup(const uint32_t);

#ifdef __cplusplus
}
#endif

#endif /* NETWORK_H_ */
