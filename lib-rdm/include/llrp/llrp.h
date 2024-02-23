/**
 * @file llrp.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef LLRP_H_
#define LLRP_H_

/**
 * Table A-2: LLRP Constants
 */
#define LLRP_MULTICAST_IPV4_ADDRESS_REQUEST  "239.255.250.133"
#define LLRP_MULTICAST_IPV4_ADDRESS_RESPONSE "239.255.250.134"
#define LLRP_MULTICAST_IPV6_ADDRESS_REQUEST  "ff18::85:0:0:85"
#define LLRP_MULTICAST_IPV6_ADDRESS_RESPONSE "ff18::85:0:0:86"
#define LLRP_PORT                            5569
#define LLRP_TIMEOUT_MS                      2000 /* milliseconds */
#define LLRP_TARGET_TIMEOUT_MS               500  /* milliseconds */
#define LLRP_MAX_BACKOFF_MS                  1500 /* milliseconds */
#define LLRP_KNOWN_UID_SIZE                  200
#define LLRP_BROADCAST_CID                   "fbad822c-bd0c-4d4c-bdc8-7eabebc85aff"

#endif /* LLRP_H_ */
