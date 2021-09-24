/**
 * @file lightsetdata.h
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef LIGHTSETDATA_H_
#define LIGHTSETDATA_H_

#include <cstdint>
#include <cstring>
#include <algorithm>

#include "lightset.h"

namespace lightset {

class Data {
public:
	Data(const Data&) = delete;

	static Data& Get() {
		static Data instance;
		return instance;
	}

	static void SetSourceA(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength) {
		Get().IMergeSourceA(nPortIndex, pData, nLength, MergeMode::LTP);
	}

	static void MergeSourceA(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, MergeMode mergeMode) {
		 Get().IMergeSourceA(nPortIndex, pData, nLength, mergeMode);
	}

	static void SetSourceB(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength) {
		Get().IMergeSourceB(nPortIndex, pData, nLength, MergeMode::LTP);
	}

	static void MergeSourceB(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, MergeMode mergeMode) {
		 Get().IMergeSourceB(nPortIndex, pData, nLength, mergeMode);
	}

	static void Set(LightSet *pLightSet, uint32_t nPortIndex) {
		Get().ISet(pLightSet, nPortIndex);
	}

	static void SetClear(LightSet *pLightSet, uint32_t nPortIndex) {
		Get().ISetClear(pLightSet, nPortIndex);
	}

	static void ClearLength(uint32_t nPortIndex) {
		Get().IClearLength(nPortIndex);
	}

private:
	Data() {}

	void IMergeSourceA(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, MergeMode mergeMode) {
		memcpy(m_OutputPort[nPortIndex].sourceA.data, pData, nLength);

		m_OutputPort[nPortIndex].nLength = nLength;

		if (mergeMode == MergeMode::HTP) {
			for (uint32_t i = 0; i < nLength; i++) {
				auto data = std::max(m_OutputPort[nPortIndex].sourceA.data[i], m_OutputPort[nPortIndex].sourceB.data[i]);
				m_OutputPort[nPortIndex].data[i] = data;
			}

			return;
		}

		memcpy(m_OutputPort[nPortIndex].data, pData, nLength);
	}

	void IMergeSourceB(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, MergeMode mergeMode) {
		memcpy(m_OutputPort[nPortIndex].sourceB.data, pData, nLength);

		m_OutputPort[nPortIndex].nLength = nLength;

		if (mergeMode == MergeMode::HTP) {
			for (uint32_t i = 0; i < nLength; i++) {
				auto data = std::max(m_OutputPort[nPortIndex].sourceA.data[i], m_OutputPort[nPortIndex].sourceB.data[i]);
				m_OutputPort[nPortIndex].data[i] = data;
			}

			return;
		}

		memcpy(m_OutputPort[nPortIndex].data, pData, nLength);
	}

	void ISet(LightSet *pLightSet, uint32_t nPortIndex) const {
		pLightSet->SetData(nPortIndex, m_OutputPort[nPortIndex].data, m_OutputPort[nPortIndex].nLength);
	}

	void ISetClear(LightSet *pLightSet, uint32_t nPortIndex) {
		memset(m_OutputPort[nPortIndex].data, 0, Dmx::UNIVERSE_SIZE);
		m_OutputPort[nPortIndex].nLength = Dmx::UNIVERSE_SIZE;
		ISet(pLightSet, nPortIndex);
	}

	void IClearLength(uint32_t nPortIndex) {
		m_OutputPort[nPortIndex].nLength = 0;
	}

private:
	struct Source {
		uint8_t data[Dmx::UNIVERSE_SIZE];
	};

	struct OutputPort {
		Source sourceA;
		Source sourceB;
		uint8_t data[Dmx::UNIVERSE_SIZE];
		uint32_t nLength;
	};

	OutputPort m_OutputPort[8];
};

}  // namespace lightset

#endif /* LIGHTSETDATA_H_ */
