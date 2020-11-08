/**
 * @file oscserverparams.h
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>

#include "oscserver.h"

#include "lightset.h"

struct TOSCServerParams {
	uint32_t nSetList;
	uint16_t nIncomingPort;
	uint16_t nOutgoingPort;
	TLightSetOutputType tOutputType;
	bool bPartialTransmission;
	char aPath[OscServerMax::PATH_LENGTH];
	char aPathInfo[OscServerMax::PATH_LENGTH];
	char aPathBlackOut[OscServerMax::PATH_LENGTH];
	bool bEnableNoChangeUpdate;
} __attribute__((packed));

struct OSCServerParamsMask {
	static constexpr auto INCOMING_PORT = (1U << 0);
	static constexpr auto OUTGOING_PORT = (1U << 1);
	static constexpr auto PATH = (1U << 2);
	static constexpr auto TRANSMISSION = (1U << 3);
	static constexpr auto OUTPUT = (1U << 4);
	static constexpr auto PATH_INFO = (1U << 5);
	static constexpr auto PATH_BLACKOUT = (1U << 6);
	static constexpr auto ENABLE_NO_CHANGE_OUTPUT = (1U << 7);
};

class OSCServerParamsStore {
public:
	virtual ~OSCServerParamsStore() {}

	virtual void Update(const struct TOSCServerParams *pOSCServerParams)=0;
	virtual void Copy(struct TOSCServerParams *pOSCServerParams)=0;
};

class OSCServerParams {
public:
	OSCServerParams(OSCServerParamsStore *m_pOSCServerParamsStore=nullptr);

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TOSCServerParams *ptOSCServerParams, char *pBuffer, uint32_t nLength, uint32_t &nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize);

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

	TLightSetOutputType GetOutputType() const {
		return m_tOSCServerParams.tOutputType;
	}

	bool IsEnableNoChangeUpdate() const {
		return m_tOSCServerParams.bEnableNoChangeUpdate;
	}

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_tOSCServerParams.nSetList & nMask) == nMask;
    }

private:
	OSCServerParamsStore *m_pOSCServerParamsStore;
    struct TOSCServerParams m_tOSCServerParams;
};

#endif /* OSCSERVERPARAMS_H_ */
