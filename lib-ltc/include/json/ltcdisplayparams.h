/**
 * @file ltcdisplayparams.h
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

#ifndef JSON_LTCDISPLAYPARAMS_H_
#define JSON_LTCDISPLAYPARAMS_H_

#include "configurationstore.h"
#include "json/ltcdisplayparamsconst.h"
#include "json/json_key.h"
#include "json/json_params_base.h"

namespace json
{
class LtcDisplayParams : public JsonParamsBase<LtcDisplayParams>
{
   public:
    LtcDisplayParams();

    LtcDisplayParams(const LtcDisplayParams&) = delete;
    LtcDisplayParams& operator=(const LtcDisplayParams&) = delete;

    LtcDisplayParams(LtcDisplayParams&&) = delete;
    LtcDisplayParams& operator=(LtcDisplayParams&&) = delete;

    void Load() { JsonParamsBase::Load(LtcDisplayParamsConst::kFileName); }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set();

   protected:
    void Dump();

   private:
    static void SetOledIntensity(const char* val, uint32_t len);
    static void SetRotaryFullstep(const char* val, uint32_t len);
    static void SetMaX7219Type(const char* val, uint32_t len);
    static void SetMaX7219Intensity(const char* val, uint32_t len);
#if !defined(CONFIG_LTC_DISABLE_WS28XX)     
    static void SetPixelType(const char* val, uint32_t len);
#endif    
    static void SetInfoMsg(const char* val, uint32_t len);

    static constexpr json::Key kLtcDisplayKeys[] = 
    {
        MakeKey(SetOledIntensity, LtcDisplayParamsConst::kOledIntensity), 
        MakeKey(SetRotaryFullstep, LtcDisplayParamsConst::kRotaryFullstep),
        MakeKey(SetMaX7219Type, LtcDisplayParamsConst::kMax7219Type),   
        MakeKey(SetMaX7219Intensity, LtcDisplayParamsConst::kMax7219Intensity),
#if !defined(CONFIG_LTC_DISABLE_WS28XX) 
        MakeKey(SetPixelType, LtcDisplayParamsConst::kPixelType),     
#endif
        MakeKey(SetInfoMsg, LtcDisplayParamsConst::kInfoMsg)
    };

    inline static common::store::LtcDisplay store_ltcdisplay;

    friend class JsonParamsBase<LtcDisplayParams>;
};
} // namespace json

#endif  // JSON_LTCDISPLAYPARAMS_H_
