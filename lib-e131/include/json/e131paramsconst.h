/**
 * @file e131paramsconst.h
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

#ifndef JSON_E131PARAMSCONST_H_
#define JSON_E131PARAMSCONST_H_

#include "dmxnode_outputtype.h"
#if defined(DMXNODE_OUTPUT_DMX)
#include "json/json_key.h"
#include "common/utils/utils_hash.h"
#include "dmx.h"
#endif

namespace json {
struct E131ParamsConst {
    static constexpr char kFileName[] = "e131.json";

#if defined(DMX_MAX_PORTS)
    static constexpr json::PortKey kPriorityPortA{"priority_port_a", 15, Fnv1a32("priority_port_a", 15)};
#if (DMX_MAX_PORTS > 1)
    static constexpr json::PortKey kPriorityPortB{"priority_port_b", 15, Fnv1a32("priority_port_b", 15)};
#endif
#if (DMX_MAX_PORTS > 2)
    static constexpr json::PortKey kPriorityPortC{"priority_port_c", 15, Fnv1a32("priority_port_c", 15)};
#endif
#if (DMX_MAX_PORTS == 4)
    static constexpr json::PortKey kPriorityPortD{"priority_port_d", 15, Fnv1a32("priority_port_d", 15)};
#endif

    static constexpr json::PortKey kPriorityPort[] = {
        kPriorityPortA,
#if (DMX_MAX_PORTS > 1)
        kPriorityPortB,
#endif
#if (DMX_MAX_PORTS > 2)
        kPriorityPortC,
#endif
#if (DMX_MAX_PORTS == 4)
        kPriorityPortD,
#endif
    };
#endif
};
} // namespace json

#endif // JSON_E131PARAMSCONST_H_
