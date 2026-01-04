/**
 * @file utils_flags.h
 * Generic enum class bitmask helpers (C++20, freestanding-safe, Google Style)
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

#ifndef COMMON_UTILS_UTILS_FLAGS_H_
#define COMMON_UTILS_UTILS_FLAGS_H_

#include <cstdint>
#include <type_traits>

#include "common/utils/utils_enum.h" // Ensure this provides ToValue and FromValue

namespace common
{

template <typename E>
    requires std::is_enum_v<E>
constexpr E operator|(E lhs, E rhs)
{
    return static_cast<E>(ToValue(lhs) | ToValue(rhs));
}

template <typename E>
    requires std::is_enum_v<E>
constexpr E operator&(E lhs, E rhs)
{
    return static_cast<E>(ToValue(lhs) & ToValue(rhs));
}

template <typename E>
    requires std::is_enum_v<E>
constexpr E operator~(E e)
{
    return static_cast<E>(~ToValue(e));
}

template <typename E>
    requires std::is_enum_v<E>
constexpr E& operator|=(E& lhs, E rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

template <typename E>
    requires std::is_enum_v<E>
constexpr E& operator&=(E& lhs, E rhs)
{
    lhs = lhs & rhs;
    return lhs;
}

template <typename E>
    requires std::is_enum_v<E>
constexpr void SetFlag(uint32_t& flags, E bit, bool enable)
{
    if (enable)
    {
        flags |= ToValue(bit);
    }
    else
    {
        flags &= ~ToValue(bit);
    }
}

template <typename E>
    requires std::is_enum_v<E>
constexpr uint32_t SetFlagValue(uint32_t flags, E bit, bool enable)
{
    if (enable)
    {
        return flags | ToValue(bit);
    }
    else
    {
        return flags & ~ToValue(bit);
    }
}

template <typename E>
requires std::is_enum_v<E>
constexpr bool IsFlagSet(uint32_t flags, E bit) {
    return (flags & ToValue(bit)) != 0;
}

} // namespace common

#endif  // COMMON_UTILS_UTILS_FLAGS_H_
