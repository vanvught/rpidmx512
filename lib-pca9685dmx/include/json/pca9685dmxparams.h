/**
 * @file pca9685dmxparams.h
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

 #include <cstdint>
 
#include "configurationstore.h"
#include "json/json_key.h"
#include "json/json_params_base.h"
#include "json/pca9685dmxparamsconst.h"

namespace json
{
class Pca9685DmxParams : public JsonParamsBase<Pca9685DmxParams>
{
   public:
    Pca9685DmxParams();

    Pca9685DmxParams(const Pca9685DmxParams&) = delete;
    Pca9685DmxParams& operator=(const Pca9685DmxParams&) = delete;

    Pca9685DmxParams(Pca9685DmxParams&&) = delete;
    Pca9685DmxParams& operator=(Pca9685DmxParams&&) = delete;

    void Load() { JsonParamsBase::Load(Pca9685DmxParamsConst::kFileName); }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set();

   protected:
    void Dump();

   private:
    static void SetMode(const char* val, uint32_t len);

    static constexpr json::Key kPca9685DmxKeys[] = {
        MakeKey(SetMode, Pca9685DmxParamsConst::kMode),

    };

    inline static common::store::DmxPwm store_dmxpwm;

    friend class JsonParamsBase<Pca9685DmxParams>;
};
} // namespace json 