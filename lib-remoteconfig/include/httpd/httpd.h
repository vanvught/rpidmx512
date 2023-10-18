/**
 * @file httpd.h
 *
 */
/* Copyright (C) 2021-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef HTTPD_H_
#define HTTPD_H_

#include <cstdint>

#include "network.h"

namespace http {
static constexpr uint32_t BUFSIZE = 1440;
enum class Status {
	OK = 200,
	BAD_REQUEST = 400,
	NOT_FOUND = 404,
	REQUEST_TIMEOUT = 408,
	REQUEST_ENTITY_TOO_LARGE = 413,
	REQUEST_URI_TOO_LONG = 414,
	INTERNAL_SERVER_ERROR = 500,
	METHOD_NOT_IMPLEMENTED = 501,
	VERSION_NOT_SUPPORTED = 505,
	UNKNOWN_ERROR = 520
};
enum class RequestMethod {
	GET, POST, UNKNOWN
};

enum class contentTypes {
	TEXT_HTML, TEXT_CSS, TEXT_JS, APPLICATION_JSON, NOT_DEFINED
};
}  // namespace http

class HttpDaemon {
public:
	HttpDaemon();
	void Run() {
		uint32_t nConnectionHandle;
		m_nBytesReceived = Network::Get()->TcpRead(m_nHandle, const_cast<const uint8_t **>(reinterpret_cast<uint8_t **>(&m_RequestHeaderResponse)), nConnectionHandle);

		if (__builtin_expect((m_nBytesReceived == 0), 1)) {
			return;
		}

		HandleRequest(nConnectionHandle);
	}

private:
	void HandleRequest(const uint32_t nConnectionHandle);
	http::Status ParseRequest();
	http::Status ParseMethod(char *pLine);
	http::Status ParseHeaderField(char *pLine);
	http::Status HandleGet();
	http::Status HandlePost(bool hasDataOnly);
	http::Status HandleGetTxt();

private:
	const char *m_pContentType;
	char *m_pUri { nullptr };
	char *m_pFileData { nullptr };
	char *m_RequestHeaderResponse { nullptr };

	uint32_t m_nContentLength { 0 };
	uint32_t m_nFileDataLength { 0 };
	uint32_t m_nRequestContentLength { 0 };
	int32_t m_nHandle { -1 };

	uint32_t m_nBytesReceived { 0 };

	http::Status m_Status { http::Status::UNKNOWN_ERROR };
	http::RequestMethod m_RequestMethod { http::RequestMethod::UNKNOWN };

	bool m_bContentTypeJson { false };
	bool m_IsAction { false };

	static char m_Content[http::BUFSIZE];
};

#endif /* HTTPD_H_ */
