/**
 * @file sparkfundmxparams.h
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

#ifndef JSON_SPARKFUNDMXPARAMS_H_
#define JSON_SPARKFUNDMXPARAMS_H_

#include <cstdint>
#include "configurationstore.h"
#include "json/sparkfundmxparamsconst.h"
#include "json/json_key.h"
#include "json/json_params_base.h"
#include "sparkfundmx.h"

namespace json
{
class SparkFunDmxParams : public JsonParamsBase<SparkFunDmxParams>
{
   public:
    SparkFunDmxParams();
    explicit SparkFunDmxParams(uint32_t motor_index);

    SparkFunDmxParams(const SparkFunDmxParams&) = delete;
    SparkFunDmxParams& operator=(const SparkFunDmxParams&) = delete;

    SparkFunDmxParams(SparkFunDmxParams&&) = delete;
    SparkFunDmxParams& operator=(SparkFunDmxParams&&) = delete;

    void Load()
    {
        if (motor_index_ < common::store::l6470dmx::kMaxMotors)
        {
            JsonParamsBase::Load(file_name_);
        }
        else
        {
            JsonParamsBase::Load(SparkFunDmxParamsConst::kFileName);
        }
    }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set(SparkFunDmx* sparkfundmx);

   protected:
    void Dump();

   private:
    static void SetPosition(const char* val, uint32_t len);
    static void SetSpiCs(const char* val, uint32_t len);
    static void SetResetPin(const char* val, uint32_t len);
    static void SetBusyPin(const char* val, uint32_t len);

    static constexpr Key kSparkFunKeys[] = {
        MakeKey(SetPosition, SparkFunDmxParamsConst::kPosition), //
        MakeKey(SetSpiCs, SparkFunDmxParamsConst::kSpiCs),       //
        MakeKey(SetResetPin, SparkFunDmxParamsConst::kResetPin), //
        MakeKey(SetBusyPin, SparkFunDmxParamsConst::kBusyPin)    //
    };

    uint32_t motor_index_{common::store::l6470dmx::kMaxMotors};
    char file_name_[16];

    inline static common::store::l6470dmx::SparkFun store_sparkfun;

    friend class JsonParamsBase<SparkFunDmxParams>;
};
} // namespace json

#endif  // JSON_SPARKFUNDMXPARAMS_H_
