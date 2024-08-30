/**
 * @file lightsetchain.h
 *
 */
/* Copyright (C) 2017-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>

#include "lightset.h"

#define LIGHTSET_TYPE_UNDEFINED -1

struct TLightSetEntry {
	LightSet *pLightSet;
	int	nType;
};

class LightSetChain final: public LightSet {
public:
	LightSetChain();
	~LightSetChain() override;

	void Start(const uint32_t nPortIndex) override;
	void Stop(const uint32_t nPortIndex) override;

	void SetData(const uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate = true) override;
	void Sync(const uint32_t nPortIndex) override;
	void Sync() override;
#if defined (OUTPUT_HAVE_STYLESWITCH)
	void SetOutputStyle(const uint32_t nPortIndex, const lightset::OutputStyle outputStyle) override;
	lightset::OutputStyle GetOutputStyle(const uint32_t nPortIndex) const override;
#endif
	void Print() override;

public: // RDM
	bool SetDmxStartAddress(uint16_t nDmxStartAddress) override;
	 uint16_t GetDmxStartAddress() override {
		return m_nDmxStartAddress;
	}

	 uint16_t GetDmxFootprint() override  {
		return m_nDmxFootprint;
	}

	bool GetSlotInfo(uint16_t nSlotOffset, lightset::SlotInfo &tSlotInfo) override;

public:
	bool Add(LightSet *, int nType = LIGHTSET_TYPE_UNDEFINED);
	bool IsEmpty() const;
	bool Exist(LightSet *);
	bool Exist(LightSet *, int, bool DoIgnoreType = false);

	uint8_t GetSize() const;
	int GetType(uint8_t) const;
	const LightSet *GetLightSet(uint8_t);

public:
	void Dump(uint8_t);
	void Dump();

private:
	uint8_t m_nSize { 0 };
	TLightSetEntry *m_pTable;
	uint16_t m_nDmxStartAddress { lightset::dmx::ADDRESS_INVALID };
	uint16_t m_nDmxFootprint { 0 };
};

#endif /* LIGHTSETCHAIN_H_ */
