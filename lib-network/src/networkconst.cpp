/**
 * @file networkconst.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "networkconst.h"

const char NetworkConst::PARAMS_FILE_NAME[] = "network.txt";

const char NetworkConst::PARAMS_USE_DHCP[] = "use_dhcp";
const char NetworkConst::PARAMS_IP_ADDRESS[] = "ip_address";
const char NetworkConst::PARAMS_NET_MASK[] = "net_mask";
const char NetworkConst::PARAMS_HOSTNAME[] = "hostname";

const char NetworkConst::PARAMS_NTP_SERVER[] = "ntp_server";
const char NetworkConst::PARAMS_NTP_UTC_OFFSET[] = "ntp_utc_offset";

#if !defined (H3)
 const char NetworkConst::PARAMS_DEFAULT_GATEWAY[] = "default_gateway";
 const char NetworkConst::PARAMS_NAME_SERVER[] = "name_server";
#endif

const char NetworkConst::MSG_NETWORK_INIT[] = "Network init";
