/**
 * @file httd.cpp
 *
 */
/* Copyright (C) 2021-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if defined ENABLE_CONTENT
extern int get_file_content(const char *fileName, char *pDst);
#endif

#include "httpd/httpd.h"
#include "remoteconfig.h"
#include "remoteconfigjson.h"
#include "properties.h"
#include "sscan.h"
#include "propertiesconfig.h"

#include "network.h"
#include "hardware.h"
#include "ledblink.h"
#include "display.h"

#include "debug.h"

using namespace http;

enum class contentTypes {
	TEXT_HTML, TEXT_CSS, TEXT_JS, APPLICATION_JSON, NOT_DEFINED
};

static constexpr char contentType[static_cast<uint32_t>(contentTypes::NOT_DEFINED)][32] =
	{ "text/html", "text/css", "text/javascript", "application/json" };

HttpDaemon::HttpDaemon() : m_pContentType(contentType[static_cast<uint32_t>(contentTypes::TEXT_HTML)]) {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void HttpDaemon::Start() {
	DEBUG_ENTRY

	assert(m_nHandle == -1);
	m_nHandle = Network::Get()->TcpBegin(80);
	assert(m_nHandle != -1);

	DEBUG_EXIT
}

void HttpDaemon::Stop() {
	DEBUG_ENTRY

	assert(m_nHandle != -1);
	m_nHandle = Network::Get()->TcpEnd(80);
	assert(m_nHandle == -1);

	DEBUG_EXIT
}

void HttpDaemon::Run() {
	m_nBytesReceived = Network::Get()->TcpRead(m_nHandle, const_cast<const uint8_t **>(reinterpret_cast<uint8_t **>(&m_RequestHeaderResponse)));

	if (__builtin_expect((m_nBytesReceived <= 0), 1)) {
		return;
	}

	const char *pStatusMsg = "OK";

	DEBUG_PRINTF("m_Status=%u, m_RequestMethod=%u", static_cast<uint32_t>(m_Status), static_cast<uint32_t>(m_RequestMethod));

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

		m_pContentType = contentType[static_cast<uint32_t>(contentTypes::TEXT_HTML)];
		m_nContentLength = static_cast<uint16_t>(snprintf(m_Content, BUFSIZE - 1U,
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

	Network::Get()->TcpWrite(m_nHandle, reinterpret_cast<uint8_t *>(m_RequestHeaderResponse), static_cast<uint16_t>(nHeaderLength));
	Network::Get()->TcpWrite(m_nHandle, reinterpret_cast<uint8_t *>(m_Content), m_nContentLength);
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

	for (uint16_t i = 0; i < static_cast<uint16_t>(m_nBytesReceived); i++) {
		if (m_RequestHeaderResponse[i] == '\n') {
			assert(i > 1);
			m_RequestHeaderResponse[i - 1] = '\0';

			if (nLine++ == 0) {
				status = ParseMethod(pLine);
			} else {
				if (pLine[0] == '\0') {
					assert((i + 1) <= m_nBytesReceived);
					m_nFileDataLength = static_cast<uint16_t>(static_cast<uint16_t>(m_nBytesReceived) - 1U - i);
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

	if ((pToken = strtok(pLine, " ")) == 0) {
		return Status::METHOD_NOT_IMPLEMENTED;
	}

	if (strcmp(pToken, "GET") == 0) {
		m_RequestMethod = RequestMethod::GET;
	} else if (strcmp(pToken, "POST") == 0) {
		m_RequestMethod = RequestMethod::POST;
	} else {
		return Status::METHOD_NOT_IMPLEMENTED;
	}

	if ((pToken = strtok(0, " ")) == 0) {
		return Status::BAD_REQUEST;
	}

	m_pUri = pToken;

	if ((pToken = strtok(0, "/")) == nullptr || strcmp(pToken, "HTTP") != 0) {
		return Status::BAD_REQUEST;
	}

	if ((pToken = strtok(0, " \n")) == nullptr) {
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
		if ((pToken = strtok(0, " ;")) == nullptr) {
			return Status::BAD_REQUEST;
		}
		if (strcmp(pToken, "application/json") == 0) {
			m_bContentTypeJson = true;
		}
	} else if (strcasecmp(pToken, "Content-Length") == 0) {
		if ((pToken = strtok(0, " ")) == nullptr) {
			return Status::BAD_REQUEST;
		}

		uint32_t nTmp = 0;
		while (*pToken != '\0') {
			auto nDigit = static_cast<uint32_t>(*pToken++ - '0');
			if (nDigit > 9) {
				return Status::BAD_REQUEST;;
			}

			nTmp *= 10;
			nTmp += nDigit;

			if (nTmp > BUFSIZE) {
				return Status::REQUEST_ENTITY_TOO_LARGE;
			}
		}

		m_nRequestContentLength = static_cast<uint16_t>(nTmp);
	}

	DEBUG_EXIT
	return Status::OK;
}

/**
 * GET
 */

