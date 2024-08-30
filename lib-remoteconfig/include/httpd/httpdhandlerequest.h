/**
 * @file httpdhandlerequest.h
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef HTTPD_HTTPDHANDLEREQUEST_H_
#define HTTPD_HTTPDHANDLEREQUEST_H_

#include <cstdint>

#include "http.h"

#include "debug.h"

class HttpDeamonHandleRequest {
public:
	HttpDeamonHandleRequest(uint32_t nConnectionHandle, int32_t nHandle) : m_nConnectionHandle(nConnectionHandle), m_nHandle(nHandle) {
		DEBUG_ENTRY
		DEBUG_PRINTF("m_nConnectionHandle=%u", m_nConnectionHandle);
		DEBUG_EXIT
	}

	void HandleRequest(const uint32_t nBytesReceived, char *pRequestHeaderResponse);

private:
	http::Status ParseRequest();
	http::Status ParseMethod(char *pLine);
	http::Status ParseHeaderField(char *pLine);
	http::Status HandleGet();
	http::Status HandleGetTxt();
	http::Status HandlePost(const bool hasDataOnly);
	http::Status HandleDelete(const bool hasDataOnly);

private:
	uint32_t m_nConnectionHandle;
	int32_t m_nHandle;
	uint32_t m_nContentSize { 0 };
	uint32_t m_nFileDataLength { 0 };
	uint32_t m_nRequestContentSize { 0 };
	uint32_t m_nBytesReceived { 0 };

	char *m_pUri { nullptr };
	char *m_pFileData { nullptr };
	const char *m_pContent { nullptr };
	char *m_RequestHeaderResponse { nullptr };

	http::Status m_Status { http::Status::UNKNOWN_ERROR };
	http::RequestMethod m_RequestMethod { http::RequestMethod::UNKNOWN };
	http::contentTypes m_ContentType { http::contentTypes::NOT_DEFINED };

	bool m_IsAction { false };

	static char m_DynamicContent[http::BUFSIZE];
};


#endif /* HTTPD_HTTPDHANDLEREQUEST_H_ */
