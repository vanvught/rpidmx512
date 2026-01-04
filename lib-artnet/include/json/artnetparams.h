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

#if !defined (MAX_ARRAY_SIZE)
#error
#endif

namespace json
{
class ArtNetParams : public JsonParamsBase<ArtNetParams>
{
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
        MakeKey(SetEnableRdm, ArtNetParamsConst::kEnableRdm),         
        MakeKey(SetMapUniverse0, ArtNetParamsConst::kMapUniverse0),
        MakeKey(SetDestinationIpPort, ArtNetParamsConst::kDestinationIpPort[0]),
        MakeKey(SetProtocolPort, ArtNetParamsConst::kProtocolPort[0]),
        MakeKey(SetRdmEnablePort, ArtNetParamsConst::kRdmEnablePort[0]),
#if (MAX_ARRAY_SIZE > 1)
        MakeKey(SetDestinationIpPort, ArtNetParamsConst::kDestinationIpPort[1]),
        MakeKey(SetProtocolPort, ArtNetParamsConst::kProtocolPort[1]),
        MakeKey(SetRdmEnablePort, ArtNetParamsConst::kRdmEnablePort[1]),
#endif
#if (MAX_ARRAY_SIZE > 2)
        MakeKey(SetDestinationIpPort, ArtNetParamsConst::kDestinationIpPort[2]),
        MakeKey(SetProtocolPort, ArtNetParamsConst::kProtocolPort[2]),
        MakeKey(SetRdmEnablePort, ArtNetParamsConst::kRdmEnablePort[2]),
#endif
#if (MAX_ARRAY_SIZE == 4)
        MakeKey(SetDestinationIpPort, ArtNetParamsConst::kDestinationIpPort[3]),
        MakeKey(SetProtocolPort, ArtNetParamsConst::kProtocolPort[3]),
        MakeKey(SetRdmEnablePort, ArtNetParamsConst::kRdmEnablePort[3]),
#endif
    };

    inline static common::store::DmxNode store_dmxnode_;

    friend class JsonParamsBase<ArtNetParams>;
};
} // namespace json

#endif  // JSON_ARTNETPARAMS_H_
