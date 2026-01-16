/**
 * @file modeparamsconst.h
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

#ifndef JSON_MODEPARAMSCONST_H_
#define JSON_MODEPARAMSCONST_H_

#include <cstdint>

#include "json/json_key.h"
#include "l6470dmxmodes.h"

namespace json
{
namespace dmxmode
{
[[nodiscard]] inline constexpr const char* GetSwitchAction(uint8_t action)
{
    return action == L6470_ABSPOS_COPY ? "copy" : "reset";
}

[[nodiscard]] inline constexpr const char* GetSwitchDir(uint8_t dir)
{
	return dir == L6470_DIR_FWD ? "forward" : "reverse";
}

} // namespace dmxmode

struct ModeParamsConst
{
	static constexpr json::SimpleKey kDmxMode {
	    "dmx_mode",
	    8,
	    Fnv1a32("dmx_mode", 8)
	};


	static constexpr json::SimpleKey kMaxSteps {
	    "mode_max_steps",
	    14,
	    Fnv1a32("mode_max_steps", 14)
	};

	static constexpr json::SimpleKey kSwitchAct {
	    "mode_switch_act",
	    15,
	    Fnv1a32("mode_switch_act", 15)
	};
	
	static constexpr json::SimpleKey kSwitchDir {
	    "mode_switch_dir",
	    16,
	    Fnv1a32("mode_switch_dir", 16)
	};

	static constexpr json::SimpleKey kSwitchSps {
	    "mode_switch_sps",
	    15,
	    Fnv1a32("mode_switch_sps", 15)
	};
	
	static constexpr json::SimpleKey kSwitch {
	    "mode_switch",
	    11,
	    Fnv1a32("mode_switch", 11)
	};
};
} // namespace json

#endif  // JSON_MODEPARAMSCONST_H_
