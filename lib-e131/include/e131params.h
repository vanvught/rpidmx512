/**
 * @file e131params.h
 *
 */
/* Copyright (C) 2016-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef E131PARAMS_H_
#define E131PARAMS_H_

#include <stdint.h>
#include <stdbool.h>

#include "e131bridge.h"

#include "lightset.h"

#define E131_PARAMS_MAX_PORTS	4

struct TE131Params {
    uint32_t nSetList;
    TLightSetOutputType tOutputType;
    uint16_t nUniverse;
    uint16_t nUniversePort[E131_PARAMS_MAX_PORTS];
	uint8_t nMergeMode;
	uint8_t nMergeModePort[E131_PARAMS_MAX_PORTS];
	float nNetworkTimeout;
	bool bDisableMergeTimeout;
};

enum TE131ParamsMask {
	E131_PARAMS_MASK_UNIVERSE = (1 << 0),
	E131_PARAMS_MASK_MERGE_MODE = (1 << 1),
	E131_PARAMS_MASK_OUTPUT = (1 << 2),
	E131_PARAMS_MASK_CID = (1 << 3),
	E131_PARAMS_MASK_UNIVERSE_A = (1 << 4),
	E131_PARAMS_MASK_UNIVERSE_B = (1 << 5),
	E131_PARAMS_MASK_UNIVERSE_C = (1 << 6),
	E131_PARAMS_MASK_UNIVERSE_D = (1 << 7),
	E131_PARAMS_MASK_MERGE_MODE_A = (1 << 8),
	E131_PARAMS_MASK_MERGE_MODE_B = (1 << 9),
	E131_PARAMS_MASK_MERGE_MODE_C = (1 << 10),
	E131_PARAMS_MASK_MERGE_MODE_D = (1 << 11),
	E131_PARAMS_MASK_NETWORK_TIMEOUT = (1 << 12),
	E131_PARAMS_MASK_MERGE_TIMEOUT = (1 << 13)
};

class E131ParamsStore {
public:
	virtual ~E131ParamsStore(void);

	virtual void Update(const struct TE131Params *pE131Params)=0;
	virtual void Copy(struct TE131Params *pE131Params)=0;
};

class E131Params {
public:
	E131Params(E131ParamsStore *pE131ParamsStore = 0);
	~E131Params(void);

	bool Load(void);
	void Load(const char *pBuffer, uint32_t nLength);

	bool Builder(const struct TE131Params *ptE131Params, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);
	bool Save(uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Set(E131Bridge *);

	void Dump(void);

	TLightSetOutputType GetOutputType(void) {
		return m_tE131Params.tOutputType;
	}

	uint16_t GetUniverse(void) {
		return m_tE131Params.nUniverse;
	}

	TE131Merge GetMergeMode(void) {
		return (TE131Merge) m_tE131Params.nMergeMode;
	}

	uint16_t GetUniverse(uint8_t nPort, bool &IsSet) const;

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t) const;

private:
    E131ParamsStore *m_pE131ParamsStore;
    struct TE131Params m_tE131Params;
};

#endif /* E131PARAMS_H_ */
