/**
 * @file tlc59711dmxparams.h
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
 

#ifndef JSON_TLC59711DMXPARAMS_H_
#define JSON_TLC59711DMXPARAMS_H_

#include "configurationstore.h"
#include "json/json_key.h"
#include "json/dmxledparamsconst.h"
#include "json/dmxnodeparamsconst.h"
#include "json/json_params_base.h"

namespace json
{
class Tlc59711DmxParams : public JsonParamsBase<Tlc59711DmxParams>
{
   public:
    Tlc59711DmxParams();

    Tlc59711DmxParams(const Tlc59711DmxParams&) = delete;
    Tlc59711DmxParams& operator=(const Tlc59711DmxParams&) = delete;

    Tlc59711DmxParams(Tlc59711DmxParams&&) = delete;
    Tlc59711DmxParams& operator=(Tlc59711DmxParams&&) = delete;

    void Load() { JsonParamsBase::Load(json::DmxLedParamsConst::kFileName); }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set();

   protected:
    void Dump();

   private:
    static void SetType(const char* val, uint32_t len);
    static void SetCount(const char* val, uint32_t len);
    static void SetSpiSpeedHz(const char* val, uint32_t len);
	static void SetDmxStartAddress(const char* val, uint32_t len);


    static constexpr json::Key kPixelDmxKeys[] = {
		MakeKey(SetType, DmxLedParamsConst::kType), 
		MakeKey(SetCount, DmxLedParamsConst::kCount),
		MakeKey(SetDmxStartAddress, DmxNodeParamsConst::kDmxStartAddress),
		MakeKey(SetSpiSpeedHz, DmxLedParamsConst::kSpiSpeedHz)
    };

    inline static common::store::DmxLed store_dmxled;

    friend class JsonParamsBase<Tlc59711DmxParams>;
};
} // namespace json

#endif  // JSON_TLC59711DMXPARAMS_H_
