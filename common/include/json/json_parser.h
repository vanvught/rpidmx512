/**
 * @file json_parser.h
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

#ifndef JSON_JSON_PARSER_H_
#define JSON_JSON_PARSER_H_

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "common/utils/utils_hash.h"
#include "json/json_key.h"
#include "json/json_tokenizer.h"

inline void ParseJsonWithTable(const char* buffer, size_t size, const json::Key* keys, size_t key_count)
{
    JsonTokenizer tok(buffer, size);
    tok.SkipWhitespace();

    if (tok.p >= tok.end || *tok.p != '{') return;
    ++tok.p;

    while (tok.p < tok.end)
    {
        const char* json_key;
        size_t json_key_len;
        if (!tok.NextString(json_key, json_key_len)) break;

        if (!tok.Expect(':')) break;

        const char* val;
        size_t val_len;
        if (!tok.NextValue(val, val_len)) break;

        uint32_t h = Fnv1a32Runtime(json_key, static_cast<uint32_t>(json_key_len));
        bool matched = false;

        for (size_t i = 0; i < key_count; ++i)
        {
            if (keys[i].GetHash() == h)
            {
                if (keys[i].type == json::Key::kSimple)
                {
                    keys[i].set_simple(val, val_len);
                }
                else
                {
                    keys[i].set_keyed(json_key, json_key_len, val, val_len);
                }
                matched = true;
                break;
            }
        }

        if (!matched)
        {
            // Unknown key
        }

        tok.SkipWhitespace();
        if (tok.p < tok.end && *tok.p == ',')
        {
            ++tok.p;
        }
        else if (tok.p < tok.end && *tok.p == '}')
        {
            break;
        }
    }
}

template <size_t N> inline void ParseJsonWithTable(const char* buffer, size_t size, const json::Key (&keys)[N])
{
    ParseJsonWithTable(buffer, size, keys, N);
}

#endif  // JSON_JSON_PARSER_H_
