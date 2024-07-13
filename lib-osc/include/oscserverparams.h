/**
 * @file oscserverparams.h
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef OSCSERVERPARAMS_H_
#define OSCSERVERPARAMS_H_

#include <cstdint>

#include "oscserver.h"
#include "configstore.h"
#include "lightset.h"

namespace osc {
namespace server {
struct Params {
	uint32_t nSetList;
	uint16_t nIncomingPort;
	uint16_t nOutgoingPort;
	uint8_t tOutputType;
	bool bPartialTransmission;
	char aPath[osc::server::Max::PATH_LENGTH];
	char aPathInfo[osc::server::Max::PATH_LENGTH];
	char aPathBlackOut[osc::server::Max::PATH_LENGTH];
} __attribute__((packed));

struct ParamsMask {
	static constexpr uint32_t INCOMING_PORT = (1U << 0);
	static constexpr uint32_t OUTGOING_PORT = (1U << 1);
	static constexpr uint32_t PATH = (1U << 2);
	static constexpr uint32_t TRANSMISSION = (1U << 3);
	static constexpr uint32_t OUTPUT = (1U << 4);
	static constexpr uint32_t PATH_INFO = (1U << 5);
	static constexpr uint32_t PATH_BLACKOUT = (1U << 6);
};
}  // namespace server
}  // namespace osc

class StoreOscServer {
public:
	static void Update(const struct osc::server::Params *pParams) {
		ConfigStore::Get()->Update(configstore::Store::OSC, pParams, sizeof(struct osc::server::Params));
	}

	static void Copy(struct osc::server::Params *pParams) {
		ConfigStore::Get()->Copy(configstore::Store::OSC, pParams, sizeof(struct osc::server::Params));
	}
};

class OSCServerParams {
public:
	OSCServerParams();

	void Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const osc::server::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	void Set(OscServer *pOscServer);

	uint16_t GetIncomingPort() const {
		return m_Params.nIncomingPort;
	}

	uint16_t GetOutgoingPort() const {
		return m_Params.nOutgoingPort;
	}

	bool GetPartialTransmission() const {
		return m_Params.bPartialTransmission;
	}

    static void staticCallbackFunction(void *p, const char *s);

private:
	void Dump();
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_Params.nSetList & nMask) == nMask;
    }

private:
    osc::server::Params m_Params;
};

#endif /* OSCSERVERPARAMS_H_ */
