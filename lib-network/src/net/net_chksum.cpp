/**
 * @file net_chksum.cpp
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#pragma GCC push_options
#pragma GCC optimize ("O2")
#pragma GCC optimize ("no-tree-loop-distribute-patterns")

#include <cstdint>

namespace net {
uint16_t net_chksum(const void *data, uint32_t len) {
	auto *ptr = reinterpret_cast<const uint16_t *>(data);
	uint32_t sum = 0;

	while (len > 1) {
		sum += *ptr;
		ptr++;
		len -= 2;
	}

	/* Add left-over byte, if any */
	if (len > 0) {
		sum += __builtin_bswap16(static_cast<uint16_t>(*(reinterpret_cast<const uint8_t *>(ptr)) << 8));
	}

	/* Fold 32-bit sum into 16 bits */
	while (sum >> 16) {
		sum = (sum >> 16) + (sum & 0xFFFF);
	}

	return static_cast<uint16_t>(~sum);
}
}  // namespace net
