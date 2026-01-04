/**
 * @file rdmsensorbh1750.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDMSENSORBH1750_H_
#define RDMSENSORBH1750_H_

#include <cstdint>

#include "rdmsensor.h"
#include "bh1750.h"
#include "rdm_e120.h"

class RDMSensorBH170 : public RDMSensor, sensor::BH170
{
   public:
    explicit RDMSensorBH170(uint8_t sensor, uint8_t address = 0) : RDMSensor(sensor), sensor::BH170(address)
    {
        SetType(E120_SENS_ILLUMINANCE);
        SetUnit(E120_UNITS_LUX);
        SetPrefix(E120_PREFIX_NONE);
        SetRangeMin(rdm::sensor::SafeRangeMin(sensor::bh1750::RANGE_MIN));
        SetRangeMax(rdm::sensor::SafeRangeMax(sensor::bh1750::RANGE_MAX));
        SetNormalMin(rdm::sensor::SafeRangeMin(sensor::bh1750::RANGE_MIN));
        SetNormalMax(rdm::sensor::SafeRangeMax(sensor::bh1750::RANGE_MAX));
        SetDescription(sensor::bh1750::DESCRIPTION);
    }

    bool Initialize() override { return sensor::BH170::Initialize(); }

    int16_t GetValue() override { return static_cast<int16_t>(sensor::BH170::Get() & 0x7FFF); }
};

#endif  // RDMSENSORBH1750_H_
