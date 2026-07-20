/**
 * @file artnetparamsconst.h
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef JSON_ARTNETPARAMSCONST_H_
#define JSON_ARTNETPARAMSCONST_H_

#include "common/utils/utils_hash.h"
#include "json/json_key.h"
#include "dmxnode_outputtype.h"
#if defined(DMXNODE_OUTPUT_DMX)
#include "dmx.h"
#endif

namespace json {
struct ArtNetParamsConst {
    static constexpr char kFileName[] = "artnet.json";
    static constexpr auto kMapUniverse0 = json::MakeSimpleKey("map_universe0");

#if defined(RDM_CONTROLLER) || defined(RDM_RESPONDER)
    static constexpr auto kEnableRdm = json::MakeSimpleKey("enable_rdm");

    static constexpr json::PortKey kRdmEnablePortA{"rdm_enable_port_a", 17, Fnv1a32("rdm_enable_port_a", 17)};
#if (DMX_MAX_PORTS > 1)
    static constexpr json::PortKey kRdmEnablePortB{"rdm_enable_port_b", 17, Fnv1a32("rdm_enable_port_b", 17)};
#endif
#if (DMX_MAX_PORTS > 2)
    static constexpr json::PortKey kRdmEnablePortC{"rdm_enable_port_c", 17, Fnv1a32("rdm_enable_port_c", 17)};
#endif
#if (DMX_MAX_PORTS == 4)
    static constexpr json::PortKey kRdmEnablePortD{"rdm_enable_port_d", 17, Fnv1a32("rdm_enable_port_d", 17)};
#endif

    static constexpr json::PortKey kRdmEnablePort[] = {
        kRdmEnablePortA,
#if (DMX_MAX_PORTS > 1)
        kRdmEnablePortB,
#endif
#if (DMX_MAX_PORTS > 2)
        kRdmEnablePortC,
#endif
#if (DMX_MAX_PORTS == 4)
        kRdmEnablePortD,
#endif
    };
#endif

#if defined(DMX_MAX_PORTS)
    static constexpr json::PortKey kDestinationIpPortA{"destination_ip_port_a", 21, Fnv1a32("destination_ip_port_a", 21)};
#if (DMX_MAX_PORTS > 1)
    static constexpr json::PortKey kDestinationIpPortB{"destination_ip_port_b", 21, Fnv1a32("destination_ip_port_b", 21)};
#endif
#if (DMX_MAX_PORTS > 2)
    static constexpr json::PortKey kDestinationIpPortC{"destination_ip_port_c", 21, Fnv1a32("destination_ip_port_c", 21)};
#endif
#if (DMX_MAX_PORTS == 4)
    static constexpr json::PortKey kDestinationIpPortD{"destination_ip_port_d", 21, Fnv1a32("destination_ip_port_d", 21)};
#endif

    static constexpr json::PortKey kDestinationIpPort[] = {
        kDestinationIpPortA,
#if (DMX_MAX_PORTS > 1)
        kDestinationIpPortB,
#endif
#if (DMX_MAX_PORTS > 2)
        kDestinationIpPortC,
#endif
#if (DMX_MAX_PORTS == 4)
        kDestinationIpPortD,
#endif
    };

    static constexpr json::PortKey kProtocolPortA{"protocol_port_a", 15, Fnv1a32("protocol_port_a", 15)};
#if (DMX_MAX_PORTS > 1)
    static constexpr json::PortKey kProtocolPortB{"protocol_port_b", 15, Fnv1a32("protocol_port_b", 15)};
#endif
#if (DMX_MAX_PORTS > 2)
    static constexpr json::PortKey kProtocolPortC{"protocol_port_c", 15, Fnv1a32("protocol_port_c", 15)};
#endif
#if (DMX_MAX_PORTS == 4)
    static constexpr json::PortKey kProtocolPortD{"protocol_port_d", 15, Fnv1a32("protocol_port_d", 15)};
#endif

    static constexpr json::PortKey kProtocolPort[] = {
        kProtocolPortA,
#if (DMX_MAX_PORTS > 1)
        kProtocolPortB,
#endif
#if (DMX_MAX_PORTS > 2)
        kProtocolPortC,
#endif
#if (DMX_MAX_PORTS == 4)
        kProtocolPortD,
#endif
    };

    static constexpr json::PortKey kBgDiscoveryPortA{"bg_discovery_port_a", 19, Fnv1a32("bg_discovery_port_a", 19)};
#if (DMX_MAX_PORTS > 1)
    static constexpr json::PortKey kBgDiscoveryPortB{"bg_discovery_port_b", 19, Fnv1a32("bg_discovery_port_b", 19)};
#endif
#if (DMX_MAX_PORTS > 2)
    static constexpr json::PortKey kBgDiscoveryPortC{"bg_discovery_port_c", 19, Fnv1a32("bg_discovery_port_c", 19)};
#endif
#if (DMX_MAX_PORTS == 4)
    static constexpr json::PortKey kBgDiscoveryPortD{"bg_discovery_port_d", 19, Fnv1a32("bg_discovery_port_d", 19)};
#endif

    static constexpr json::PortKey kBgDiscoveryPort[] = {
        kBgDiscoveryPortA,
#if (DMX_MAX_PORTS > 1)
        kBgDiscoveryPortB,
#endif
#if (DMX_MAX_PORTS > 2)
        kBgDiscoveryPortC,
#endif
#if (DMX_MAX_PORTS == 4)
        kBgDiscoveryPortD,
#endif
    };
#endif
};
} // namespace json

#endif // JSON_ARTNETPARAMSCONST_H_
