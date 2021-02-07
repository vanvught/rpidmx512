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

#ifndef NETWORKCONST_H_
#define NETWORKCONST_H_

struct NetworkConst {
	static const char PARAMS_FILE_NAME[];
	static const char PARAMS_USE_DHCP[];
	static const char PARAMS_IP_ADDRESS[];
	static const char PARAMS_NET_MASK[];
	static const char PARAMS_HOSTNAME[];

	static const char PARAMS_NTP_SERVER[];
	static const char PARAMS_NTP_UTC_OFFSET[];

#if !defined (H3)
	static const char PARAMS_DEFAULT_GATEWAY[];
	static const char PARAMS_NAME_SERVER[];
#endif

	static const char MSG_NETWORK_INIT[];
};

#endif /* NETWORKCONST_H_ */
