/**
 * @file rdmdeviceparams.h
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
 

#ifndef JSON_RDMDEVICEPARAMS_H_
#define JSON_RDMDEVICEPARAMS_H_

 #include "configurationstore.h"
#include "json/rdmdeviceparamsconst.h"
#include "json/json_key.h"
#include "json/json_params_base.h"

namespace json
{
class RdmDeviceParams : public JsonParamsBase<RdmDeviceParams>
{
   public:
    RdmDeviceParams();

    RdmDeviceParams(const RdmDeviceParams&) = delete;
    RdmDeviceParams& operator=(const RdmDeviceParams&) = delete;

    RdmDeviceParams(RdmDeviceParams&&) = delete;
    RdmDeviceParams& operator=(RdmDeviceParams&&) = delete;

    void Load() { JsonParamsBase::Load(RdmDeviceParamsConst::kFileName); }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set();

   protected:
    void Dump();

   private:
    static void SetLabel(const char* val, uint32_t len);
    static void SetProductCategory(const char* val, uint32_t len);
    static void SetProductDetail(const char* val, uint32_t len);

    static constexpr json::Key kRdmDeviceKeys[] = {
		MakeKey(SetLabel,RdmDeviceParamsConst::kLabel),
 		MakeKey(SetProductCategory,RdmDeviceParamsConst::kProductCategory),
  		MakeKey(SetProductDetail, RdmDeviceParamsConst::kProductDetail)
    };

    inline static common::store::RdmDevice store_rdmdevice;

    friend class JsonParamsBase<RdmDeviceParams>;
};
} // namespace json

#endif  // JSON_RDMDEVICEPARAMS_H_