Status HttpDaemon::HandleGet() {
	int nLength = 0;

#if defined(ENABLE_CONTENT)
	if ((strcmp(m_pUri, "/") == 0) || (strcmp(m_pUri, "/index.html") == 0)) {
		m_pContentType = contentType[static_cast<uint32_t>(contentTypes::TEXT_HTML)];
		nLength = get_file_content("index.html", m_Content);
	} else if (strcmp(m_pUri, "/styles.css") == 0) {
		m_pContentType = contentType[static_cast<uint32_t>(contentTypes::TEXT_CSS)];
		nLength = get_file_content("styles.css", m_Content);
	} else if (strcmp(m_pUri, "/index.js") == 0) {
		m_pContentType = contentType[static_cast<uint32_t>(contentTypes::TEXT_JS)];
		nLength = get_file_content("index.js", m_Content);
	} else
#endif
	if (memcmp(m_pUri, "/json/", 6) == 0) {
		m_pContentType = contentType[static_cast<uint32_t>(contentTypes::APPLICATION_JSON)];
		const auto *pGet = &m_pUri[6];
		if (strcmp(pGet, "list") == 0) {
			nLength = remoteconfig::json_get_list(m_Content, sizeof(m_Content));
		} else if (strcmp(pGet, "version") == 0) {
			nLength = remoteconfig::json_get_version(m_Content, sizeof(m_Content));
		} else if (strcmp(pGet, "uptime") == 0) {
			if (!RemoteConfig::Get()->IsEnableUptime()) {
				DEBUG_PUTS("Status::BAD_REQUEST");
				return Status::BAD_REQUEST;
			}
			nLength = remoteconfig::json_get_uptime(m_Content, sizeof(m_Content));
		} else if (strcmp(pGet, "display") == 0) {
			nLength = remoteconfig::json_get_display(m_Content, sizeof(m_Content));
		} else if (strcmp(pGet, "directory") == 0) {
			nLength = remoteconfig::json_get_directory(m_Content, sizeof(m_Content));
		} else {
			return HandleGetTxt();
		}
	}

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
				Hardware::Get()->Reboot();
				__builtin_unreachable();
			}
		} else if (Sscan::Uint8(m_pFileData, "display", value8) == Sscan::OK) {
			Display::Get()->SetSleep(value8 == 0);
			DEBUG_PRINTF("Display::Get()->SetSleep(%d)", value8 == 0);
		} else if (Sscan::Uint8(m_pFileData, "identify", value8) == Sscan::OK) {
			if (value8 != 0) {
				LedBlink::Get()->SetMode(ledblink::Mode::FAST);
			} else {
				LedBlink::Get()->SetMode(ledblink::Mode::NORMAL);
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

	m_pContentType = contentType[static_cast<uint32_t>(contentTypes::TEXT_HTML)];
	m_nContentLength = static_cast<uint16_t>(snprintf(m_Content, BUFSIZE - 1U,
			"<!DOCTYPE html>\n"
			"<html>\n"
			"<head><title>Submit</title></head>\n"
			"<body><h1>OK</h1></body>\n"
			"</html>\n"));

	return Status::OK;
}
