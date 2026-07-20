/**
 * @file e131params.h
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org */

#ifndef JSON_E131PARAMS_H_
#define JSON_E131PARAMS_H_

#include "configurationstore.h"
#include "json/e131paramsconst.h"
#include "json/json_key.h"
#include "json/json_params_base.h"
#include "dmxnode_outputtype.h"
#if defined(DMXNODE_OUTPUT_DMX)
#include "dmx.h"
#endif

namespace json {
class E131Params : public JsonParamsBase<E131Params> {
   public:
    E131Params();

    E131Params(const E131Params&) = delete;
    E131Params& operator=(const E131Params&) = delete;

    E131Params(E131Params&&) = delete;
    E131Params& operator=(E131Params&&) = delete;

    void Load() { JsonParamsBase::Load(json::E131ParamsConst::kFileName); }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set();

   protected:
    void Dump();

   private:
    static void SetPriority(const char* key, uint32_t key_len, const char* val, uint32_t val_len);
#if defined(DMX_MAX_PORTS)
    static constexpr json::Key kE131PriorityKeys[] = {

        MakeKey(SetPriority, E131ParamsConst::kPriorityPort[0]),
#if (DMX_MAX_PORTS > 1)
        MakeKey(SetPriority, E131ParamsConst::kPriorityPort[1]),
#endif
#if (DMX_MAX_PORTS > 2)
        MakeKey(SetPriority, E131ParamsConst::kPriorityPort[2]),
#endif
#if (DMX_MAX_PORTS == 4)
        MakeKey(SetPriority, E131ParamsConst::kPriorityPort[3]),
#endif

    };
#endif

    inline static common::store::DmxNode store_dmxnode;

    friend class JsonParamsBase<E131Params>;
};
} // namespace json

#endif // JSON_E131PARAMS_H_
