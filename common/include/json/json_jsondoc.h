/**
 * @file json_jsondoc.h
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

#ifndef JSON_JSON_JSONDOC_H_
#define JSON_JSON_JSONDOC_H_

#include <cstdint>
#include <cassert>
#include <cstdio>

class JsonDoc
{
   public:
    JsonDoc(char* buf, uint32_t max_len) : buf_(buf), max_len_(max_len)
    {
        assert(buf != nullptr);
        assert(max_len > 2); // Need at least space for {}
        Write("{");
    }

    ~JsonDoc() = default;

    class KeyProxy
    {
       public:
        KeyProxy(JsonDoc& doc, const char* key) : doc_(doc), key_(key) {}

        KeyProxy& operator=(const char* value)
        {
            doc_.WriteField(key_, value);
            return *this;
        }

        KeyProxy& operator=(uint32_t value)
        {
            doc_.WriteField(key_, value);
            return *this;
        }

       private:
        JsonDoc& doc_;
        const char* key_;
    };

    KeyProxy operator[](const char* key) { return KeyProxy(*this, key); }

    void End() { Write("}"); }

    uint32_t Size() const { return pos_; }

   private:
    int CopyString(char* dst, size_t size, const char* src)
    {
        if (size == 0)
        {
            return 0;
        }

        size_t length = 0;
        while ((length < size - 1) && (*src != '\0'))
        {
            *dst++ = *src++;
            ++length;
        }

        *dst = '\0'; // Always null-terminate
        return static_cast<int>(length);
    }

    void WriteField(const char* key, const char* value)
    {
        if (!first_)
        {
            Write(",");
        }

        Write("\"");
        Write(key);
        Write("\":\"");
        Write(value);
        Write("\"");
        first_ = false;
    }

    void WriteField(const char* key, uint32_t value)
    {
        if (!first_)
        {
            Write(",");
        }

        Write("\"");
        Write(key);
        Write("\":");

        char num_buf[11]; // Enough for 32-bit uint
        char* p = num_buf + sizeof(num_buf);
        *--p = '\0';

        if (value == 0)
        {
            *--p = '0';
        }
        else
        {
            while (value != 0)
            {
                *--p = static_cast<char>('0' + (value % 10));
                value /= 10;
            }
        }

        Write(p); // Already null-terminated
        first_ = false;
    }

    void Write(const char* s)
    {
        if (pos_ >= max_len_)
        {
            return;
        }

        int ret = CopyString(buf_ + pos_, max_len_ - pos_, s);

        // CopyString always null-terminates and returns number of chars copied (excluding null)
        if (ret >= 0 && static_cast<uint32_t>(ret) + pos_ < max_len_)
        {
            pos_ += static_cast<uint32_t>(ret);
        }
        else
        {
            pos_ = max_len_; // Clamp to signal overflow
        }
    }

    char* buf_;
    uint32_t max_len_;
    bool first_{true};
    uint32_t pos_{0};
};

#endif  // JSON_JSON_JSONDOC_H_
