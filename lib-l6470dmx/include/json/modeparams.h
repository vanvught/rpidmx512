/**
 * @file modeparams.h
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

#ifndef JSON_MODEPARAMS_H_
#define JSON_MODEPARAMS_H_

#include <cstdint>
#include "configurationstore.h"
#include "json/modeparamsconst.h"
#include "json/json_key.h"
#include "json/json_params_base.h"
#include "json/dmxnodeparamsconst.h"

namespace json
{
class ModeParams : public JsonParamsBase<ModeParams>
{
   public:
    explicit ModeParams(uint32_t motor_index);

    ModeParams(const ModeParams&) = delete;
    ModeParams& operator=(const ModeParams&) = delete;

    ModeParams(ModeParams&&) = delete;
    ModeParams& operator=(ModeParams&&) = delete;

    void Load()
    {
        if (motor_index_ < common::store::l6470dmx::kMaxMotors)
        {
            JsonParamsBase::Load(file_name_);
        }
    }
    void Store(const char* buffer, uint32_t buffer_size);
    
   protected:
    void Dump();

   private:
    static void SetDmxMode(const char* val, uint32_t len);
    static void SetDmxStartAddress(const char* val, uint32_t len);
    static void SetMaxSteps(const char* val, uint32_t len);
    static void SetSwitchAct(const char* val, uint32_t len);
    static void SetSwitchDir(const char* val, uint32_t len);
    static void SetSwitchSps(const char* val, uint32_t len);
    static void SetSwitch(const char* val, uint32_t len);

    static constexpr Key kModeKeys[] = {
		MakeKey(SetDmxMode, ModeParamsConst::kDmxMode), 
		MakeKey(SetDmxStartAddress, DmxNodeParamsConst::kDmxStartAddress), 
		MakeKey(SetMaxSteps, ModeParamsConst::kMaxSteps),                                        
		MakeKey(SetSwitchAct, ModeParamsConst::kSwitchAct),
		MakeKey(SetSwitchDir, ModeParamsConst::kSwitchDir),
		MakeKey(SetSwitchSps, ModeParamsConst::kSwitchSps),
		MakeKey(SetSwitch, ModeParamsConst::kSwitch)
	};

    uint32_t motor_index_{common::store::l6470dmx::kMaxMotors};
    char file_name_[16];

    inline static common::store::l6470dmx::Mode store_mode;

    friend class JsonParamsBase<ModeParams>;
};
} // namespace json

#endif  // JSON_MODEPARAMS_H_
