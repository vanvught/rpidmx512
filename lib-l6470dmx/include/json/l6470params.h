/**
 * @file l6470params.h
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

#ifndef JSON_L6470PARAMS_H_
#define JSON_L6470PARAMS_H_

#include <cstdint>

#include "configurationstore.h"
#include "json/l6470paramsconst.h"
#include "json/json_key.h"
#include "json/json_params_base.h"
#include "l6470.h"

namespace json
{
class L6470Params : public JsonParamsBase<L6470Params>
{
   public:
    explicit L6470Params(uint32_t motor_index);

    L6470Params(const L6470Params&) = delete;
    L6470Params& operator=(const L6470Params&) = delete;

    L6470Params(L6470Params&&) = delete;
    L6470Params& operator=(L6470Params&&) = delete;

    void Load()
    {
        if (motor_index_ < common::store::l6470dmx::kMaxMotors)
        {
            JsonParamsBase::Load(file_name_);
        }
    }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set(L6470*);
    
   protected:
    void Dump();

   private:
    static void SetMinSpeed(const char* val, uint32_t len);
    static void SetMaxSpeed(const char* val, uint32_t len);
    static void SetAcc(const char* val, uint32_t len);
    static void SetDec(const char* val, uint32_t len);
    static void SetKvalHold(const char* val, uint32_t len);
    static void SetKvalRun(const char* val, uint32_t len);
    static void SetKvalAcc(const char* val, uint32_t len);
    static void SetKvalDec(const char* val, uint32_t len);
    static void SetMicroSteps(const char* val, uint32_t len);

    static constexpr Key kL6470Keys[] = {
		MakeKey(SetMinSpeed, L6470ParamsConst::kMinSpeed), 
		MakeKey(SetMaxSpeed, L6470ParamsConst::kMaxSpeed),                                        
		MakeKey(SetAcc, L6470ParamsConst::kAcc),
		MakeKey(SetDec, L6470ParamsConst::kDec),
		MakeKey(SetKvalHold, L6470ParamsConst::kKvalHold),
		MakeKey(SetKvalRun, L6470ParamsConst::kKvalRun),
		MakeKey(SetKvalAcc, L6470ParamsConst::kKvalAcc),
		MakeKey(SetKvalDec, L6470ParamsConst::kKvalDec),
		MakeKey(SetMicroSteps, L6470ParamsConst::kMicroSteps)
	};

    uint32_t motor_index_{common::store::l6470dmx::kMaxMotors};
    char file_name_[16];

    inline static common::store::l6470dmx::L6470 store_l6470;

    friend class JsonParamsBase<L6470Params>;
};
} // namespace json

#endif  // JSON_L6470PARAMS_H_
