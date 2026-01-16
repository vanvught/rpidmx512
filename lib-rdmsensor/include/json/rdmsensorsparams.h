/**
 * @file rdmsensorsparams.h
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

#ifndef JSON_RDMSENSORSPARAMS_H_
#define JSON_RDMSENSORSPARAMS_H_

#include <cstdint>
#include "common/utils/utils_array.h"
#include "configurationstore.h"
#include "json/rdmsensorsparamsconst.h"
#include "json/json_key.h"
#include "json/json_params_base.h"

namespace json
{
class RdmSensorsParams : public JsonParamsBase<RdmSensorsParams>
{
   public:
    RdmSensorsParams();

    RdmSensorsParams(const RdmSensorsParams&) = delete;
    RdmSensorsParams& operator=(const RdmSensorsParams&) = delete;

    RdmSensorsParams(RdmSensorsParams&&) = delete;
    RdmSensorsParams& operator=(RdmSensorsParams&&) = delete;

    void Load() { JsonParamsBase::Load(RdmSensorsParamsConst::kFileName); }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set();

    size_t static constexpr KeysSize() { return common::ArraySize(kRdmSensorsKeys); }
    static constexpr auto& Keys() { return kRdmSensorsKeys; }

   protected:
    void Dump();

   private:
    static void SetBH170(const char* val, uint32_t len);
    static void SetHTU21D(const char* val, uint32_t len);
    static void SetINA219(const char* val, uint32_t len);
    static void SetMCP9808(const char* val, uint32_t len);
    static void SetSI7021(const char* val, uint32_t len);
    static void SetMCP3424(const char* val, uint32_t len);

    static constexpr Key kRdmSensorsKeys[] = {
		MakeKey(SetBH170, RdmSensorsParamsConst::kBH170), 
		MakeKey(SetHTU21D, RdmSensorsParamsConst::kHTU21D),
		MakeKey(SetINA219, RdmSensorsParamsConst::kINA219), 
		MakeKey(SetMCP9808, RdmSensorsParamsConst::kMCP9808),     
		MakeKey(SetSI7021, RdmSensorsParamsConst::kSI7021), 
		MakeKey(SetMCP3424, RdmSensorsParamsConst::kMCP3424)
    };

   private:
    inline static common::store::RdmSensors store_rdmsensors;

    friend class JsonParamsBase<RdmSensorsParams>;
};
} // namespace json

#endif  // JSON_RDMSENSORSPARAMS_H_
