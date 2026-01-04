/**
 * @file rdmsensorsstore.h
 *
 */
/* Copyright (C) 2023-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDMSENSORSSTORE_H_
#define RDMSENSORSSTORE_H_

#include <cstdint>
#include <cassert>

#include "configstore.h"
#include "configurationstore.h"

namespace rdmsensors_store
{
inline void SaveCalibration(uint32_t sensor, int32_t calibration)
{
    assert(sensor < common::store::rdm::sensors::kMaxSensors);
    auto c = static_cast<int16_t>(calibration);
    ConfigStore::Instance().RdmSensorsUpdateIndexed(&common::store::RdmSensors::calibrate, sensor, c);
}
} // namespace rdmsensors_store

#endif  // RDMSENSORSSTORE_H_
