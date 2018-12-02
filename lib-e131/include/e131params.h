/**
 * @file e131params.h
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

#ifndef E131PARAMS_H_
#define E131PARAMS_H_

#include <stdint.h>
#include <stdbool.h>

#include "e131bridge.h"

enum TE131OutputType {
	E131_OUTPUT_TYPE_DMX,
	E131_OUTPUT_TYPE_SPI,
	E131_OUTPUT_TYPE_MONITOR
};

struct TE131Params {
    uint32_t nSetList;
    TE131OutputType tOutputType;
    char aCidString[UUID_STRING_LENGTH + 2];
    bool bHaveCustomCid;
    uint16_t nUniverse;
    uint16_t nUniversePort[E131_MAX_PORTS];
	uint8_t nMergeMode;
	uint8_t nMergeModePort[E131_MAX_PORTS];
};

class E131ParamsStore {
public:
	virtual ~E131ParamsStore(void);

	virtual void Update(const struct TE131Params *pE131Params)=0;
	virtual void Copy(struct TE131Params *pE131Params)=0;

private:
};

class E131Params {
public:
	E131Params(E131ParamsStore *pE131ParamsStore = 0);
	~E131Params(void);

	bool Load(void);
	void Set(E131Bridge *);
	void Dump(void);

	inline TE131OutputType GetOutputType(void) {
		return m_tE131Params.tOutputType;
	}

	inline uint16_t GetUniverse(void) {
		return m_tE131Params.nUniverse;
	}

	inline TE131Merge GetMergeMode(void) {
		return (TE131Merge) m_tE131Params.nMergeMode;
	}

	inline bool isHaveCustomCid(void) {
		return m_tE131Params.bHaveCustomCid;
	}

	inline const char* GetCidString(void) {
		return (const char*) m_tE131Params.aCidString;
	}

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
