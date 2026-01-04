/**
 * @file rgbpanelparams.h
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

#ifndef JSON_RGBPANELPARAMS_H_
#define JSON_RGBPANELPARAMS_H_

#include "configurationstore.h"
#include "json/rgbpanelparamsconst.h"
#include "json/json_key.h"
#include "json/json_params_base.h"

namespace json
{
class RgbPanelParams : public JsonParamsBase<RgbPanelParams>
{
   public:
    RgbPanelParams();

    RgbPanelParams(const RgbPanelParams&) = delete;
    RgbPanelParams& operator=(const RgbPanelParams&) = delete;

    RgbPanelParams(RgbPanelParams&&) = delete;
    RgbPanelParams& operator=(RgbPanelParams&&) = delete;

    void Load() { JsonParamsBase::Load(RgbPanelParamsConst::kFileName); }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set();

   protected:
    void Dump();

   private:
    static void SetCols(const char* val, uint32_t len);
    static void SetRows(const char* val, uint32_t len);
    static void SetChain(const char* val, uint32_t len);
    static void SetType(const char* val, uint32_t len);

    static constexpr json::Key kRgbPanelKeys[] = {
		MakeKey(SetCols, RgbPanelParamsConst::kCols),
 		MakeKey(SetRows, RgbPanelParamsConst::kRows),
  		MakeKey(SetChain, RgbPanelParamsConst::kChain),
  		MakeKey(SetType, RgbPanelParamsConst::kType)
    };

    inline static common::store::RgbPanel store_rgbpanel;

    friend class JsonParamsBase<RgbPanelParams>;
};
} // namespace json

#endif  // JSON_RGBPANELPARAMS_H_
