/**
 * @file rdmdevicestore.h
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

#ifndef RDMDEVICESTORE_H_
#define RDMDEVICESTORE_H_

#include <cstdint>

#include "configstore.h"
#include "configurationstore.h"
 #include "firmware/debug/debug_debug.h"

namespace rdmdevice_store
{
inline void SaveLabel(const char* label, uint32_t length)
{
	DEBUG_PRINTF("%.*s", length, label);

    ConfigStore::Instance().RdmDeviceUpdateArray(&common::store::RdmDevice::device_root_label, label, length);
    ConfigStore::Instance().RdmDeviceUpdate(&common::store::RdmDevice::device_root_label_length, static_cast<uint8_t>(length));
}
} // namespace rdm_device_store

#endif  // RDMDEVICESTORE_H_
