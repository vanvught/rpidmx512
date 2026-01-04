/**
 * @file json_params_base.h
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

#ifndef JSON_JSON_PARAMS_BASE_H_
#define JSON_JSON_PARAMS_BASE_H_

#include <cassert>
#include <cstdio>
#include <cstdint>

 #include "firmware/debug/debug_debug.h"

namespace json
{
template <typename Derived> class JsonParamsBase
{
   public:
    void Load([[maybe_unused]] const char* file_name)
    {
#if !defined(DISABLE_FS)
        FILE* fp = fopen(file_name, "r");
        if (fp != nullptr)
        {
            char buffer[512]; // Adjust as needed for max config size
            size_t size = fread(buffer, 1, sizeof(buffer), fp);
            fclose(fp);

            if (size > 0)
            {
                static_cast<Derived*>(this)->Store(buffer, static_cast<uint32_t>(size));
            }
            else
            {
                DEBUG_PUTS("Empty or failed read");
            }
#ifndef NDEBUG
            static_cast<Derived*>(this)->Dump();
#endif
        }
        else
        {
            DEBUG_PUTS("Failed to open file");
        }
#endif
    }

   protected:
    JsonParamsBase() = default;
    ~JsonParamsBase() = default;
};

} // namespace json

#endif  // JSON_JSON_PARAMS_BASE_H_
