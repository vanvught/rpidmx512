/**
 * @file httd.cpp
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

#include <cstring>
#include <cstdio>
#include <cassert>

#include "httpd/httpd.h"
#include "remoteconfig.h"
#include "remoteconfigjson.h"
#include "properties.h"
#include "sscan.h"
#include "propertiesconfig.h"
#include "../http/content/json_switch.h"

#include "network.h"
#include "hardware.h"
#include "display.h"

#include "debug.h"

#if defined ENABLE_CONTENT
extern int get_file_content(const char *fileName, char *pDst, http::contentTypes& contentType);
#endif

char HttpDaemon::m_Content[http::BUFSIZE];

using namespace http;

static constexpr char s_contentType[static_cast<uint32_t>(contentTypes::NOT_DEFINED)][32] =
	{ "text/html", "text/css", "text/javascript", "application/json" };

HttpDaemon::HttpDaemon() : m_pContentType(s_contentType[static_cast<uint32_t>(contentTypes::TEXT_HTML)]) {
	DEBUG_ENTRY

	assert(m_nHandle == -1);
	m_nHandle = Network::Get()->TcpBegin(80);
	assert(m_nHandle != -1);

	DEBUG_EXIT
}

void HttpDaemon::HandleRequest(const uint32_t nConnectionHandle) {
	const char *pStatusMsg = "OK";

	DEBUG_PRINTF("%u: m_Status=%u, m_RequestMethod=%u", nConnectionHandle, static_cast<uint32_t>(m_Status), static_cast<uint32_t>(m_RequestMethod));

	if (m_Status == Status::UNKNOWN_ERROR) {
		m_Status = ParseRequest();
		if (m_Status == Status::OK) {
			if (m_RequestMethod == RequestMethod::GET) {
				m_Status = HandleGet();
			} else if (m_RequestMethod == RequestMethod::POST) {
				m_Status = HandlePost(false);
				if ((m_Status == Status::OK) && (m_nFileDataLength == 0)) {
					DEBUG_PUTS("There is a POST header only -> no data");
					return;
				}
			}
		}
	} else if ((m_Status == Status::OK) && (m_RequestMethod == RequestMethod::POST)) {
		m_Status = HandlePost(true);
	}

	if (m_Status != Status::OK) {
		switch (m_Status) {
		case Status::BAD_REQUEST:
			pStatusMsg = "Bad Request";
			break;
		case Status::NOT_FOUND:
			pStatusMsg = "Not Found";
			break;
		case Status::REQUEST_ENTITY_TOO_LARGE:
			pStatusMsg = "Request Entity Too Large";
			break;
		case Status::REQUEST_URI_TOO_LONG:
			pStatusMsg = "Request-URI Too Long";
			break;
		case Status::INTERNAL_SERVER_ERROR:
			pStatusMsg = "Internal Server Error";
			break;
		case Status::METHOD_NOT_IMPLEMENTED:
			pStatusMsg = "Method Not Implemented";
			break;
		case Status::VERSION_NOT_SUPPORTED:
			pStatusMsg = "Version Not Supported";
			break;
		default:
			pStatusMsg = "Unknown Error";
			break;
		}

		m_pContentType = s_contentType[static_cast<uint32_t>(contentTypes::TEXT_HTML)];
		m_nContentLength = static_cast<uint32_t>(snprintf(m_Content, BUFSIZE - 1U,
				"<!DOCTYPE html>\n"
				"<html>\n"
				"<head><title>%u %s</title></head>\n"
				"<body><h1>%s</h1></body>\n"
				"</html>\n", static_cast<uint32_t>(m_Status), pStatusMsg, pStatusMsg));
	}

	uint8_t nLength;
	const int nHeaderLength = snprintf(m_RequestHeaderResponse, BUFSIZE - 1U,
			"HTTP/1.1 %u %s\r\n"
			"Server: %s\r\n"
			"Content-Type: %s\r\n"
			"Content-Length: %u\r\n"
			"Connection: close\r\n"
			"\r\n", static_cast<uint32_t>(m_Status), pStatusMsg, Hardware::Get()->GetBoardName(nLength), m_pContentType, m_nContentLength);

	Network::Get()->TcpWrite(m_nHandle, reinterpret_cast<uint8_t *>(m_RequestHeaderResponse), static_cast<uint16_t>(nHeaderLength), nConnectionHandle);
	Network::Get()->TcpWrite(m_nHandle, reinterpret_cast<uint8_t *>(m_Content), static_cast<uint16_t>(m_nContentLength), nConnectionHandle);
	DEBUG_PRINTF("m_nContentLength=%u", m_nContentLength);

	m_Status = Status::UNKNOWN_ERROR;
	m_RequestMethod = RequestMethod::UNKNOWN;
}

Status HttpDaemon::ParseRequest() {
	char *pLine = m_RequestHeaderResponse;
	uint32_t nLine = 0;
	Status status = Status::UNKNOWN_ERROR;
	m_bContentTypeJson = false;
	m_nRequestContentLength = 0;
	m_nFileDataLength = 0;

	for (uint32_t i = 0; i < m_nBytesReceived; i++) {
		if (m_RequestHeaderResponse[i] == '\n') {
			assert(i > 1);
			m_RequestHeaderResponse[i - 1] = '\0';

			if (nLine++ == 0) {
				status = ParseMethod(pLine);
			} else {
				if (pLine[0] == '\0') {
					assert((i + 1) <= m_nBytesReceived);
					m_nFileDataLength = static_cast<uint16_t>(m_nBytesReceived - 1 - i);
					if (m_nFileDataLength > 0) {
						m_pFileData = &m_RequestHeaderResponse[i + 1];
						m_pFileData[m_nFileDataLength] = '\0';
					}
					return Status::OK;
				}
				status = ParseHeaderField(pLine);
			}

			if (status != Status::OK) {
				return status;
			}

			pLine = &m_RequestHeaderResponse[++i];
		}
	}

	return Status::OK;
}

/**
 * Supported: "METHOD uri HTTP/1.1"
 * Where METHOD is "GET" or "POST"
 */

