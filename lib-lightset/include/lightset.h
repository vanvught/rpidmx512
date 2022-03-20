/**
 * @file lightset.h
 *
 */
/* Copyright (C) 2016-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef LIGHTSET_H_
#define LIGHTSET_H_

#include <cstdint>
#include <climits>

namespace lightset {
struct Dmx {
	static constexpr auto ADDRESS_INVALID = 0xFFFF;
	static constexpr auto START_ADDRESS_DEFAULT = 1U;
	static constexpr auto UNIVERSE_SIZE = 512U;
	static constexpr auto MAX_VALUE = 255U;
};

enum class MergeMode {
	HTP, LTP
};

enum class PortDir {
	INPUT, OUTPUT, DISABLE
};

struct SlotInfo {
	uint16_t nCategory;
	uint8_t nType;
};

// WiFi solutions only
enum class OutputType {
	DMX, SPI, MONITOR, UNDEFINED
};

inline static MergeMode get_merge_mode(const char *pMergeMode) {
	if (pMergeMode != nullptr) {
		if (((pMergeMode[0] | 0x20) == 'l')
		 && ((pMergeMode[1] | 0x20) == 't')
		 && ((pMergeMode[2] | 0x20) == 'p')) {
			return MergeMode::LTP;
		}
	}
	return MergeMode::HTP;
}

inline static const char* get_merge_mode(MergeMode m, const bool bToUpper = false) {
	if (bToUpper) {
		return (m == MergeMode::HTP) ? "HTP" : "LTP";
	}
	return (m == MergeMode::HTP) ? "htp" : "ltp";
}

inline static const char* get_merge_mode(unsigned m, const bool bToUpper = false) {
	return get_merge_mode(static_cast<MergeMode>(m), bToUpper);
}

inline static PortDir get_direction(const char *pPortDir) {
	if (pPortDir != nullptr) {
		if (((pPortDir[0] | 0x20) == 'i')
		 && ((pPortDir[1] | 0x20) == 'n')
		 && ((pPortDir[2] | 0x20) == 'p')
		 && ((pPortDir[3] | 0x20) == 'u')
		 && ((pPortDir[4] | 0x20) == 't')) {
			return PortDir::INPUT;
		}
	}
	return PortDir::OUTPUT;
}

inline static const char* get_direction(uint32_t nPortIndex, uint8_t nDirection) {
	if (nPortIndex < CHAR_BIT) {
		return ((nDirection >> nPortIndex) & 0x1) == static_cast<uint8_t>(PortDir::INPUT) ? "input" : "output";
	}

	return "output";
}
}  // namespace lightset

class LightSet {
public:
	LightSet() {}
	virtual ~LightSet() {}

	virtual void Start(uint32_t nPortIndex)= 0;
	virtual void Stop(uint32_t nPortIndex)= 0;
	virtual void SetData(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength)= 0;
	// Optional
	virtual void Blackout(__attribute__((unused)) bool bBlackout) {}
	virtual void Print() {}
	// RDM Optional
	virtual bool SetDmxStartAddress(uint16_t nDmxStartAddress);
	virtual uint16_t GetDmxStartAddress();
	virtual uint16_t GetDmxFootprint();
	virtual bool GetSlotInfo(uint16_t nSlotOffset, lightset::SlotInfo &tSlotInfo);

#if defined (ESP8266)
	// WiFi solutions only
	static const char *GetOutputType(lightset::OutputType type);
	static lightset::OutputType GetOutputType(const char *sType);
#endif
};

#endif /* LIGHTSET_H_ */
