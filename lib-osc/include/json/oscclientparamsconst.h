/**
 * @file oscclientparamsconst.h
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
 

#ifndef JSON_OSCCLIENTPARAMSCONST_H_
#define JSON_OSCCLIENTPARAMSCONST_H_

#include "json/json_key.h"

namespace json
{
struct OscClientParamsConst
{
	static constexpr char kFileName[] = "oscclient.json";
	
	static constexpr json::SimpleKey kServerIp {
	    "server_ip",
	    9,
	    Fnv1a32("server_ip", 9)
	};
	
	static constexpr json::SimpleKey kPingDisable {
	    "ping_disable",
	    12,
	    Fnv1a32("ping_disable", 12)
	};
	
	static constexpr json::SimpleKey kPingDelay {
	    "ping_delay",
	    10,
	    Fnv1a32("ping_delay", 10)
	};

    static constexpr json::PortKey kCmd0{"cmd0", 4, Fnv1a32("cmd0", 4)};
    static constexpr json::PortKey kCmd1{"cmd1", 4, Fnv1a32("cmd1", 4)};
    static constexpr json::PortKey kCmd2{"cmd2", 4, Fnv1a32("cmd2", 4)};
    static constexpr json::PortKey kCmd3{"cmd3", 4, Fnv1a32("cmd3", 4)};
    static constexpr json::PortKey kCmd4{"cmd4", 4, Fnv1a32("cmd4", 4)};
    static constexpr json::PortKey kCmd5{"cmd5", 4, Fnv1a32("cmd5", 4)};
    static constexpr json::PortKey kCmd6{"cmd6", 4, Fnv1a32("cmd6", 4)};
    static constexpr json::PortKey kCmd7{"cmd7", 4, Fnv1a32("cmd7", 4)};

    static constexpr json::PortKey kCmd[] = 
    {
		kCmd0,
		kCmd1,
    	kCmd2,
    	kCmd3,
		kCmd4,
		kCmd5,
    	kCmd6,
    	kCmd7,
    };
    
    static constexpr json::PortKey kLed0{"led0", 4, Fnv1a32("led0", 4)};
    static constexpr json::PortKey kLed1{"led1", 4, Fnv1a32("led1", 4)};
    static constexpr json::PortKey kLed2{"led2", 4, Fnv1a32("led2", 4)};
    static constexpr json::PortKey kLed3{"led3", 4, Fnv1a32("led3", 4)};
    static constexpr json::PortKey kLed4{"led4", 4, Fnv1a32("led4", 4)};
    static constexpr json::PortKey kLed5{"led5", 4, Fnv1a32("led5", 4)};
    static constexpr json::PortKey kLed6{"led6", 4, Fnv1a32("led6", 4)};
    static constexpr json::PortKey kLed7{"led7", 4, Fnv1a32("led7", 4)};

    static constexpr json::PortKey kLed[] = 
    {
		kLed0,
		kLed1,
    	kLed2,
    	kLed3,
		kLed4,
		kLed5,
    	kLed6,
    	kLed7,
    };

};
} // namespace json

#endif  // JSON_OSCCLIENTPARAMSCONST_H_
