/**
 * @file oscsimplemessage.cpp
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstring>

#include "oscsimplemessage.h"
#include "osc.h"

OscSimpleMessage::OscSimpleMessage(const uint8_t* osc_message, uint32_t length) : osc_message_(osc_message), length_(length)
{
    auto result = osc::StringValidate(osc_message_, length);

    if (result < 0)
    {
        return;
    }

    arg_ = &osc_message_[result];
    auto data_offset = static_cast<uint32_t>(result);

    result = osc::StringValidate(arg_, length_ - static_cast<unsigned>(result));

    if ((result < 0) || (arg_[0] != ','))
    {
        return;
    }

    // Support for 1 osc-string or blob only
    if (((arg_[1] == 's') || (arg_[1] == 'b')) && (arg_[2] != '\0'))
    {
        return;
    }

    arg_++; // Skip ','
    argc_ = strlen(reinterpret_cast<const char*>(arg_));

    data_offset += static_cast<uint32_t>(result);

    osc_message_data_ = &osc_message_[data_offset];
    osc_message_data_length_ = length_ - data_offset;

    is_valid_ = true;
}

float OscSimpleMessage::GetFloat(unsigned argc)
{
    union pcast32
    {
        int32_t i;
        float f;
    } osc_pcast32;

    if ((osc_message_data_length_ >= 4 * (1 + argc)) && (arg_[argc] == osc::type::FLOAT))
    {
        osc_pcast32.i = static_cast<int32_t>(__builtin_bswap32(*reinterpret_cast<const uint32_t*>((4 * argc) + osc_message_data_)));
        return osc_pcast32.f;
    }

    return 0;
}

int OscSimpleMessage::GetInt(unsigned argc)
{
    if ((osc_message_data_length_ >= 4 * (1 + argc)) && (arg_[argc] == osc::type::INT32))
    {
        return static_cast<int>(__builtin_bswap32(*reinterpret_cast<const uint32_t*>((4 * argc) + osc_message_data_)));
    }

    return 0;
}

const char* OscSimpleMessage::GetString([[maybe_unused]] unsigned argc)
{
    if ((arg_[0] == osc::type::STRING) && (osc_message_data_length_ == osc::StringSize(reinterpret_cast<const char*>(osc_message_data_))))
    {
        return reinterpret_cast<const char*>(osc_message_data_);
    }

    return nullptr;
}

OSCBlob OscSimpleMessage::GetBlob([[maybe_unused]] unsigned argc)
{
    if (arg_[0] == osc::type::BLOB)
    {
        auto size = __builtin_bswap32(*reinterpret_cast<const uint32_t*>(osc_message_data_));
        auto* data = osc_message_data_ + 4;

        if ((size + 4) <= osc_message_data_length_)
        {
            return OSCBlob(data, size);
        }
    }

    return OSCBlob(nullptr, 0);
}
