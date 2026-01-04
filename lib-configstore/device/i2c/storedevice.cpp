/**
 * @file storedevice.cpp
 *
 */
/* Copyright (C) 2022-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_CONFIGSTORE)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "configstoredevice.h"
#include "i2c/at24cxx.h"
 #include "firmware/debug/debug_debug.h"

namespace storedevice
{
#if !defined(CONFIG_FLASHROM_I2C_INDEX)
#define CONFIG_FLASHROM_I2C_INDEX 0
#endif
static constexpr uint8_t kI2CIndex = CONFIG_FLASHROM_I2C_INDEX;
/* Backwards compatibility with SPI FLASH */
static constexpr auto kFlashSectorSize = 4096U;
static constexpr auto kRomSize = 4096U;
} // namespace storedevice

StoreDevice::StoreDevice() : AT24C32(storedevice::kI2CIndex)
{
    DEBUG_ENTRY();

    detected_ = AT24C32::IsConnected();

    if (!detected_)
    {
        printf("StoreDevice: No AT24C32 at %2x", AT24C32::GetAddress());
    }
    else
    {
        printf("StoreDevice: AT24C32 total %u bytes [%u kB]\n", GetSize(), GetSize() / 1024U);
    }

    DEBUG_EXIT();
}

StoreDevice::~StoreDevice(){DEBUG_ENTRY(); DEBUG_EXIT();}

uint32_t StoreDevice::GetSize() const
{
    return storedevice::kRomSize;
}

uint32_t StoreDevice::GetSectorSize() const
{
    return storedevice::kFlashSectorSize;
}

bool StoreDevice::Read(uint32_t offset, uint32_t length, uint8_t* buffer, storedevice::Result& result)
{
    DEBUG_ENTRY();
    assert((offset + length) <= storedevice::ROM_SIZE);

    AT24C32::Read(offset, buffer, length);

    result = storedevice::Result::kOk;

    DEBUG_EXIT();
    return true;
}

bool StoreDevice::Erase([[maybe_unused]] uint32_t offset, [[maybe_unused]] uint32_t length, storedevice::Result& result)
{
    DEBUG_ENTRY();

    result = storedevice::Result::kOk;

    DEBUG_EXIT();
    return true;
}

bool StoreDevice::Write(uint32_t offset, uint32_t length, const uint8_t* buffer, storedevice::Result& result)
{
    DEBUG_ENTRY();
    assert((offset + length) <= ROM_SIZE);

    AT24C32::Write(offset, buffer, length);

    result = storedevice::Result::kOk;

    DEBUG_EXIT();
    return true;
}