Status HttpDaemon::ParseMethod(char *pLine) {
	assert(pLine != nullptr);
	char *pToken;

	if ((pToken = strtok(pLine, " ")) == nullptr) {
		return Status::METHOD_NOT_IMPLEMENTED;
	}

	if (strcmp(pToken, "GET") == 0) {
		m_RequestMethod = RequestMethod::GET;
	} else if (strcmp(pToken, "POST") == 0) {
		m_RequestMethod = RequestMethod::POST;
	} else {
		return Status::METHOD_NOT_IMPLEMENTED;
	}

	if ((pToken = strtok(nullptr, " ")) == nullptr) {
		return Status::BAD_REQUEST;
	}

	m_pUri = pToken;

	if ((pToken = strtok(nullptr, "/")) == nullptr || strcmp(pToken, "HTTP") != 0) {
		return Status::BAD_REQUEST;
	}

	if ((pToken = strtok(nullptr, " \n")) == nullptr) {
		return Status::BAD_REQUEST;
	}

	if (strcmp(pToken, "1.1") != 0) {
		return Status::VERSION_NOT_SUPPORTED;
	}

	return Status::OK;
}

/**
 * Only interested in "Content-Type" and
 * "Content-Length"
 * Where we check for "Content-Type: application/json"
 */

Status HttpDaemon::ParseHeaderField(char *pLine) {
	char *pToken;

	assert(pLine != 0);
	if ((pToken = strtok(pLine, ":")) == nullptr) {
		return Status::BAD_REQUEST;
	}

	if (strcasecmp(pToken, "Content-Type") == 0) {
		if ((pToken = strtok(nullptr, " ;")) == nullptr) {
			return Status::BAD_REQUEST;
		}
		if (strcmp(pToken, "application/json") == 0) {
			m_bContentTypeJson = true;
		}
	} else if (strcasecmp(pToken, "Content-Length") == 0) {
		if ((pToken = strtok(nullptr, " ")) == nullptr) {
			return Status::BAD_REQUEST;
		}

		uint32_t nTmp = 0;
		while (*pToken != '\0') {
			auto nDigit = static_cast<uint32_t>(*pToken++ - '0');
			if (nDigit > 9) {
				return Status::BAD_REQUEST;
			}

			nTmp *= 10;
			nTmp += nDigit;

			if (nTmp > BUFSIZE) {
				return Status::REQUEST_ENTITY_TOO_LARGE;
			}
		}

		m_nRequestContentLength = nTmp;
	}

	DEBUG_EXIT
	return Status::OK;
}

