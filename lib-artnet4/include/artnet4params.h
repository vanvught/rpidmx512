/**
 * @file artnet4params.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
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

#ifndef ARTNET4PARAMS_H_
#define ARTNET4PARAMS_H_

#include <stdint.h>
#include <stdbool.h>

#include "artnetparams.h"
#include "artnet4node.h"

struct TArtNet4Params {
	uint32_t nSetList;
	bool bMapUniverse0;
};

enum TArtNet4ParamsMask {
	ARTNET4_PARAMS_MASK_MAP_UNIVERSE0 = (1 << 0)
};

class ArtNet4ParamsStore {
public:
	virtual ~ArtNet4ParamsStore(void);

	virtual void Update(const struct TArtNet4Params *pArtNet4Params)=0;
	virtual void Copy(struct TArtNet4Params *pArtNet4Params)=0;
};

class ArtNet4Params: public ArtNetParams {
public:
	ArtNet4Params(ArtNet4ParamsStore *pArtNet4ParamsStore = 0);
	~ArtNet4Params(void);

	bool Load(void);
	void Load(const char *pBuffer, uint32_t nLength);

	bool Builder(const struct TArtNet4Params *pArtNet4Params, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);
	bool Save(uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Set(ArtNet4Node *pArtNet4Node);

	void Dump(void);

	bool IsMapUniverse0(void) {
		return m_tArtNet4Params.bMapUniverse0;
	}

public:
	static void staticCallbackFunction(void *p, const char *s);

private:
	void callbackFunction(const char *pLine);
	bool isMaskSet(uint32_t nMask) const;

private:
	ArtNet4ParamsStore *m_pArtNet4ParamsStore;
	struct TArtNet4Params m_tArtNet4Params;
};

#endif /* ARTNET4PARAMS_H_ */
