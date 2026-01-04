/**
 * @file json_tokenizer.h
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
 

#ifndef JSON_JSON_TOKENIZER_H_
#define JSON_JSON_TOKENIZER_H_

#include <cstddef>

struct JsonTokenizer
{
    const char* p;
    const char* end;

    constexpr JsonTokenizer(const char* buffer, size_t size) : p(buffer), end(buffer + size) {}

    constexpr void SkipWhitespace()
    {
        while (p < end && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) ++p;
    }

    bool NextString(const char*& out, size_t& len)
    {
        SkipWhitespace();
        if (p >= end || *p != '"') return false;
        ++p; // skip "
        out = p;
        while (p < end && *p != '"') ++p;
        if (p >= end) return false;
        len = static_cast<size_t>(p - out);
        ++p; // skip "
        return true;
    }

    bool Expect(char c)
    {
        SkipWhitespace();
        if (p >= end || *p != c) return false;
        ++p;
        return true;
    }

    bool NextValue(const char*& out, size_t& len)
    {
        SkipWhitespace();
        if (p >= end) return false;

        if (*p == '"')
        {
            return NextString(out, len);
        }

        out = p;
        while (p < end && *p != ',' && *p != '}' && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') ++p;
        len = static_cast<size_t>(p - out);
        return len > 0;
    }
};

#endif  // JSON_JSON_TOKENIZER_H_