/**
 * GET
 */

Status HttpDaemon::HandleGet() {
	int nLength = 0;

	if (memcmp(m_pUri, "/json/", 6) == 0) {
		m_pContentType = s_contentType[static_cast<uint32_t>(contentTypes::APPLICATION_JSON)];
		const auto *pGet = &m_pUri[6];
		switch (http::get_uint(pGet)) {
		case http::json::get::LIST:
			nLength = remoteconfig::json_get_list(m_Content, sizeof(m_Content));
			break;
		case http::json::get::VERSION:
			nLength = remoteconfig::json_get_version(m_Content, sizeof(m_Content));
			break;
		case http::json::get::UPTIME:
			if (!RemoteConfig::Get()->IsEnableUptime()) {
				DEBUG_PUTS("Status::BAD_REQUEST");
				return Status::BAD_REQUEST;
			}
			nLength = remoteconfig::json_get_uptime(m_Content, sizeof(m_Content));
			break;
		case http::json::get::DISPLAY:
			nLength = remoteconfig::json_get_display(m_Content, sizeof(m_Content));
			break;
		case http::json::get::DIRECTORY:
			nLength = remoteconfig::json_get_directory(m_Content, sizeof(m_Content));
			break;
#if defined (ENABLE_NET_PHYSTATUS)
		case http::json::get::PHYSTATUS:
			nLength = remoteconfig::net::json_get_phystatus(m_Content, sizeof(m_Content));
			break;
#endif
		default:
#if defined (ENABLE_PHY_SWITCH)
			if (memcmp(pGet, "dsa/", 4) == 0) {
				const auto *pDsa = &pGet[4];
				switch (http::get_uint(pDsa)) {
				case http::json::get::PORTSTATUS:
					nLength = remoteconfig::dsa::json_get_portstatus(m_Content, sizeof(m_Content));
					break;
				default:
					break;
				}
			} else {
#endif
				return HandleGetTxt();
#if defined (ENABLE_PHY_SWITCH)
			}
#endif
			break;
		}
	}
#if defined (ENABLE_CONTENT)
	else if (strcmp(m_pUri, "/") == 0) {
		http::contentTypes contentType;
		nLength = get_file_content("index.html", m_Content, contentType);
		m_pContentType = s_contentType[static_cast<uint32_t>(contentType)];
	}
#if defined (ENABLE_PHY_SWITCH)
	else if (strcmp(m_pUri, "/dsa") == 0) {
		http::contentTypes contentType;
		nLength = get_file_content("dsa.html", m_Content, contentType);
		m_pContentType = s_contentType[static_cast<uint32_t>(contentType)];
	}
#endif
	else {
		http::contentTypes contentType;
		nLength = get_file_content(&m_pUri[1], m_Content, contentType);
		m_pContentType = s_contentType[static_cast<uint32_t>(contentType)];
	}
#endif

	if (nLength <= 0) {
		DEBUG_EXIT
		return Status::NOT_FOUND;
	}

	m_nContentLength = static_cast<uint16_t>(nLength);

	DEBUG_EXIT
	return Status::OK;
}

