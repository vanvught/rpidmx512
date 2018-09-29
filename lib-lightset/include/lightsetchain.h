/**
 * @file lightsetchain.h
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef LIGHTSETCHAIN_H_
#define LIGHTSETCHAIN_H_

#include <stdint.h>

#include "lightset.h"

#define LIGHTSET_TYPE_UNDEFINED -1

struct TLightSetEntry {
	LightSet *pLightSet;
	int	nType;
};

class LightSetChain: public LightSet {
public:
	LightSetChain(void);
	~LightSetChain(void);

	void Start(uint8_t nPort);
	void Stop(uint8_t nPort);

	void SetData(uint8_t nPort, const uint8_t *, uint16_t);

public: // RDM
	bool SetDmxStartAddress(uint16_t nDmxStartAddress);
	inline uint16_t GetDmxStartAddress(void) {
		return m_nDmxStartAddress;
	}

	inline uint16_t GetDmxFootprint(void)  {
		return m_nDmxFootprint;
	}

	bool GetSlotInfo(uint16_t nSlotOffset, struct TLightSetSlotInfo &tSlotInfo);

public:
	bool Add(LightSet *, int nType = LIGHTSET_TYPE_UNDEFINED);
	bool IsEmpty(void) const;
	bool Exist(LightSet *);
	bool Exist(LightSet *, int, bool DoIgnoreType = false);

	uint8_t GetSize(void) const;
	int GetType(uint8_t) const;
	const LightSet *GetLightSet(uint8_t);

public:
	void Dump(uint8_t);
	void Dump(void);

private:
	uint8_t m_nSize;
	TLightSetEntry *m_pTable;
	uint16_t m_nDmxStartAddress;
	uint16_t m_nDmxFootprint;
};

#endif /* LIGHTSETCHAIN_H_ */
