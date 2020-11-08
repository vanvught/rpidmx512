/**
 * @file artnet4params.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "artnetparams.h"
#include "artnet4node.h"

struct TArtNet4Params {
	uint32_t nSetList;
	bool bMapUniverse0;
} __attribute__((packed));

struct ArtNet4ParamsMask {
	static constexpr auto MAP_UNIVERSE0 = (1U << 0);
};

class ArtNet4ParamsStore {
public:
	virtual ~ArtNet4ParamsStore() {}

	virtual void Update(const struct TArtNet4Params *pArtNet4Params)=0;
	virtual void Copy(struct TArtNet4Params *pArtNet4Params)=0;
};

class ArtNet4Params: public ArtNetParams {
public:
	ArtNet4Params(ArtNet4ParamsStore *pArtNet4ParamsStore = nullptr);

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TArtNet4Params *pArtNet4Params, char *pBuffer, uint32_t nLength, uint32_t &nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Set(ArtNet4Node *pArtNet4Node);

	void Dump();

	bool IsMapUniverse0() const {
		return m_tArtNet4Params.bMapUniverse0;
	}

	static void staticCallbackFunction(void *p, const char *s);

private:
	void callbackFunction(const char *pLine);
	bool isMaskSet(uint32_t nMask) const {
		return (m_tArtNet4Params.nSetList & nMask) == nMask;
	}

private:
	ArtNet4ParamsStore *m_pArtNet4ParamsStore;
	struct TArtNet4Params m_tArtNet4Params;
};

#endif /* ARTNET4PARAMS_H_ */
