/**
 * @file httpd.h
 *  @brief HTTP daemon class for managing HTTP server tasks.
 *
 * This class handles HTTP requests and integrates with the network and mDNS subsystems.
 * It uses placement new to construct and destruct request handlers explicitly.

 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef HTTPD_HTTPD_H_
#define HTTPD_HTTPD_H_

#include <cstdint>

#include "http.h"
#include "httpdhandlerequest.h"

#include "network.h"

#include "../../lib-network/config/net_config.h"

#include "debug.h"

/**
 * @class HttpDaemon
 * @brief A class that manages an HTTP server with support for multiple connections.
 *
 * The HttpDaemon class sets up an HTTP server, handles incoming requests, and integrates with mDNS.
 */
class HttpDaemon {
public:
	/**
	 * @brief Constructor for HttpDaemon.
	 *
	 * Initializes the HTTP daemon, sets up the TCP listener on port 80,
	 * creates request handlers, and registers the service with mDNS.
	 */
	HttpDaemon();

	/**
	 * @brief Destructor for HttpDaemon.
	 *
	 * Cleans up resources, unregisters the mDNS service,
	 * destroys request handlers, and stops the TCP listener.
	 */
	~HttpDaemon();

private:
	static void Input(const int32_t nConnectionHandle, const uint8_t *pBuffer, const uint32_t nSize) {
		handleRequest[nConnectionHandle].HandleRequest(nSize, const_cast<char *>(reinterpret_cast<const char *>(pBuffer)));
	}

	/**
	 * https://www.gd32-dmx.org/memory.html
	 */
#if defined (GD32F207RG) || defined (GD32F450VE) || defined (GD32F470ZK)
# define SECTION_HTTPD	__attribute__ ((section (".httpd")))
#else
# define SECTION_HTTPD
#endif
	/*
	 * Each handler corresponds to a connection handle. Objects are constructed
	 * using placement new and must be explicitly destructed.
	 */
	static inline HttpDeamonHandleRequest handleRequest[TCP_MAX_TCBS_ALLOWED] __attribute__ ((aligned (4))) SECTION_HTTPD;
	int32_t m_nHandle { -1 };
};

#endif /* HTTPD_HTTPD_H_ */
