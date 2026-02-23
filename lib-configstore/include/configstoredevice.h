/**
 * @file configstoredevice.h
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

#ifndef CONFIGSTOREDEVICE_H_
#define CONFIGSTOREDEVICE_H_

#include <cstdint>

namespace storedevice
{
enum class Result
{
    kOk,
    kError
};
} // namespace storedevice

#if defined(CONFIG_STORE_USE_I2C)
#include "i2c/at24cxx.h"
class StoreDevice : AT24C32
{
#elif defined(CONFIG_STORE_USE_ROM)
#include "flashcode.h"
class StoreDevice : FlashCode
{
#else
class StoreDevice
{
#endif
   public:
    StoreDevice();
    ~StoreDevice();

    bool IsDetected() const { return detected_; }
    uint32_t GetSectorSize() const;
    uint32_t GetSize() const;

    bool Read(uint32_t offset, uint32_t length, uint8_t* buffer, storedevice::Result& result);
    bool Erase(uint32_t offset, uint32_t length, storedevice::Result& result);
    bool Write(uint32_t offset, uint32_t length, const uint8_t* buffer, storedevice::Result& result);

   private:
    bool detected_{false};
};

#endif  // CONFIGSTOREDEVICE_H_
