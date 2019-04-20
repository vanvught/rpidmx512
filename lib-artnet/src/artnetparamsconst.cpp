/**
 * @file artnetparamsconst.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "artnetparamsconst.h"
#include "artnet.h"

alignas(uint32_t) const char ArtNetParamsConst::FILE_NAME[] = "artnet.txt";
alignas(uint32_t) const char ArtNetParamsConst::NET[] = "net";												///< 0 {default}
alignas(uint32_t) const char ArtNetParamsConst::SUBNET[] = "subnet";										///< 0 {default}
alignas(uint32_t) const char ArtNetParamsConst::TIMECODE[] = "use_timecode";
alignas(uint32_t) const char ArtNetParamsConst::TIMESYNC[] = "use_timesync";
alignas(uint32_t) const char ArtNetParamsConst::RDM[] = "enable_rdm";										///< Enable RDM, 0 {default}
alignas(uint32_t) const char ArtNetParamsConst::RDM_DISCOVERY[] = "rdm_discovery_at_startup";				///< 0 {default}
alignas(uint32_t) const char ArtNetParamsConst::NODE_SHORT_NAME[] = "short_name";
alignas(uint32_t) const char ArtNetParamsConst::NODE_LONG_NAME[] = "long_name";
alignas(uint32_t) const char ArtNetParamsConst::NODE_MANUFACTURER_ID[] = "manufacturer_id";
alignas(uint32_t) const char ArtNetParamsConst::NODE_OEM_VALUE[] = "oem_value";
alignas(uint32_t) const char ArtNetParamsConst::NODE_NETWORK_DATA_LOSS_TIMEOUT[] = "network_data_loss_timeout";	///< 10 {default}
alignas(uint32_t) const char ArtNetParamsConst::NODE_DISABLE_MERGE_TIMEOUT[] = "disable_merge_timeout";			///< 0 {default}
alignas(uint32_t) const char ArtNetParamsConst::UNIVERSE_PORT[ARTNET_MAX_PORTS][16] = { "universe_port_a", "universe_port_b", "universe_port_c", "universe_port_d" };
alignas(uint32_t) const char ArtNetParamsConst::MERGE_MODE[] = "merge_mode";
alignas(uint32_t) const char ArtNetParamsConst::MERGE_MODE_PORT[ARTNET_MAX_PORTS][18] = { "merge_mode_port_a", "merge_mode_port_b", "merge_mode_port_c", "merge_mode_port_d" };
alignas(uint32_t) const char ArtNetParamsConst::PROTOCOL[] = "protocol";
alignas(uint32_t) const char ArtNetParamsConst::PROTOCOL_PORT[ARTNET_MAX_PORTS][16] = { "protocol_port_a", "protocol_port_b", "protocol_port_c", "protocol_port_d" };
