/**
 * @file utils_enum.h
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

#ifndef COMMON_UTILS_UTILS_ENUM_H_
#define COMMON_UTILS_UTILS_ENUM_H_

#include <type_traits>

namespace common
{

/// Converts an enum class value to its underlying integer type.
template <typename Enum>
inline auto ToValue(Enum e) noexcept {
    return static_cast<std::underlying_type_t<Enum>>(e);
}

/// Converts an integer value to the corresponding enum class value.
template <typename Enum>
inline Enum FromValue(std::underlying_type_t<Enum> value) noexcept {
    return static_cast<Enum>(value);
}

} // namespace common

#endif  // COMMON_UTILS_UTILS_ENUM_H_
