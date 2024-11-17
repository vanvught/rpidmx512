/**
 * @file networkparamsconst.h
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NETWORKPARAMSCONST_H_
#define NETWORKPARAMSCONST_H_

struct NetworkParamsConst {
	static inline const char FILE_NAME[] = "network.txt";

	static inline const char USE_DHCP[] = "use_dhcp";

	static inline const char IP_ADDRESS[] = "ip_address";
	static inline const char NET_MASK[] = "net_mask";
	static inline const char DEFAULT_GATEWAY[] = "default_gateway";
	static inline const char HOSTNAME[] = "hostname";

	static inline const char NTP_SERVER[] = "ntp_server";

#if defined (ESP8266)
	static inline const char NAME_SERVER[] = "name_server";

	static inline const char SSID[] = "ssid";
	static inline const char PASSWORD[] = "password";
#endif
};

#endif /* NETWORKPARAMSCONST_H_ */
