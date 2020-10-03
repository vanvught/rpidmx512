/**
 * @file artnetparamsconst.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

const char ArtNetParamsConst::FILE_NAME[] = "artnet.txt";

const char ArtNetParamsConst::NET[] = "net";
const char ArtNetParamsConst::SUBNET[] = "subnet";

const char ArtNetParamsConst::TIMECODE[] = "use_timecode";
const char ArtNetParamsConst::TIMESYNC[] = "use_timesync";

const char ArtNetParamsConst::RDM[] = "enable_rdm";
const char ArtNetParamsConst::RDM_DISCOVERY[] = "rdm_discovery_at_startup";

const char ArtNetParamsConst::NODE_SHORT_NAME[] = "short_name";
const char ArtNetParamsConst::NODE_LONG_NAME[] = "long_name";
const char ArtNetParamsConst::NODE_MANUFACTURER_ID[] = "manufacturer_id";
const char ArtNetParamsConst::NODE_OEM_VALUE[] = "oem_value";
const char ArtNetParamsConst::NODE_NETWORK_DATA_LOSS_TIMEOUT[] = "network_data_loss_timeout";
const char ArtNetParamsConst::NODE_DISABLE_MERGE_TIMEOUT[] = "disable_merge_timeout";

const char ArtNetParamsConst::PROTOCOL[] = "protocol";
const char ArtNetParamsConst::PROTOCOL_PORT[ArtNet::MAX_PORTS][16] = { "protocol_port_a", "protocol_port_b", "protocol_port_c", "protocol_port_d" };
const char ArtNetParamsConst::DIRECTION[] = "direction";
const char ArtNetParamsConst::DESTINATION_IP_PORT[ArtNet::MAX_PORTS][24] = { "destination_ip_port_a", "destination_ip_port_b", "destination_ip_port_c", "destination_ip_port_d" };
