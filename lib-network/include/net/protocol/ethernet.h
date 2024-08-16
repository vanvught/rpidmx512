/**
 * @file ethernet.h
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NET_PROTOCOL_ETHERNET_H_
#define NET_PROTOCOL_ETHERNET_H_

#include <stdint.h>

#if !defined (PACKED)
# define PACKED __attribute__((packed))
#endif

enum MTU {
	MTU_SIZE = 1500
};

enum ETH_ADDR {
	ETH_ADDR_LEN = 6
};

struct ether_header {
	uint8_t dst[ETH_ADDR_LEN];		/*  6 */
	uint8_t src[ETH_ADDR_LEN];		/* 12 */
	uint16_t type;					/* 14 */
} PACKED;

namespace net {
static constexpr uint32_t ETH_HWADDR_LEN = 6;


}  // namespace net

#endif /* NET_PROTOCOL_ETHERNET_H_ */
