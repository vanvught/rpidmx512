/**
 * @file oscsimplemessage.h
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

#ifndef OSCSIMPLEMESSAGE_H_
#define OSCSIMPLEMESSAGE_H_

#include <cstdint>

#include "osc.h"
#include "oscblob.h"

class OscSimpleMessage
{
   public:
    OscSimpleMessage(const uint8_t* osc_message, uint32_t length);

    bool IsValid() const { return is_valid_; }

    int GetArgc() const
    {
        if (is_valid_)
        {
            return static_cast<int>(argc_);
        }

        return -1;
    }

    char GetType(unsigned argc) const
    {
        if (argc < argc_)
        {
            return static_cast<char>(arg_[argc]);
        }

        return osc::type::UNKNOWN;
    }

    float GetFloat(unsigned argc);
    int GetInt(unsigned argc);
    const char* GetString(unsigned argc);
    OSCBlob GetBlob(unsigned argc);

   private:
    const uint8_t* osc_message_;
    uint32_t length_;
    const uint8_t* arg_;
    uint32_t argc_{0};
    const uint8_t* osc_message_data_{nullptr};
    uint32_t osc_message_data_length_{0};
    bool is_valid_{false};
};

#endif  // OSCSIMPLEMESSAGE_H_
