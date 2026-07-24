/**
 * @file displayudfshowe131.cpp
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <algorithm>

#include "displayudf.h"
#include "e131bridge.h"
#include "dmxnode.h"
#include "displayudf_debug.h"
#include "dmxnode_outputtype.h"
#if defined(DMXNODE_OUTPUT_DMX)
#include "dmx.h"
#endif

void DisplayUdf::ShowE131Bridge() {
#if defined(DMX_MAX_PORTS)
    DISPLAYUDF_DEBUG_ENTRY();
    DISPLAYUDF_DEBUG_PRINTF("dmxnode::kDmxportOffset=%u", dmxnode::kDmxportOffset);

    if constexpr (dmxnode::kConfigPortCount != 0) {
        auto* e131 = E131Bridge::Get();

        Printf(labels_[static_cast<uint32_t>(displayudf::Labels::kAp)], "AP: %d", e131->GetActiveOutputPorts() + e131->GetActiveInputPorts());

        for (uint32_t config_port_index = 0; config_port_index < dmxnode::kConfigPortCount; config_port_index++) {
            const auto kPortIndex = config_port_index + dmxnode::kDmxportOffset;

            if (kPortIndex >= std::min(static_cast<uint32_t>(4), dmxnode::kMaxPorts)) {
                break;
            }

            const auto kLabelIndex = static_cast<uint32_t>(displayudf::Labels::kUniversePortA) + config_port_index;

            if (kLabelIndex != 0xFF) {
                uint16_t n_universe;
                if (e131->GetUniverse(kPortIndex, n_universe, dmxnode::Direction::kOutput)) {
                    Printf(labels_[kLabelIndex], "Port %c: %d %s", ('A' + config_port_index), n_universe, dmxnode::GetMergeMode(e131->GetMergeMode(kPortIndex), true));
                }
            }
        }
    }

    DISPLAYUDF_DEBUG_EXIT();
#endif
}
