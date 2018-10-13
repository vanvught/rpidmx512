/**
 * @file artnetparams.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 *
 * Art-Net 3 Protocol Release V1.4 Document Revision 1.4bk 23/1/2016
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef ARTNETPARAMS_H_
#define ARTNETPARAMS_H_

#include <stdint.h>
#include <stdbool.h>

#if defined (__circle__)
#include <circle/time.h>
#else
#include <time.h>
#endif

#include "artnetnode.h"
#include "common.h"

enum  TOutputType {
	OUTPUT_TYPE_DMX,
	OUTPUT_TYPE_SPI,
	OUTPUT_TYPE_MONITOR
};

struct TArtNetParams {
    uint32_t nSetList;
    uint8_t nNet;
    uint8_t nSubnet;
    uint8_t nUniverse;
    TOutputType tOutputType;
    bool bUseTimeCode;
    bool bUseTimeSync;
    bool bEnableRdm;
    bool bRdmDiscovery;
    uint8_t aShortName[ARTNET_SHORT_NAME_LENGTH];
	uint8_t aLongName[ARTNET_LONG_NAME_LENGTH];
	uint8_t aManufacturerId[2];
	uint8_t aOemValue[2];
	time_t nNetworkTimeout;
	bool bDisableMergeTimeout;
	uint8_t nUniversePort[ARTNET_MAX_PORTS];
};

class ArtNetParamsStore {
public:
	virtual ~ArtNetParamsStore(void);

	virtual void Update(const struct TArtNetParams *pArtNetParams)=0;
	virtual void Copy(struct TArtNetParams *pArtNetParams)=0;

private:
};

class ArtNetParams {
public:
	ArtNetParams(ArtNetParamsStore *pArtNetParamsStore = 0);
	~ArtNetParams(void);

	bool Load(void);

	inline uint8_t GetNet(void) {
		return m_tArtNetParams.nNet;
	}

	inline uint8_t GetSubnet(void) {
		return m_tArtNetParams.nSubnet;
	}

	inline uint8_t GetUniverse(void) {
		return m_tArtNetParams.nUniverse;
	}

	inline const uint8_t *GetShortName(void) {
		return m_tArtNetParams.aShortName;
	}

	inline const uint8_t *GetLongName(void) {
		return m_tArtNetParams.aLongName;
	}

	inline TOutputType GetOutputType(void) {
		return m_tArtNetParams.tOutputType;
	}

	inline const uint8_t *GetManufacturerId(void) {
		return m_tArtNetParams.aManufacturerId;
	}

	inline time_t GetNetworkTimeout(void) {
		return m_tArtNetParams.nNetworkTimeout;
	}

	inline bool IsUseTimeCode(void) {
		return m_tArtNetParams.bUseTimeCode;
	}

	inline bool IsUseTimeSync(void) {
		return m_tArtNetParams.bUseTimeSync;

	}

	inline bool IsRdm(void) {
		return m_tArtNetParams.bEnableRdm;
	}

	inline bool IsRdmDiscovery(void) {
		return m_tArtNetParams.bRdmDiscovery;
	}

	uint8_t GetUniverse(uint8_t nPort, bool &IsSet) const;

	void Set(ArtNetNode *);
	void Dump(void);

public:
	static uint32_t GetMaskShortName(void);
	static uint32_t GetMaskLongName(void);
	static uint32_t GetMaskNet(void);
	static uint32_t GetMaskSubnet(void);
	static uint32_t GetMaskUniverse(uint8_t nPort);

private:
	bool isMaskSet(uint32_t) const;
	uint16_t HexUint16(const char *) const;

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);

private:
    ArtNetParamsStore *m_pArtNetParamsStore;
    struct TArtNetParams m_tArtNetParams;
};

#endif /* ARTNETPARAMS_H_ */
