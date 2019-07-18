/**
 * @file oscclientparams.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>

#include "oscclient.h"

#define OSCCLIENT_PARAMS_CMD_MAX_COUNT				10
#define OSCCLIENT_PARAMS_CMD_MAX_PATH_LENGTH		64 // (1 << 6)

struct TOscClientParams {
    uint32_t nSetList;
    uint32_t nServerIp;
	uint16_t nOutgoingPort;
	uint16_t nIncomingPort;
	uint8_t nPingDisable;
	uint8_t nPingDelay;
	uint8_t aCmd[OSCCLIENT_PARAMS_CMD_MAX_COUNT][OSCCLIENT_PARAMS_CMD_MAX_PATH_LENGTH];
};

enum TOscClientParamsMask {
	OSCCLIENT_PARAMS_MASK_SERVER_IP = (1 << 0),
	OSCCLIENT_PARAMS_MASK_OUTGOING_PORT = (1 << 1),
	OSCCLIENT_PARAMS_MASK_INCOMING_PORT = (1 << 2),
	OSCCLIENT_PARAMS_MASK_PING_DISABLE = (1 << 3),
	OSCCLIENT_PARAMS_MASK_PING_DELAY = (1 << 4),
	OSCCLIENT_PARAMS_MASK_CMD = (1 << 5)
};

class OscClientParamsStore {
public:
	virtual ~OscClientParamsStore(void);

	virtual void Update(const struct TOscClientParams *pOscClientParams)=0;
	virtual void Copy(struct TOscClientParams *pOscClientParams)=0;
};

class OscClientParams {
public:
	OscClientParams(OscClientParamsStore *m_pOscClientParamsStore=0);
	~OscClientParams(void);

	bool Load(void);
	void Load(const char *pBuffer, uint32_t nLength);

	bool Builder(const struct TOscClientParams *ptOscClientParams, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);
	bool Save(uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Set(OscClient *pOscClient);

	void Dump(void);

	uint32_t GetServerIP(void) {
		return m_tOscClientParams.nServerIp;
	}

	uint16_t GetOutgoingPort(void) {
		return m_tOscClientParams.nOutgoingPort;
	}

	uint16_t GetIncomingPort(void) {
		return m_tOscClientParams.nIncomingPort;
	}

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);
	bool isMaskSet(uint16_t nMask) const;

private:
	OscClientParamsStore *m_pOscClientParamsStore;
    struct TOscClientParams m_tOscClientParams;
    char m_aCmd[8];
};

#endif /* OSCCLIENTPARAMS_H_ */
