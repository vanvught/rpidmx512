/**
 * @file dmxmonitorparams.h
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org */

#ifndef JSON_DMXMONITORPARAMS_H_
#define JSON_DMXMONITORPARAMS_H_

#include "configurationstore.h"
#include "json/json_key.h"
#include "json/dmxmonitorparamsconst.h"
#include "json/json_params_base.h"

namespace json
{
class DmxMonitorParams : public JsonParamsBase<DmxMonitorParams>
{
   public:
    DmxMonitorParams();

    DmxMonitorParams(const DmxMonitorParams&) = delete;
    DmxMonitorParams& operator=(const DmxMonitorParams&) = delete;

    DmxMonitorParams(DmxMonitorParams&&) = delete;
    DmxMonitorParams& operator=(DmxMonitorParams&&) = delete;

    void Load() { JsonParamsBase::Load(json::DmxMonitorParamsConst::kFileName); }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set();

   protected:
    void Dump();

   private:
    static void SetDmxStartAddress(const char* val, uint32_t len);
    static void SetDmxMaxChannels(const char* val, uint32_t len);
    static void SetFormat(const char* val, uint32_t len);

    static constexpr json::Key kDmxMonitorKeys[] = 
    {
		MakeKey(SetDmxStartAddress, DmxMonitorParamsConst::kDmxStartAddress), 
		MakeKey(SetDmxMaxChannels, DmxMonitorParamsConst::kDmxMaxChannels),
        MakeKey(SetFormat, DmxMonitorParamsConst::kFormat)
    };

    inline static common::store::DmxMonitor store_dmxmonitor;

    friend class JsonParamsBase<DmxMonitorParams>;
};
} // namespace json

#endif  // JSON_DMXMONITORPARAMS_H_
