/**
 * @file oscserverparams.h
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
	static constexpr auto INCOMING_PORT = (1U << 0);
	static constexpr auto OUTGOING_PORT = (1U << 1);
	static constexpr auto PATH = (1U << 2);
	static constexpr auto TRANSMISSION = (1U << 3);
	static constexpr auto OUTPUT = (1U << 4);
	static constexpr auto PATH_INFO = (1U << 5);
	static constexpr auto PATH_BLACKOUT = (1U << 6);
};
}  // namespace server
}  // namespace osc

class OSCServerParamsStore {
public:
	virtual ~OSCServerParamsStore() {}

	virtual void Update(const osc::server::Params *pOSCServerParams)=0;
	virtual void Copy(osc::server::Params *pOSCServerParams)=0;
};

class OSCServerParams {
public:
	OSCServerParams(OSCServerParamsStore *m_pOSCServerParamsStore);

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const osc::server::Params *ptOSCServerParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	void Set(OscServer *pOscServer);

	void Dump();

	uint16_t GetIncomingPort() const {
		return m_tOSCServerParams.nIncomingPort;
	}

	uint16_t GetOutgoingPort() const {
		return m_tOSCServerParams.nOutgoingPort;
	}

	bool GetPartialTransmission() const {
		return m_tOSCServerParams.bPartialTransmission;
	}

#if defined (ESP8266)
	lightset::OutputType GetOutputType() const {
		return static_cast<lightset::OutputType>(m_tOSCServerParams.tOutputType);
	}
#endif

    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_tOSCServerParams.nSetList & nMask) == nMask;
    }

private:
	OSCServerParamsStore *m_pOSCServerParamsStore;
    osc::server::Params m_tOSCServerParams;
};

#endif /* OSCSERVERPARAMS_H_ */
