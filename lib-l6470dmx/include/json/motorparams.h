/**
 * @file motorparams.h
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

#ifndef JSON_MOTORPARAMS_H_
#define JSON_MOTORPARAMS_H_

#include <cstdint>
#include "configurationstore.h"
#include "json/motorparamsconst.h"
#include "json/json_key.h"
#include "json/json_params_base.h"
#include "l6470dmxmodes.h"

namespace json
{
class MotorParams : public JsonParamsBase<MotorParams>
{
   public:
    explicit MotorParams(uint32_t motor_index);

    MotorParams(const MotorParams&) = delete;
    MotorParams& operator=(const MotorParams&) = delete;

    MotorParams(MotorParams&&) = delete;
    MotorParams& operator=(MotorParams&&) = delete;

    void Load()
    {
        if (motor_index_ < common::store::l6470dmx::kMaxMotors)
        {
            JsonParamsBase::Load(file_name_);
        }
    }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set(L6470* l6470);
    
   protected:
    void Dump();

   private:
    static void SetStepAngel(const char* val, uint32_t len);
    static void SetVoltage(const char* val, uint32_t len);
    static void SetCurrent(const char* val, uint32_t len);
    static void SetResistance(const char* val, uint32_t len);
	static void SetInductance(const char* val, uint32_t len);

    static constexpr Key kMotorKeys[] = {
		MakeKey(SetStepAngel, MotorParamsConst::kStepAngel), 
		MakeKey(SetVoltage, MotorParamsConst::kVoltage),                                        
		MakeKey(SetCurrent, MotorParamsConst::kCurrent),
		MakeKey(SetResistance, MotorParamsConst::kResistance),
		MakeKey(SetInductance, MotorParamsConst::kInductance)
	};

    uint32_t motor_index_{common::store::l6470dmx::kMaxMotors};
    char file_name_[16];

    inline static common::store::l6470dmx::Motor store_motor;

    friend class JsonParamsBase<MotorParams>;
};
} // namespace json

#endif  // JSON_MOTORPARAMS_H_
