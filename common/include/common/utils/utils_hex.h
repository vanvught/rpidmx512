/**
 * @file utils_hex.h
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

#ifndef COMMON_UTILS_UTILS_HEX_H_
#define COMMON_UTILS_UTILS_HEX_H_

#include <cstdint>
#include <cstddef>
#include <cassert>

namespace common::hex
{
constexpr char ToCharLowercase(uint32_t value)
{
    return "0123456789abcdef"[value & 0xf];
}

constexpr char ToCharUppercase(uint32_t value)
{
    return "0123456789ABCDEF"[value & 0xf];
}

enum class Case
{
    kLower,
    kUpper
};

template <Case letter_case = Case::kLower, size_t N> 
char* ToString(char (&string)[N + 1], uint32_t value)
{
    static_assert(N % 2 == 0, "Hex string length must be even");
    static_assert(N <= 8, "Cannot represent more than 32 bits");

    for (size_t i = 0; i < N; ++i)
    {
        size_t shift = (N - 1 - i) * 4;
        uint32_t nybble = (value >> shift);

        if constexpr (letter_case == Case::kLower)
        {
            string[i] = hex::ToCharLowercase(nybble);
        }
        else
        {
            string[i] = hex::ToCharUppercase(nybble);
        }
    }

    string[N] = '\0';
    return string;
}

template <size_t N> 
char* ToStringLower(char (&s)[N + 1], uint32_t v)
{
    return ToString<Case::kLower, N>(s, v);
}

template <size_t N> 
char* ToStringUpper(char (&s)[N + 1], uint32_t v)
{
    return ToString<Case::kUpper, N>(s, v);
}

constexpr uint8_t FromChar(char c)
{
    return (c >= '0' && c <= '9')   ? static_cast<uint8_t>(c - '0')
           : (c >= 'a' && c <= 'f') ? static_cast<uint8_t>(c - 'a' + 10)
           : (c >= 'A' && c <= 'F') ? static_cast<uint8_t>(c - 'A' + 10)
                                    : 0xFF; // Invalid
}

template <size_t N> constexpr uint32_t FromHex(const char (&string)[N])
{
    static_assert(N > 1, "Hex string must not be empty (must include \\0)");
    static_assert(N <= 9, "Too many hex digits for uint32_t");
    assert(string[N - 1] == '\0');

    uint32_t result = 0;
    for (size_t i = 0; i < N - 1; ++i)
    {
        const uint8_t kNibble = FromChar(string[i]);
        if (kNibble == 0xFF)
        {
            return 0; // Or: static_assert(false, "Invalid hex digit"); if desired
        }
        result = (result << 4) | kNibble;
    }
    return result;
}

} // namespace common::hex

#endif  // COMMON_UTILS_UTILS_HEX_H_
