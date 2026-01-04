/**
 * @file rdmnetdevice.h
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDMNETDEVICE_H_
#define RDMNETDEVICE_H_

#include <cstdint>

#include "e117.h"
#include "llrp/llrpdevice.h"
#include "hal_uuid.h"
#include "firmware/debug/debug_debug.h"

class RDMNetDevice final : public LLRPDevice
{
   public:
    RDMNetDevice()
    {
        DEBUG_ENTRY();

        DEBUG_EXIT();
    }

    ~RDMNetDevice()
    {
        DEBUG_ENTRY();

        DEBUG_EXIT();
    };

    void Print()
    {
        static constexpr auto kUuidStringLength = 36;
        char uuid_str[kUuidStringLength + 1];
        uuid_str[kUuidStringLength] = '\0';

        uint8_t cid[e117::kCidLength];
        hal::UuidCopy(cid);
        uuid_unparse(cid, uuid_str);

        printf("RDMNet\n");
        printf(" CID : %s\n", uuid_str);

        LLRPDevice::Print();
    }
};

#endif /* RDMNETDEVICE_H_ */
