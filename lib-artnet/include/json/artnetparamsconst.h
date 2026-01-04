/**
 * @file artnetparamsconst.h
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org */

#ifndef JSON_ARTNETPARAMSCONST_H_
#define JSON_ARTNETPARAMSCONST_H_

#include "common/utils/utils_hash.h"
#include "json/json_key.h"

#undef MAX_ARRAY_SIZE
#if defined(DMXNODE_OUTPUT_DMX)
#include "dmx.h"
#define MAX_ARRAY_SIZE DMX_MAX_PORTS
#else
#define MAX_ARRAY_SIZE 4
#endif

static_assert(MAX_ARRAY_SIZE <= common::store::dmxnode::kParamPorts);

namespace json
{
struct ArtNetParamsConst
{
    static constexpr char kFileName[] = "artnet.json";

    static constexpr json::PortKey kDestinationIpPortA{"destination_ip_port_a", 21, Fnv1a32("destination_ip_port_a", 21)};
#if (MAX_ARRAY_SIZE > 1)
    static constexpr json::PortKey kDestinationIpPortB{"destination_ip_port_b", 21, Fnv1a32("destination_ip_port_b", 21)};
#endif
#if (MAX_ARRAY_SIZE > 2)
    static constexpr json::PortKey kDestinationIpPortC{"destination_ip_port_c", 21, Fnv1a32("destination_ip_port_c", 21)};
#endif
#if (MAX_ARRAY_SIZE == 4)
    static constexpr json::PortKey kDestinationIpPortD{"destination_ip_port_d", 21, Fnv1a32("destination_ip_port_d", 21)};
#endif

    static constexpr json::PortKey kDestinationIpPort[] = 
    {
        kDestinationIpPortA,
#if (MAX_ARRAY_SIZE > 1)
        kDestinationIpPortB,
#endif
#if (MAX_ARRAY_SIZE > 2)
        kDestinationIpPortC,
#endif
#if (MAX_ARRAY_SIZE == 4)
        kDestinationIpPortD,
#endif
	};
 
    // Art-Net 4
    static constexpr SimpleKey kEnableRdm 
    {
		"enable_rdm", 
		10, 
		Fnv1a32("enable_rdm", 10)
	};
    
    static constexpr SimpleKey kMapUniverse0 
    {
		"map_universe0", 
		13, 
		Fnv1a32("map_universe0", 13)
	};

    static constexpr json::PortKey kProtocolPortA{"protocol_port_a", 15, Fnv1a32("protocol_port_a", 15)};
#if (MAX_ARRAY_SIZE > 1)
    static constexpr json::PortKey kProtocolPortB{"protocol_port_b", 15, Fnv1a32("protocol_port_b", 15)};
#endif
#if (MAX_ARRAY_SIZE > 2)
    static constexpr json::PortKey kProtocolPortC{"protocol_port_c", 15, Fnv1a32("protocol_port_c", 15)};
#endif
#if (MAX_ARRAY_SIZE == 4)
    static constexpr json::PortKey kProtocolPortD{"protocol_port_d", 15, Fnv1a32("protocol_port_d", 15)};
#endif

    static constexpr json::PortKey kProtocolPort[] = 
    {
        kProtocolPortA,
#if (MAX_ARRAY_SIZE > 1)
        kProtocolPortB,
#endif
#if (MAX_ARRAY_SIZE > 2)
        kProtocolPortC,
#endif
#if (MAX_ARRAY_SIZE == 4)
        kProtocolPortD,
#endif
    };

    static constexpr json::PortKey kRdmEnablePortA{"rdm_enable_port_a", 17, Fnv1a32("rdm_enable_port_a", 17)};
#if (MAX_ARRAY_SIZE > 1)
    static constexpr json::PortKey kRdmEnablePortB{"rdm_enable_port_b", 17, Fnv1a32("rdm_enable_port_b", 17)};
#endif
#if (MAX_ARRAY_SIZE > 2)
    static constexpr json::PortKey kRdmEnablePortC{"rdm_enable_port_c", 17, Fnv1a32("rdm_enable_port_c", 17)};
#endif
#if (MAX_ARRAY_SIZE == 4)
    static constexpr json::PortKey kRdmEnablePortD{"rdm_enable_port_d", 17, Fnv1a32("rdm_enable_port_d", 17)};
#endif

    static constexpr json::PortKey kRdmEnablePort[] = {
        kRdmEnablePortA,
#if (MAX_ARRAY_SIZE > 1)
        kRdmEnablePortB,
#endif
#if (MAX_ARRAY_SIZE > 2)
        kRdmEnablePortC,
#endif
#if (MAX_ARRAY_SIZE == 4)
        kRdmEnablePortD,
#endif
    };
};
} // namespace json

#endif  // JSON_ARTNETPARAMSCONST_H_
