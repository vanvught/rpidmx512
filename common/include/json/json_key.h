/**
 * @file json_key.h
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

#ifndef JSON_JSON_KEY_H_
#define JSON_JSON_KEY_H_

#include <cstdint>
#include <cstddef>

#include "common/utils/utils_hash.h"

namespace json
{
struct SimpleKey
{
    const char* name;
    uint8_t length;
    uint32_t hash;
};

struct PortKey
{
    const char* name;
    uint8_t length;
    uint32_t hash;
};

struct Key
{
    union
    {
        const SimpleKey* simple_key;
        const PortKey* port_key;
    };

    union
    {
        void (*set_simple)(const char*, uint32_t);
        void (*set_keyed)(const char*, uint32_t, const char*, uint32_t);
    };

    enum
    {
        kSimple,
        kKeyed
    } type;
    
    constexpr const char* GetName() const noexcept {
	    return type == kSimple ? simple_key->name : port_key->name;
	}
	
	constexpr uint8_t GetLength() const noexcept {
	    return type == kSimple ? simple_key->length : port_key->length;
	}

    constexpr uint32_t GetHash() const noexcept {
		 return type == kSimple ? simple_key->hash : port_key->hash;
	}
};

constexpr Key MakeKey(void (*set)(const char*, uint32_t), const SimpleKey& simple) noexcept
{
    return Key{
        .simple_key = &simple,
        .set_simple = set,
        .type = Key::kSimple
    };
}

constexpr Key MakeKey(void (*set)(const char*, uint32_t, const char*, uint32_t), const PortKey& port) noexcept
{
    return Key{
        .port_key = &port,
        .set_keyed = set,
        .type = Key::kKeyed
    };
}
} // namespace json

#endif  // JSON_JSON_KEY_H_
