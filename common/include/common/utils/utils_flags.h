/**
 * @file utils_flags.h
 * Generic enum class bitmask helpers (C++23, freestanding-safe, Google Style)
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef COMMON_UTILS_UTILS_FLAGS_H_
#define COMMON_UTILS_UTILS_FLAGS_H_

#include <cstdint>
#include <type_traits>
#include <utility>

namespace common {
template <typename E>
    requires std::is_enum_v<E>
constexpr E operator|(E lhs, E rhs) {
    return static_cast<E>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

template <typename E>
    requires std::is_enum_v<E>
constexpr E operator&(E lhs, E rhs) {
    return static_cast<E>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

template <typename E>
    requires std::is_enum_v<E>
constexpr E operator~(E e) {
    return static_cast<E>(~std::to_underlying(e));
}

template <typename E>
    requires std::is_enum_v<E>
constexpr E& operator|=(E& lhs, E rhs) {
    lhs = lhs | rhs;
    return lhs;
}

template <typename E>
    requires std::is_enum_v<E>
constexpr E& operator&=(E& lhs, E rhs) {
    lhs = lhs & rhs;
    return lhs;
}

template <typename E>
    requires std::is_enum_v<E>
constexpr void SetFlag(uint32_t& flags, E bit, bool enable) {
    if (enable) {
        flags |= std::to_underlying(bit);
    } else {
        flags &= ~std::to_underlying(bit);
    }
}

template <typename E>
    requires std::is_enum_v<E>
constexpr uint32_t SetFlagValue(uint32_t flags, E bit, bool enable) {
    if (enable) {
        return flags | std::to_underlying(bit);
    }
         
	return flags & ~std::to_underlying(bit);
}

template <typename E>
    requires std::is_enum_v<E>
constexpr bool IsFlagSet(uint32_t flags, E bit) {
    return (flags & std::to_underlying(bit)) != 0;
}
} // namespace common

#endif // COMMON_UTILS_UTILS_FLAGS_H_
