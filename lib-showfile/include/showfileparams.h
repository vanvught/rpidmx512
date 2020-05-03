/**
 * @file showfileparams.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef SHOWFILEPARAMS_H_
#define SHOWFILEPARAMS_H_

#include <stdint.h>

#include "showfile.h"

struct TShowFileParams {
    uint32_t nSetList;
    uint8_t nFormat;
    uint8_t nShow;
    uint16_t nOptions;
    uint16_t nOscPortIncoming;
    uint16_t nOscPortOutgoing;
    uint8_t nProtocol;
    uint16_t nUniverse;
    uint8_t nDisableUnicast;
    uint8_t nDmxMaster;
}__attribute__((packed));

enum TShowFileOptions {
	SHOWFILE_OPTION_AUTO_START = (1 << 0),
	SHOWFILE_OPTION_LOOP = (1 << 1),
	SHOWFILE_OPTION_DISABLE_SYNC = (1 << 2)
};

enum TShowFileParamsMask {
	SHOWFILE_PARAMS_MASK_FORMAT = (1 << 0),
	SHOWFILE_PARAMS_MASK_SHOW = (1 << 1),
	SHOWFILE_PARAMS_MASK_OPTIONS = (1 << 2),
	SHOWFILE_PARAMS_MASK_OSC_PORT_INCOMING = (1 << 3),
	SHOWFILE_PARAMS_MASK_OSC_PORT_OUTGOING = (1 << 4),
	SHOWFILE_PARAMS_MASK_PROTOCOL = (1 << 5),
	SHOWFILE_PARAMS_MASK_SACN_UNIVERSE = (1 << 6),
	SHOWFILE_PARAMS_MASK_ARTNET_UNICAST_DISABLED = (1 << 7),
	SHOWFILE_PARAMS_MASK_DMX_MASTER = (1 << 8)
};

class ShowFileParamsStore {
public:
	virtual ~ShowFileParamsStore(void) {}

	virtual void Update(const struct TShowFileParams *pShowFileParams)=0;
	virtual void Copy(struct TShowFileParams *pShowFileParams)=0;
};

class ShowFileParams {
public:
	ShowFileParams(ShowFileParamsStore *pShowFileParamsStore = 0);
	~ShowFileParams(void);

	bool Load(void);
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TShowFileParams *ptShowFileParamss, char *pBuffer, uint32_t nLength, uint32_t &nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t &nSize);

	void Set(void);

	void Dump(void);

	TShowFileFormats GetFormat(void) {
		return static_cast<TShowFileFormats>(m_tShowFileParams.nFormat);
	}

	TShowFileProtocols GetProtocol(void) {
		return static_cast<TShowFileProtocols>(m_tShowFileParams.nProtocol);
	}

	uint8_t GetShow(void) {
		return m_tShowFileParams.nShow;
	}

	bool IsAutoStart(void) {
		return isOptionSet(SHOWFILE_OPTION_AUTO_START);
	}

	bool IsArtNetBroadcast(void) {
		return isMaskSet(SHOWFILE_PARAMS_MASK_ARTNET_UNICAST_DISABLED);
	}

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void HandleOptions(const char *pLine, const char *pKeyword, TShowFileOptions tShowFileOptions);
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) {
    	return (m_tShowFileParams.nSetList & nMask) == nMask;
    }
    bool isOptionSet(uint16_t nMask) {
    	return (m_tShowFileParams.nOptions & nMask) == nMask;
    }

private:
    ShowFileParamsStore *m_pShowFileParamsStore;
    struct TShowFileParams m_tShowFileParams;
};

#endif /* SHOWFILEPARAMS_H_ */
