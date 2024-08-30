/**
 * @file lightsetdata.h
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "lightset.h"

#if defined (GD32)
/**
 * https://www.gd32-dmx.org/memory.html
 */
# include "gd32.h"
# if defined (GD32F450VI) || defined (GD32H7XX)
#  define SECTION_LIGHTSET __attribute__ ((section (".lightset")))
# else
#  define SECTION_LIGHTSET
# endif
#else
# define SECTION_LIGHTSET
#endif
namespace lightset {

class Data {
public:
//	Data(const Data&) = delete;

	static Data& Get() {
		static Data instance SECTION_LIGHTSET;
		return instance;
	}

	static void SetSourceA(const uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength) {
		Get().IMergeSourceA(nPortIndex, pData, nLength, MergeMode::LTP);
	}

	static void MergeSourceA(const uint32_t nPortIndex, const uint8_t *pData, const uint32_t nLength, const MergeMode mergeMode) {
		 Get().IMergeSourceA(nPortIndex, pData, nLength, mergeMode);
	}

	static void SetSourceB(const uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength) {
		Get().IMergeSourceB(nPortIndex, pData, nLength, MergeMode::LTP);
	}

	static void MergeSourceB(const uint32_t nPortIndex, const uint8_t *pData, const uint32_t nLength, const MergeMode mergeMode) {
		 Get().IMergeSourceB(nPortIndex, pData, nLength, mergeMode);
	}

	static void Set(LightSet *const pLightSet, uint32_t nPortIndex) {
		Get().ISet(pLightSet, nPortIndex);
	}

	static void Output(LightSet *const pLightSet, uint32_t nPortIndex) {
		Get().IOutput(pLightSet, nPortIndex);
	}

	static void OutputClear(LightSet *const pLightSet, uint32_t nPortIndex) {
		Get().IOutputClear(pLightSet, nPortIndex);
	}

	static void ClearLength(const uint32_t nPortIndex) {
		Get().IClearLength(nPortIndex);
	}

	static uint32_t GetLength(const uint32_t nPortIndex) {
		return Get().IGetLength(nPortIndex);
	}

	static const uint8_t *Backup(const uint32_t nPortIndex) {
		return Get().IBackup(nPortIndex);
	}

	static void Restore(const uint32_t nPortIndex, const uint8_t *pData) {
		Get().IRestore(nPortIndex, pData);
	}

private:
//	Data() {}

	void IMergeSourceA(const uint32_t nPortIndex, const uint8_t *pData, const uint32_t nLength, const MergeMode mergeMode) {
		assert(nPortIndex < PORTS);
		assert(pData != nullptr);

		memcpy(m_OutputPort[nPortIndex].sourceA.data, pData, nLength);

		m_OutputPort[nPortIndex].nLength = nLength;

		if (mergeMode == MergeMode::HTP) {
			for (uint32_t i = 0; i < nLength; i++) {
				const auto data = std::max(m_OutputPort[nPortIndex].sourceA.data[i], m_OutputPort[nPortIndex].sourceB.data[i]);
				m_OutputPort[nPortIndex].data[i] = data;
			}

			return;
		}

		memcpy(m_OutputPort[nPortIndex].data, pData, nLength);
	}

	void IMergeSourceB(const uint32_t nPortIndex, const uint8_t *pData, const uint32_t nLength, const MergeMode mergeMode) {
		assert(nPortIndex < PORTS);
		assert(pData != nullptr);

		memcpy(m_OutputPort[nPortIndex].sourceB.data, pData, nLength);

		m_OutputPort[nPortIndex].nLength = nLength;

		if (mergeMode == MergeMode::HTP) {
			for (uint32_t i = 0; i < nLength; i++) {
				const auto data = std::max(m_OutputPort[nPortIndex].sourceA.data[i], m_OutputPort[nPortIndex].sourceB.data[i]);
				m_OutputPort[nPortIndex].data[i] = data;
			}

			return;
		}

		memcpy(m_OutputPort[nPortIndex].data, pData, nLength);
	}

	void ISet(LightSet *const pLightSet, const uint32_t nPortIndex) const {
		assert(pLightSet != nullptr);
		assert(nPortIndex < PORTS);

		pLightSet->SetData(nPortIndex, m_OutputPort[nPortIndex].data, m_OutputPort[nPortIndex].nLength, false);
	}

	void IOutput(LightSet *const pLightSet, const uint32_t nPortIndex) const {
		assert(pLightSet != nullptr);
		assert(nPortIndex < PORTS);

		pLightSet->SetData(nPortIndex, m_OutputPort[nPortIndex].data, m_OutputPort[nPortIndex].nLength, true);
	}

	void IOutputClear(LightSet *const pLightSet, const uint32_t nPortIndex) {
		assert(pLightSet != nullptr);
		assert(nPortIndex < PORTS);

		memset(m_OutputPort[nPortIndex].data, 0, dmx::UNIVERSE_SIZE);
		m_OutputPort[nPortIndex].nLength = dmx::UNIVERSE_SIZE;
		IOutput(pLightSet, nPortIndex);
	}

	void IClearLength(const uint32_t nPortIndex) {
		assert(nPortIndex < PORTS);

		m_OutputPort[nPortIndex].nLength = 0;
	}

	uint32_t IGetLength(const uint32_t nPortIndex) const {
		return m_OutputPort[nPortIndex].nLength;
	}

	const uint8_t *IBackup(const uint32_t nPortIndex) {
		assert(nPortIndex < PORTS);
		return const_cast<const uint8_t *>(m_OutputPort[nPortIndex].data);
	}

	void IRestore(const uint32_t nPortIndex, const uint8_t *pData) {
		assert(nPortIndex < PORTS);
		assert(pData != nullptr);

		memcpy(m_OutputPort[nPortIndex].data, pData, dmx::UNIVERSE_SIZE);
	}

private:
#if !defined (LIGHTSET_PORTS)
# define LIGHTSET_PORTS 0
#endif

#if (LIGHTSET_PORTS == 0)
	static constexpr auto PORTS = 1;	// ISO C++ forbids zero-size array
#else
	static constexpr auto PORTS = LIGHTSET_PORTS;
#endif

	struct Source {
		uint8_t data[dmx::UNIVERSE_SIZE] __attribute__ ((aligned (4)));
	};

	struct OutputPort {
		Source sourceA;
		Source sourceB;
		uint8_t data[dmx::UNIVERSE_SIZE] __attribute__ ((aligned (4)));
		uint32_t nLength;
	};

	OutputPort m_OutputPort[PORTS];
};

}  // namespace lightset

#endif /* LIGHTSETDATA_H_ */
