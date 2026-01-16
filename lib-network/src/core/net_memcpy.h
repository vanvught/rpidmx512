/**
 * @file net_memcpy.h
 * @brief Provides optimized memory manipulation functions for embedded systems.
 *
 * This header defines utility functions for memory operations such as memset and memcpy.
 * Optimized for performance and alignment considerations on ARM architectures.
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstring>
#include <cstddef>

#include "core/protocol/ip4.h"

namespace network
{
/**
 * @brief Optimized memset function for setting memory with a fixed value.
 *
 * @tparam V The constant value to set each byte to.
 * @tparam L The constant length of the memory to set. Must be greater than 0.
 * @param dest Pointer to the memory block to fill.
 *
 * This implementation optimizes for compile-time knowledge of the memory size.
 * For sizes greater than `sizeof(uint32_t)`, it delegates to `std::memset` for
 * potentially more optimized bulk operations. For smaller sizes, including the
 * special case of `L == sizeof(uint32_t)`, it uses a loop to ensure alignment
 * safety and avoid unaligned access.
 *
 * @note The implementation considers potential unaligned access issues when
 * `L == sizeof(uint32_t)` and ensures safe byte-by-byte copying in this case.
 *
 * @warning The caller must ensure that `dest` points to a valid memory block
 * of at least `L` bytes to avoid undefined behavior.
 */
template <uint8_t V, size_t L> inline void memset(void* dest)
{
    static_assert(L > 0, "Length must be greater than 0");

    if constexpr (L > sizeof(uint32_t))
    {
        // Use std::memset for larger memory blocks
        std::memset(dest, V, L);
    }
    else
    {
        // Handle potential unaligned access for L <= sizeof(uint32_t)
#ifdef __ARM_ARCH_7A__
        volatile auto* pDst = reinterpret_cast<uint8_t*>(dest);
#else
        auto* pDst = reinterpret_cast<uint8_t*>(dest);
#endif
        // For smaller sizes L <= sizeof(uint32_t), write bytes one at a time
        for (size_t i = 0; i < L; i++)
        {
            *pDst++ = V;
        }
    }
}

inline void* memcpy(void* dst, const void* src, size_t n)
{
    return __builtin_memcpy(dst, src, n);
}

/**
 * @brief Copies an IPv4 address from a 32-bit source to a byte array.
 *
 * @param pIpAddress Pointer to the destination byte array.
 * @param nIpAddress 32-bit IPv4 address to copy.
 */
inline void memcpy_ip(uint8_t* pIpAddress, const uint32_t nIpAddress)
{
#ifdef __ARM_ARCH_7A__
    // Ensure destination pointer is aligned
    if ((reinterpret_cast<uint32_t>(pIpAddress) & ((sizeof(uint32_t) - 1))) == 0)
    {
        // Destination pointer is already aligned, perform fast copy
        *reinterpret_cast<uint32_t*>(pIpAddress) = nIpAddress;
    }
    else
#endif
    { // Destination pointer is not aligned, copy byte by byte
        typedef union pcast32
        {
            uint32_t u32;
            uint8_t u8[4];
        } _pcast32;

        _pcast32 src;
        src.u32 = nIpAddress;
#ifdef __ARM_ARCH_7A__
        volatile auto* pSrc = src.u8;
        volatile auto* pDst = pIpAddress;
#else
        auto* pSrc = src.u8;
        auto* pDst = pIpAddress;
#endif
        for (size_t i = 0; i < network::ip4::kAddressLength; i++)
        {
            *pDst++ = *pSrc++;
        }
    }
}

/**
 * @brief Copies an IPv4 address from a byte array to a 32-bit integer.
 *
 * @param pIpAddress Pointer to the source byte array.
 * @return The 32-bit IPv4 address.
 */
inline uint32_t memcpy_ip(const uint8_t* pIpAddress)
{
#ifdef __ARM_ARCH_7A__
    // Ensure source pointer is aligned
    if ((reinterpret_cast<uint32_t>(pIpAddress) & ((sizeof(uint32_t) - 1))) == 0)
    {
        // Source pointer is already aligned, perform fast copy
        return *reinterpret_cast<const uint32_t*>(pIpAddress);
    }
    else
#endif
    { // Source pointer is not aligned, copy byte by byte
        typedef union pcast32
        {
            uint32_t u32;
            uint8_t u8[4];
        } _pcast32;

        _pcast32 src;
#ifdef __ARM_ARCH_7A__
        volatile auto* pSrc = pIpAddress;
        volatile auto* pDst = src.u8;
#else
        auto* pSrc = pIpAddress;
        auto* pDst = src.u8;
#endif
        for (size_t i = 0; i < network::ip4::kAddressLength; i++)
        {
            *pDst++ = *pSrc++;
        }

        return src.u32;
    }
}
} // namespace network

#endif /* NET_MEMCPY_H_ */
