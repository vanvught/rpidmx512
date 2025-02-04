/**
 * @file igmp.h
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NET_IGMP_H_
#define NET_IGMP_H_

#include <cstdint>

namespace net {
void igmp_shutdown();
void igmp_join(const uint32_t);
void igmp_leave(const uint32_t);
void igmp_report_groups();
bool igmp_lookup_group(const uint32_t);
}  // namespace net

#if defined (CONFIG_EMAC_HASH_MULTICAST_FILTER)
void emac_multicast_enable_hash_filter();
void emac_multicast_disable_hash_filter();
void emac_multicast_set_hash(const uint8_t *);
void emac_multicast_reset_hash();
#endif

#endif /* NET_IGMP_H_ */
