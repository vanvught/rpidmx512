/**
 * @file displayudfparams.h
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

#ifndef JSON_DISPLAYUDFPARAMS_H_
#define JSON_DISPLAYUDFPARAMS_H_

#include "configurationstore.h"
#include "json/displayudfparamsconst.h"
#include "json/json_key.h"
#include "json/json_params_base.h"
#include "displayudf.h"
#include "common/utils/utils_array.h"

static_assert(common::ArraySize(json::DisplayUdfParamsConst::kLabels) == static_cast<size_t>(displayudf::Labels::kUnknown), "Mismatch between enum and kArray");

namespace json
{
class DisplayUdfParams : public JsonParamsBase<DisplayUdfParams>
{
	public:

    DisplayUdfParams();

    DisplayUdfParams(const DisplayUdfParams&) = delete;
    DisplayUdfParams& operator=(const DisplayUdfParams&) = delete;

    DisplayUdfParams(DisplayUdfParams&&) = delete;
    DisplayUdfParams& operator=(DisplayUdfParams&&) = delete;

    void Load() { JsonParamsBase::Load(DisplayUdfParamsConst::kFileName); }
    void Store(const char* buffer, uint32_t buffer_size);
    void SetAndShow();

   protected:
    void Dump();

   private:
    static void SetIntensity(const char* val, uint32_t len);
    static void SetSleepTimeout(const char* val, uint32_t len);
    static void SetFlipVertically(const char* val, uint32_t len);
	static void SetLabel(const char* key, uint32_t key_len, const char* val, uint32_t val_len);

    static constexpr Key kDisplayUdfKeys[] = {
        MakeKey(SetIntensity, DisplayUdfParamsConst::kIntensity),           
        MakeKey(SetSleepTimeout, DisplayUdfParamsConst::kSleepTimeout),
        MakeKey(SetFlipVertically, DisplayUdfParamsConst::kFlipVertically), 
        MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[0]),
        MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[1]),        
        MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[2]),
        MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[3]),                
        MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[4]),   
        MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[5]),
        MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[6]),  
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[7]),   
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[8]),
#if defined (DMX_MAX_PORTS)
#if (DMX_MAX_PORTS == 1)	
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[9]),  
#if defined(NODE_ARTNET) || defined (NODE_ARTNET_MULTI)			
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[10]),  
#endif		
#endif		
#if (DMX_MAX_PORTS == 2)		
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[9]),  
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[10]),  
#if defined(NODE_ARTNET) || defined (NODE_ARTNET_MULTI)		
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[11]),  
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[12]),  
#endif			
#endif
#if (DMX_MAX_PORTS == 3)		
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[9]),  
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[10]),  
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[11]),  
#if defined(NODE_ARTNET) || defined (NODE_ARTNET_MULTI)		
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[12]),  
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[13]),  
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[14]),  
#endif			
#endif
#if (DMX_MAX_PORTS == 4)		
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[9]),  
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[10]),  
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[11]),  
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[12]),  
#if defined(NODE_ARTNET) || defined (NODE_ARTNET_MULTI)		
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[13]),  
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[14]),  
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[15]),  	
       MakeKey(SetLabel, DisplayUdfParamsConst::kLabels[16]),  
#endif			
#endif
#endif 
   	};
   	
   	static_assert((common::ArraySize(kDisplayUdfKeys) - 3) == static_cast<size_t>(displayudf::Labels::kUnknown), "Mismatch between enum and kArray");

    inline static common::store::DisplayUdf store_displayudf;

    friend class JsonParamsBase<DisplayUdfParams>;
};
} // namespace json

#endif  // JSON_DISPLAYUDFPARAMS_H_
