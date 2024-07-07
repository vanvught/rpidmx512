/**
 * @file oscclientparams.h
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef OSCCLIENTPARAMS_H_
#define OSCCLIENTPARAMS_H_

#include <cstdint>

#include "oscclient.h"
#include "configstore.h"

namespace oscclientparams {
struct ParamsMax {
	static constexpr uint32_t CMD_COUNT = 8;
	static constexpr uint32_t CMD_PATH_LENGTH = 64;
	static constexpr uint32_t LED_COUNT = 8;
	static constexpr uint32_t LED_PATH_LENGTH = 48;
};

struct Params {
    uint32_t nSetList;
    uint32_t nServerIp;
	uint16_t nOutgoingPort;
	uint16_t nIncomingPort;
	uint8_t nPingDisable;
	uint8_t nPingDelay;
	char aCmd[ParamsMax::CMD_COUNT][ParamsMax::CMD_PATH_LENGTH];
	char aLed[ParamsMax::LED_COUNT][ParamsMax::LED_PATH_LENGTH];
} __attribute__((packed));

static_assert(sizeof(struct oscclientparams::Params) <= oscclient::STORE, "struct Params is too large");

struct Mask {
	static constexpr auto SERVER_IP = (1U << 0);
	static constexpr auto OUTGOING_PORT = (1U << 1);
	static constexpr auto INCOMING_PORT = (1U << 2);
	static constexpr auto PING_DISABLE = (1U << 3);
	static constexpr auto PING_DELAY = (1U << 4);
	static constexpr auto CMD = (1U << 5);
	static constexpr auto LED = (1U << 6);
};
}  // namespace oscclientparams

class OscClientParamsStore {
public:
	static void Update(const struct oscclientparams::Params *pParams) {
		ConfigStore::Get()->Update(configstore::Store::OSC_CLIENT, pParams, sizeof(struct oscclientparams::Params));
	}

	static void Copy(struct oscclientparams::Params *pParams) {
		ConfigStore::Get()->Copy(configstore::Store::OSC_CLIENT, pParams, sizeof(struct oscclientparams::Params));
	}
};

class OscClientParams {
public:
	OscClientParams();

	void Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct oscclientparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	void Set(OscClient *pOscClient);

	uint32_t GetServerIP() {
		return m_Params.nServerIp;
	}

	uint16_t GetOutgoingPort() {
		return m_Params.nOutgoingPort;
	}

	uint16_t GetIncomingPort() {
		return m_Params.nIncomingPort;
	}

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
	void Dump();
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_Params.nSetList & nMask) == nMask;
    }

private:
    oscclientparams::Params m_Params;
    char m_aCmd[8];
    char m_aLed[8];
};

#endif /* OSCCLIENTPARAMS_H_ */
