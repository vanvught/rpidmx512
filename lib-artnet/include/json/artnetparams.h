/**
 * @file artnetparams.h
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org */

#ifndef JSON_ARTNETPARAMS_H_
#define JSON_ARTNETPARAMS_H_

#include "configurationstore.h"
#include "json/artnetparamsconst.h"
#include "json/json_key.h"
#include "json/json_params_base.h"
#include "dmxnode_outputtype.h"
#if defined(DMXNODE_OUTPUT_DMX)
#include "dmx.h"
#endif

namespace json {
class ArtNetParams : public JsonParamsBase<ArtNetParams> {
   public:
    ArtNetParams();

    ArtNetParams(const ArtNetParams&) = delete;
    ArtNetParams& operator=(const ArtNetParams&) = delete;

    ArtNetParams(ArtNetParams&&) = delete;
    ArtNetParams& operator=(ArtNetParams&&) = delete;

    void Load() { JsonParamsBase::Load(json::ArtNetParamsConst::kFileName); }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set();

   protected:
    void Dump();

   private:
    static void SetEnableRdm(const char* val, uint32_t len);
    static void SetMapUniverse0(const char* val, uint32_t len);
    static void SetDestinationIpPort(const char* key, uint32_t key_len, const char* val, uint32_t val_len);
    static void SetProtocolPort(const char* key, uint32_t key_len, const char* val, uint32_t val_len);
    static void SetRdmEnablePort(const char* key, uint32_t key_len, const char* val, uint32_t val_len);

    static constexpr json::Key kArtNetKeys[] = {
        MakeKey(SetMapUniverse0, ArtNetParamsConst::kMapUniverse0),
#if defined(RDM_CONTROLLER) || defined(RDM_RESPONDER)
        MakeKey(SetEnableRdm, ArtNetParamsConst::kEnableRdm),
#if defined(DMX_MAX_PORTS)
        MakeKey(SetRdmEnablePort, ArtNetParamsConst::kRdmEnablePort[0]),
#if (DMX_MAX_PORTS > 1)
        MakeKey(SetRdmEnablePort, ArtNetParamsConst::kRdmEnablePort[1]),
#endif
#if (DMX_MAX_PORTS > 2)
        MakeKey(SetRdmEnablePort, ArtNetParamsConst::kRdmEnablePort[2]),
#endif
#if (DMX_MAX_PORTS == 4)
        MakeKey(SetRdmEnablePort, ArtNetParamsConst::kRdmEnablePort[3]),
#endif
#endif
#endif
#if defined(DMX_MAX_PORTS)
        MakeKey(SetDestinationIpPort, ArtNetParamsConst::kDestinationIpPort[0]),
        MakeKey(SetProtocolPort, ArtNetParamsConst::kProtocolPort[0]),
#if (DMX_MAX_PORTS > 1)
        MakeKey(SetDestinationIpPort, ArtNetParamsConst::kDestinationIpPort[1]),
        MakeKey(SetProtocolPort, ArtNetParamsConst::kProtocolPort[1]),
#endif
#if (DMX_MAX_PORTS > 2)
        MakeKey(SetDestinationIpPort, ArtNetParamsConst::kDestinationIpPort[2]),
        MakeKey(SetProtocolPort, ArtNetParamsConst::kProtocolPort[2]),
#endif
#if (DMX_MAX_PORTS == 4)
        MakeKey(SetDestinationIpPort, ArtNetParamsConst::kDestinationIpPort[3]),
        MakeKey(SetProtocolPort, ArtNetParamsConst::kProtocolPort[3]),
#endif
#endif
    };

    inline static common::store::DmxNode store_dmxnode;

    friend class JsonParamsBase<ArtNetParams>;
};
} // namespace json

#endif // JSON_ARTNETPARAMS_H_
