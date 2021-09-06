/**
 * @file httd.cpp
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cstring>
#include <cassert>

#include "httpd/httpd.h"
#include "remoteconfig.h"
#include "remoteconfigjson.h"
#include "propertiesconfig.h"

#include "network.h"
#include "hardware.h"

#include "debug.h"

using namespace http;

enum class contentTypes {
	TEXT_HTML, APPLICATION_JSON, NOT_DEFINED
};

static constexpr char contentType[static_cast<uint32_t>(contentTypes::NOT_DEFINED)][32] =
	{ "text/html", "application/json" };

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
	m_nBytesReceicved = Network::Get()->TcpRead(m_nHandle, reinterpret_cast<uint8_t **>(&m_RequestHeaderResponse));

	if (m_nBytesReceicved <= 0) {
		return;
	}

	bool bIsDataOnly;

	if ((m_RequestMethod == RequestMethod::POST) && (m_nFileDataLength == 0)) {
		bIsDataOnly = true;
	} else {
		bIsDataOnly = false;
		m_Status = ParseRequest();
	}

	const char *pStatusMsg = "OK";
	m_pContentType = contentType[static_cast<uint32_t>(contentTypes::TEXT_HTML)];

	if (m_Status == Status::OK) {
		if (m_RequestMethod == RequestMethod::GET) {
			m_Status = HandleGet();
		} else {
			assert(m_RequestMethod == RequestMethod::POST);
			m_Status = HandlePost(bIsDataOnly);
		}
	}

	// There is a POST header only -> no data
	if ((m_Status == Status::OK) && (m_RequestMethod == RequestMethod::POST) && (m_nFileDataLength == 0)) {
		return;
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
	const int nHeaderLength = snprintf(m_RequestHeaderResponse, BUFSIZE - 1,
			"HTTP/1.1 %u %s\r\n"
			"Server: %s\r\n"
			"Content-Type: %s\r\n"
			"Content-Length: %u\r\n"
			"Connection: close\r\n"
			"\r\n", static_cast<uint32_t>(m_Status), pStatusMsg, Hardware::Get()->GetBoardName(nLength), m_pContentType, m_nContentLength);

	Network::Get()->TcpWrite(m_nHandle, reinterpret_cast<uint8_t *>(m_RequestHeaderResponse), static_cast<uint16_t>(nHeaderLength));
	Network::Get()->TcpWrite(m_nHandle, reinterpret_cast<uint8_t *>(m_Content), m_nContentLength);

	m_Status = Status::UNKNOWN_ERROR;
	m_RequestMethod = RequestMethod::UNKNOWN;
	m_pFileData = nullptr;
	m_nFileDataLength = 0;
}

Status HttpDaemon::ParseRequest() {
	char *pLine = m_RequestHeaderResponse;
	uint32_t nLine = 0;
	Status status = Status::UNKNOWN_ERROR;
	m_nFileDataLength = 0;
	m_bContentTypeJson = false;

	for (uint16_t i = 0; i < static_cast<uint16_t>(m_nBytesReceicved); i++) {
		if (m_RequestHeaderResponse[i] == '\n') {
			assert(i > 1);
			m_RequestHeaderResponse[i - 1] = '\0';

			if (nLine++ == 0) {
				status = ParseMethod(pLine);
			} else {
				if (pLine[0] == '\0') {
					assert((i + 1) <= m_nBytesReceicved);
					m_nFileDataLength = static_cast<uint16_t>(m_nBytesReceicved - 1) - i;
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
	char *pToken;

	assert(pLine != nullptr);

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

	if (strcmp(pToken, "Content-Type") == 0) {
		if ((pToken = strtok(0, " ;")) == nullptr) {
			return Status::BAD_REQUEST;
		}
		if (strcmp(pToken, "application/json") == 0) {
			m_bContentTypeJson = true;
		}
	} else if (strcmp(pToken, "Content-Length") == 0) {
		if ((pToken = strtok(0, " ")) == nullptr) {
			return Status::BAD_REQUEST;
		}
		//TODO parse value OR just using m_nFileDataLength
	}

	return Status::OK;
}

/**
 * GET
 */

Status HttpDaemon::HandleGet() {
	int nLength = 0;

#if defined(ENABLE_CONTENT)
	if ((strcmp(m_pUri, "/") == 0) || (strncmp(m_pUri, "/index.html", 11) == 0)) {
		  nLength = get_file_content("index.html", m_Content);
	} else if (strcmp(m_pUri, "/styles.css") == 0) {
		 nLength = get_file_content("styles.css", m_Content);
	} else if (strcmp(m_pUri, "/index.js") == 0) {
		 nLength = get_file_content("index.js", m_Content);
	} else
#endif
	if (memcmp(m_pUri, "/json/", 6) == 0) {
		m_pContentType = contentType[static_cast<uint32_t>(contentTypes::APPLICATION_JSON)];
		const auto *pGet = &m_pUri[6];
		if (strncmp(pGet, "list", 4) == 0) {
			nLength = remoteconfig::json_get_list(m_Content, sizeof(m_Content));
		} else if (strncmp(pGet, "version", 7) == 0) {
			nLength = remoteconfig::json_get_version(m_Content, sizeof(m_Content));
		} else if (strncmp(pGet, "uptime", 6) == 0) {
			nLength = remoteconfig::json_get_uptime(m_Content, sizeof(m_Content));
		} else if (strncmp(pGet, "display", 7) == 0) {
			nLength = remoteconfig::json_get_display(m_Content, sizeof(m_Content));
		} else {
			return HandleGetJSON();
		}
	}

	if (nLength <= 0) {
		return Status::NOT_FOUND;
	}

	m_nContentLength = static_cast<uint16_t>(nLength);

	return Status::OK;
}

Status HttpDaemon::HandleGetJSON() {
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
	const auto nBytes = RemoteConfig::Get()->HandleGet(reinterpret_cast<void *>(pFileName), 1440U - 5U); //TODO

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

Status HttpDaemon::HandlePost(bool bIsDataOnly) {
	if (!bIsDataOnly) {
		if (strcmp(m_pUri, "/json") != 0) {
			return Status::NOT_FOUND;
		}

		if (!m_bContentTypeJson) {
			return Status::BAD_REQUEST;
		}
	}

	if (bIsDataOnly) {
		m_pFileData = m_RequestHeaderResponse;
		m_nFileDataLength = static_cast<uint16_t>(m_nBytesReceicved);
	}

	DEBUG_PRINTF("%d|%.*s|\n", m_nFileDataLength, m_nFileDataLength, m_pFileData);

	const auto bIsJSON = PropertiesConfig::IsJSON();

	PropertiesConfig::EnableJSON(true);
	RemoteConfig::Get()->HandleTxtFile(m_pFileData, m_nFileDataLength);

	PropertiesConfig::EnableJSON(bIsJSON);

	m_nContentLength = static_cast<uint16_t>(snprintf(m_Content, BUFSIZE - 1U,
			"<!DOCTYPE html>\n"
			"<html>\n"
			"<head><title>Submit</title></head>\n"
			"<body><h1>OK</h1></body>\n"
			"</html>\n"));

	return Status::OK;
}
