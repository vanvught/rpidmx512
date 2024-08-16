/**
 * @file net_memcpy.h
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NET_MEMCPY_H_
#define NET_MEMCPY_H_

#include <cstdint>
#include <cstddef>

#include "net/protocol/ip4.h"

namespace net {
inline void* memcpy(void *__restrict__ dest, void const *__restrict__ src, size_t n) {
	auto *plDst = reinterpret_cast<uintptr_t*>(dest);
	auto const *plSrc = reinterpret_cast<uintptr_t const*>(src);

	if (((reinterpret_cast<uintptr_t>(src) & (sizeof(uintptr_t) - 1)) == 0)
			&& ((reinterpret_cast<uintptr_t>(dest) & (sizeof(uintptr_t) - 1)) == 0)) {
		while (n >= sizeof(uintptr_t)) {
			*plDst++ = *plSrc++;
			n -= sizeof(uintptr_t);
		}
	}

	auto *pcDst = reinterpret_cast<uint8_t*>(plDst);
	auto const *pcSrc = reinterpret_cast<uint8_t const*>(plSrc);

	while (n--) {
		*pcDst++ = *pcSrc++;
	}

	return dest;
}

inline void memcpy_ip(uint8_t *pIpAddress, const uint32_t nIpAddress) {
#ifdef __ARM_ARCH_7A__
	// Ensure destination pointer is aligned
	if ((reinterpret_cast<uint32_t>(pIpAddress) & ((sizeof(uint32_t) - 1))) == 0) {
		// Destination pointer is already aligned, perform fast copy
		*reinterpret_cast<uint32_t *>(pIpAddress) = nIpAddress;
	} else
#endif
	{	// Destination pointer is not aligned, copy byte by byte
		typedef union pcast32 {
			uint32_t u32;
			uint8_t u8[4];
		} _pcast32;

		_pcast32 src;
		src.u32 = nIpAddress;
#ifdef __ARM_ARCH_7A__
		volatile auto *pSrc = src.u8;
		volatile auto *pDst = pIpAddress;
#else
		auto *pSrc = src.u8;
		auto *pDst = pIpAddress;
#endif
		for (uint32_t i = 0; i < IPv4_ADDR_LEN; i++) {
			pDst[i] = pSrc[i];
		}
	}
}

inline uint32_t memcpy_ip(const uint8_t *pIpAddress) {
#ifdef __ARM_ARCH_7A__
	// Ensure destination pointer is aligned
	if ((reinterpret_cast<uint32_t>(pIpAddress) & ((sizeof(uint32_t) - 1))) == 0) {
		// Destination pointer is already aligned, perform fast copy
		return *reinterpret_cast<const uint32_t *>(pIpAddress);
	} else
#endif
	{	// Destination pointer is not aligned, copy byte by byte
		typedef union pcast32 {
			uint32_t u32;
			uint8_t u8[4];
		} _pcast32;

		_pcast32 src;
#ifdef __ARM_ARCH_7A__
		volatile auto *pSrc = pIpAddress;
		volatile auto *pDst = src.u8;
#else
		auto *pSrc = pIpAddress;
		auto *pDst = src.u8;
#endif
		for (uint32_t i = 0; i < IPv4_ADDR_LEN; i++) {
			pDst[i] = pSrc[i];
		}

		return src.u32;
	}
}
}  // namespace net

#endif /* NET_MEMCPY_H_ */
