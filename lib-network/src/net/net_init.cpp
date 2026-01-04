/**
 * @file net_init.cpp
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

#if defined(DEBUG_NET_NET)
#if defined(NDEBUG)
#undef NDEBUG
#endif
#endif

#include "net/arp.h"
#include "net_private.h"
#if defined(CONFIG_NET_ENABLE_NTP_CLIENT) || defined(CONFIG_NET_ENABLE_PTP_NTP_CLIENT)
#include "net/apps/ntpclient.h"
#endif
#if !defined(CONFIG_NET_APPS_NO_MDNS)
#include "net/apps/mdns.h"
#endif

 #include "firmware/debug/debug_debug.h"

namespace net
{
#if defined(CONFIG_NET_ENABLE_PTP)
__attribute__((weak)) void ptp_init() {}
#endif

void NetInit()
{
    DEBUG_ENTRY();

    arp::Init();
    ip::Init();
    
#if defined(CONFIG_NET_ENABLE_PTP)
    ptp_init();
#endif

#if defined(CONFIG_NET_ENABLE_NTP_CLIENT)
    ntpclient::Init();
#endif

#if defined(CONFIG_NET_ENABLE_PTP_NTP_CLIENT)
    ntpclient::ptp::Init();
#endif

#if !defined(CONFIG_NET_APPS_NO_MDNS)
    mdns::Init();
#endif

    DEBUG_EXIT();
}
} // namespace net
