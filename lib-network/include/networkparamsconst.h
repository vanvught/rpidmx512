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
	static const char FILE_NAME[];

	static const char USE_DHCP[];
	static const char DHCP_RETRY_TIME[];

	static const char IP_ADDRESS[];
	static const char NET_MASK[];
	static const char DEFAULT_GATEWAY[];
	static const char HOSTNAME[];

	static const char NTP_SERVER[];

#if defined (ESP8266)
	static const char NAME_SERVER[];

	static const char SSID[];
	static const char PASSWORD[];
#endif
};

#endif /* NETWORKPARAMSCONST_H_ */
