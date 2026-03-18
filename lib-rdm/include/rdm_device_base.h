/**
 * @file rdm_device_base.h
 *
 */
/* Copyright (C) 2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDM_DEVICE_MINIMUM_H_
#define RDM_DEVICE_MINIMUM_H_

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <algorithm>

#include "rdmconst.h"
#include "rdm_e120.h"
#include "hal_serialnumber.h"

namespace rdm::device
{
class Base
{
   public:
    static Base& Instance()
    {
        static Base instance; // one instance for the whole program
        return instance;
    }

    void Print()
    {
		puts("RDM Device Base");
        const auto kLength = static_cast<int>(std::min(static_cast<size_t>(RDM_MANUFACTURER_LABEL_MAX_LENGTH), strlen(RDMConst::MANUFACTURER_NAME)));
        printf(" Manufacturer Name : %.*s\n", kLength, const_cast<char*>(&RDMConst::MANUFACTURER_NAME[0]));
        printf(" Manufacturer ID   : %.2X%.2X\n", uid_[0], uid_[1]);
        printf(" Serial Number     : %.2X%.2X%.2X%.2X\n", serial_number_[3], serial_number_[2], serial_number_[1], serial_number_[0]);
    }

    const uint8_t* GetUID() const { return uid_; }
    const uint8_t* GetSN() const { return serial_number_; }

   private:
    Base()
    {
        hal::SerialNumber(serial_number_);

        uid_[0] = RDMConst::MANUFACTURER_ID[0];
        uid_[1] = RDMConst::MANUFACTURER_ID[1];
        uid_[2] = serial_number_[3];
        uid_[3] = serial_number_[2];
        uid_[4] = serial_number_[1];
        uid_[5] = serial_number_[0];
    }

    uint8_t uid_[RDM_UID_SIZE];
#define DEVICE_SN_LENGTH 4
    uint8_t serial_number_[DEVICE_SN_LENGTH];
};
} // namespace rdm::device

#endif // RDM_DEVICE_MINIMUM_H_
