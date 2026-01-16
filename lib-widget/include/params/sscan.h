/**
 * @file sscan.h
 *
 */
/* Copyright (C) 2016-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef PARAMS_SSCAN_H_
#define PARAMS_SSCAN_H_

#include <cstdint>
#include <cctype>
#include <cassert>

class Sscan
{
   public:
    enum ReturnCode
    {
        OK,
        NAME_ERROR,
        VALUE_ERROR
    };
    
    static const char *CheckName(const char *buffer, const char *name) {
		while ((*name != 0) && (*buffer != 0)) {
			if (*name++ != *buffer++) {
				return nullptr;
			}
		}

		if (*name != 0) {
			return nullptr;
		}

		if (*buffer++ != '=') {
			return nullptr;
		}

		if ((*buffer == ' ') || (*buffer == 0)) {
			return nullptr;
		}

		return buffer;
}

    static ReturnCode Uint8(const char* buffer, const char* name, uint8_t& value)
    {
        assert(buffer != nullptr);
        assert(name != nullptr);

        const char* p;

        if ((p = CheckName(buffer, name)) == nullptr)
        {
            return Sscan::NAME_ERROR;
        }

        uint32_t k = 0;

        do
        {
            if (isdigit(*p) == 0)
            {
                return Sscan::VALUE_ERROR;
            }
            k = k * 10 + static_cast<uint32_t>(*p) - '0';
            p++;
        } while ((*p != ' ') && (*p != 0));

        if (k > static_cast<uint32_t>(static_cast<uint8_t>(~0)))
        {
            return Sscan::VALUE_ERROR;
        }

        value = static_cast<uint8_t>(k);

        return Sscan::OK;
    }
};

#endif  // PARAMS_SSCAN_H_
