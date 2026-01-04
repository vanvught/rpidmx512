/**
 * @file json_parsehelper.h
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

#ifndef JSON_JSON_PARSEHELPER_H_
#define JSON_JSON_PARSEHELPER_H_

#include <cstdint>
#include <type_traits>

namespace json
{
inline int32_t Atoi(const char* buffer, uint32_t size)
{
    const char* p = buffer;
    int32_t sign = 1;
    int32_t res = 0;

    if (size == 0)
    {
        return 0;
    }

    if (*p == '-')
    {
        sign = -1;
        p++;
        size--;
    }
    else if (*p == '+')
    {
        p++;
        size--;
    }

    for (; (size > 0) && (*p >= '0' && *p <= '9'); size--, p++)
    {
        res = res * 10 + (*p - '0');
    }

    return sign * res;
}

inline float Atof(const char* buffer, uint32_t size)
{
    const char* p = buffer;
    float sign = 1.0f;
    float result = 0.0f;

    if (size == 0)
    {
        return 0.0f;
    }

    if (*p == '-')
    {
        sign = -1.0f;
        ++p;
        --size;
    }
    else if (*p == '+')
    {
        ++p;
        --size;
    }

    // Parse integer part
    while (size > 0 && *p >= '0' && *p <= '9')
    {
        result = result * 10.0f + static_cast<float>(*p - '0');
        ++p;
        --size;
    }

    // Parse fractional part
    if (size > 0 && *p == '.')
    {
        ++p;
        --size;

        float divisor = 10.0f;
        while (size > 0 && *p >= '0' && *p <= '9')
        {
            result += static_cast<float>(*p - '0') / divisor;
            divisor *= 10.0f;
            ++p;
            --size;
        }
    }

    return sign * result;
}

template <typename T> T ParseValue(const char* val, uint32_t len)
{
    int32_t v = Atoi(val, len);
    if constexpr (std::is_unsigned_v<T>)
    {
        if (v < 0)
        {
            return 0; // or handle error
        }
    }
    return static_cast<T>(v);
}

template <typename T, typename F> void ParseAndApply(const char* val, uint32_t len, F&& apply)
{
    apply(ParseValue<T>(val, len));
}
} // namespace json

#endif  // JSON_JSON_PARSEHELPER_H_
