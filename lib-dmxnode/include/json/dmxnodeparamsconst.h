/**
 * @file dmxnodeparamsconst.h
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

#ifndef JSON_DMXNODEPARAMSCONST_H_
#define JSON_DMXNODEPARAMSCONST_H_

#include "json/json_key.h"
#include "common/utils/utils_hash.h"
#include "dmxnode_outputtype.h"
#if defined(DMXNODE_OUTPUT_DMX)
#include "dmx.h"
#endif

namespace json {
struct DmxNodeParamsConst {
    static constexpr char kFileName[] = "dmxnode.json";

    static constexpr auto kNodeName = json::MakeSimpleKey("node_name");
    static constexpr auto kFailsafe = json::MakeSimpleKey("failsafe");
    static constexpr auto kDisableMergeTimeout = json::MakeSimpleKey("disable_merge_timeout");
    static constexpr auto kDmxStartAddress = json::MakeSimpleKey("dmx_start_address");
    static constexpr auto kDmxSlotInfo = json::MakeSimpleKey("dmx_slot_info");

#if defined(DMX_MAX_PORTS)
    static constexpr json::PortKey kLabelPortA{"label_port_a", 12, Fnv1a32("label_port_a", 12)};
#if (DMX_MAX_PORTS > 1)
    static constexpr json::PortKey kLabelPortB{"label_port_b", 12, Fnv1a32("label_port_b", 12)};
#endif
#if (DMX_MAX_PORTS > 2)
    static constexpr json::PortKey kLabelPortC{"label_port_c", 12, Fnv1a32("label_port_c", 12)};
#endif
#if (DMX_MAX_PORTS == 4)
    static constexpr json::PortKey kLabelPortD{"label_port_d", 12, Fnv1a32("label_port_d", 12)};
#endif

    static constexpr json::PortKey kLabelPort[] = {
        kLabelPortA,
#if (DMX_MAX_PORTS > 1)
        kLabelPortB,
#endif
#if (DMX_MAX_PORTS > 2)
        kLabelPortC,
#endif
#if (DMX_MAX_PORTS == 4)
        kLabelPortD,
#endif
    };

    static constexpr json::PortKey kUniversePortA{"universe_port_a", 15, Fnv1a32("universe_port_a", 15)};
#if (DMX_MAX_PORTS > 1)
    static constexpr json::PortKey kUniversePortB{"universe_port_b", 15, Fnv1a32("universe_port_b", 15)};
#endif
#if (DMX_MAX_PORTS > 2)
    static constexpr json::PortKey kUniversePortC{"universe_port_c", 15, Fnv1a32("universe_port_c", 15)};
#endif
#if (DMX_MAX_PORTS == 4)
    static constexpr json::PortKey kUniversePortD{"universe_port_d", 15, Fnv1a32("universe_port_d", 15)};
#endif

    static constexpr json::PortKey kUniversePort[] = {
        kUniversePortA,
#if (DMX_MAX_PORTS > 1)
        kUniversePortB,
#endif
#if (DMX_MAX_PORTS > 2)
        kUniversePortC,
#endif
#if (DMX_MAX_PORTS == 4)
        kUniversePortD,
#endif
    };

    static constexpr json::PortKey kDirectionPortA{"direction_port_a", 16, Fnv1a32("direction_port_a", 16)};
#if (DMX_MAX_PORTS > 1)
    static constexpr json::PortKey kDirectionPortB{"direction_port_b", 16, Fnv1a32("direction_port_b", 16)};
#endif
#if (DMX_MAX_PORTS > 2)
    static constexpr json::PortKey kDirectionPortC{"direction_port_c", 16, Fnv1a32("direction_port_c", 16)};
#endif
#if (DMX_MAX_PORTS == 4)
    static constexpr json::PortKey kDirectionPortD{"direction_port_d", 16, Fnv1a32("direction_port_d", 16)};
#endif

    static constexpr json::PortKey kDirectionPort[] = {
        kDirectionPortA,
#if (DMX_MAX_PORTS > 1)
        kDirectionPortB,
#endif
#if (DMX_MAX_PORTS > 2)
        kDirectionPortC,
#endif
#if (DMX_MAX_PORTS == 4)
        kDirectionPortD,
#endif
    };

    static constexpr json::PortKey kMergeModePortA{"merge_mode_port_a", 17, Fnv1a32("merge_mode_port_a", 17)};
#if (DMX_MAX_PORTS > 1)
    static constexpr json::PortKey kMergeModePortB{"merge_mode_port_b", 17, Fnv1a32("merge_mode_port_b", 17)};
#endif
#if (DMX_MAX_PORTS > 2)
    static constexpr json::PortKey kMergeModePortC{"merge_mode_port_c", 17, Fnv1a32("merge_mode_port_c", 17)};
#endif
#if (DMX_MAX_PORTS == 4)
    static constexpr json::PortKey kMergeModePortD{"merge_mode_port_d", 17, Fnv1a32("merge_mode_port_d", 17)};
#endif

    static constexpr json::PortKey kMergeModePort[] = {
        kMergeModePortA,
#if (DMX_MAX_PORTS > 1)
        kMergeModePortB,
#endif
#if (DMX_MAX_PORTS > 2)
        kMergeModePortC,
#endif
#if (DMX_MAX_PORTS == 4)
        kMergeModePortD,
#endif
    };

    static constexpr json::PortKey kOutputStylePortA{"output_style_a", 14, Fnv1a32("output_style_a", 14)};
#if (DMX_MAX_PORTS > 1)
    static constexpr json::PortKey kOutputStylePortB{"output_style_b", 14, Fnv1a32("output_style_b", 14)};
#endif
#if (DMX_MAX_PORTS > 2)
    static constexpr json::PortKey kOutputStylePortC{"output_style_c", 14, Fnv1a32("output_style_c", 14)};
#endif
#if (DMX_MAX_PORTS == 4)
    static constexpr json::PortKey kOutputStylePortD{"output_style_d", 14, Fnv1a32("output_style_d", 14)};
#endif

    static constexpr json::PortKey kOutputStylePort[] = {
        kOutputStylePortA,
#if (DMX_MAX_PORTS > 1)
        kOutputStylePortB,
#endif
#if (DMX_MAX_PORTS > 2)
        kOutputStylePortC,
#endif
#if (DMX_MAX_PORTS == 4)
        kOutputStylePortD,
#endif
    };
#endif
};
} // namespace json

#endif // JSON_DMXNODEPARAMSCONST_H_