Status HttpDaemon::HandleGetTxt() {
	auto *pFileName = &m_pUri[6];
	const auto nLength = strlen(pFileName);

	if (nLength <= 4) {
		return Status::BAD_REQUEST;
	}

	if (strcmp(&pFileName[nLength - 4], ".txt") != 0) {
		return Status::BAD_REQUEST;
	}

	const auto bIsJSON = PropertiesConfig::IsJSON();

	PropertiesConfig::EnableJSON(true);
	const auto nBytes = RemoteConfig::Get()->HandleGet(reinterpret_cast<void *>(pFileName), BUFSIZE - 5U);

	PropertiesConfig::EnableJSON(bIsJSON);

	DEBUG_PRINTF("nBytes=%d", nBytes);

	if (nBytes <= 0) {
		return Status::BAD_REQUEST;
	}

	m_nContentLength = static_cast<uint16_t>(nBytes);
	memcpy(m_Content, reinterpret_cast<void *>(pFileName), nBytes);

	return Status::OK;
}

/**
 * POST
 */

Status HttpDaemon::HandlePost(bool hasDataOnly) {
	DEBUG_PRINTF("m_nBytesReceived=%d, m_nFileDataLength=%u, m_nRequestContentLength=%u -> hasDataOnly=%c", m_nBytesReceived, m_nFileDataLength, m_nRequestContentLength, hasDataOnly ? 'Y' : 'N');

	if (!hasDataOnly) {
		if (!m_bContentTypeJson) {
			return Status::BAD_REQUEST;
		}

		m_IsAction = (strcmp(m_pUri, "/json/action") == 0);

		if (!m_IsAction && (strcmp(m_pUri, "/json") != 0)) {
			return Status::NOT_FOUND;
		}
	}

	const auto hasHeadersOnly = (!hasDataOnly && ((m_nBytesReceived < m_nRequestContentLength) || m_nFileDataLength == 0));

	if (hasHeadersOnly) {
		DEBUG_PUTS("hasHeadersOnly");
		return Status::OK;
	}


	if (hasDataOnly) {
		m_pFileData = m_RequestHeaderResponse;
		m_nFileDataLength = static_cast<uint16_t>(m_nBytesReceived);
	}

	DEBUG_PRINTF("%d|%.*s|->%d", m_nFileDataLength, m_nFileDataLength, m_pFileData, m_IsAction);

	if (m_IsAction) {
		if (properties::convert_json_file(m_pFileData, m_nFileDataLength, true) <= 0) {
			DEBUG_PUTS("Status::BAD_REQUEST");
			return Status::BAD_REQUEST;
		}

		uint8_t value8;

		if (Sscan::Uint8(m_pFileData, "reboot", value8) == Sscan::OK) {
			if (value8 != 0) {
				if (!RemoteConfig::Get()->IsEnableReboot()) {
					DEBUG_PUTS("Status::BAD_REQUEST");
					return Status::BAD_REQUEST;
				}
				DEBUG_PUTS("Reboot!");
				RemoteConfig::Get()->Reboot();
				__builtin_unreachable();
			}
		} else if (Sscan::Uint8(m_pFileData, "display", value8) == Sscan::OK) {
			Display::Get()->SetSleep(value8 == 0);
			DEBUG_PRINTF("Display::Get()->SetSleep(%d)", value8 == 0);
		} else if (Sscan::Uint8(m_pFileData, "identify", value8) == Sscan::OK) {
			if (value8 != 0) {
				Hardware::Get()->SetMode(hardware::ledblink::Mode::FAST);
			} else {
				Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
			}
			DEBUG_PRINTF("identify=%d", value8 != 0);
		} else {
			DEBUG_PUTS("Status::BAD_REQUEST");
			return Status::BAD_REQUEST;
		}
	} else {
		const auto bIsJSON = PropertiesConfig::IsJSON();

		PropertiesConfig::EnableJSON(true);
		RemoteConfig::Get()->HandleSet(m_pFileData, m_nFileDataLength);

		PropertiesConfig::EnableJSON(bIsJSON);
	}

	m_pContentType = s_contentType[static_cast<uint32_t>(contentTypes::TEXT_HTML)];
	m_nContentLength = static_cast<uint16_t>(snprintf(m_Content, BUFSIZE - 1U,
			"<!DOCTYPE html>\n"
			"<html>\n"
			"<head><title>Submit</title></head>\n"
			"<body><h1>OK</h1></body>\n"
			"</html>\n"));

	return Status::OK;
}
